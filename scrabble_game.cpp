#include "scrabble_game.hpp"
#include "scrabble_word.hpp"
#include "player.hpp"
#include "scrabble_config.hpp"

#include <fstream>
#include <sstream>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
Scrabble_Game::~Scrabble_Game()
////////////////////////////////////////////////////////////////////////////////
{
  //delete all players, board, and piece-source
  for (unsigned i = 0; i < m_players.size(); ++i) {
    delete m_players[i];
  }
  delete m_game_board;
  delete m_piece_source;
}

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Game::initialize()
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(is_complete(), "Tried to initialize a game that was not fully built.");

  //read dictionary, put all words into set
  std::string dict_name = m_config.DICTIONARY();
  ifstream in(dict_name.c_str());
  my_assert(in, std::string("Unable to open dictionary-file: ") + dict_name);
  std::string word;
  while (in >> word) {
    m_valid_words.insert(word);
  }
  in.close();

  //give players their initial set of pieces
  unsigned num_pieces_per_player = m_config.NUM_PLAYER_PIECES();
  for (unsigned i = 0; i < m_players.size(); i++) {
    m_players[i]->initialize();
    unsigned num_needed = num_pieces_per_player - m_players[i]->get_num_pieces();
    for (unsigned p = 0; p < num_needed; ++p) {
      if (m_piece_source->is_empty()) {
        break;
      }
      m_players[i]->add_piece(m_piece_source->get_piece());
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Game::play()
////////////////////////////////////////////////////////////////////////////////
{
  //must initialize before real play starts
  initialize();

  //the game configuration will affect if we produce output here
  const Output_Type output = m_config.OUTPUT();

  if (output == GUI) {
    const unsigned dim = m_game_board->get_board_dim();
    unsigned count = 0;
    for (unsigned i = 0; i < dim; ++i) {
      for (unsigned j = 0; j < dim; ++j) {
        const Bonus bonus = m_game_board->get_square(i, j).get_bonus();
        if (bonus != NONE) {
          m_row_buff[count] = i;
          m_col_buff[count] = j;
          m_let_buff[count] = static_cast<char>(bonus);
          ++count;
        }
      }
    }
    bool worked = m_config.PY_CALLBACK()(BOARD_INIT, count, m_row_buff, m_col_buff, m_let_buff);
    my_static_assert(worked, "GUI failure");
  }

  //continue having players make plays until the game is over
  while (!m_game_over) {
    //need to track if all players make a null move, this implies the game is stuck and needs to end
    bool not_all_null = false;
    //begin a new round of plays (loop over each player, have them play once)
    for (unsigned i = 0; i < m_players.size() && !m_game_over; i++) {
      Player& player = *m_players[i];

      //display the state of the game
      if (output == TEXT) {
        cout << *this << endl;
      }
      else if (output == GUI) {
        for (unsigned p = 0; p < player.get_num_pieces(); ++p) {
          m_let_buff[p] = player.observe_piece(p)->get_letter();
        }
        bool worked = m_config.PY_CALLBACK()(TILES, player.get_num_pieces(), m_row_buff, m_col_buff, m_let_buff);
        my_static_assert(worked, "GUI failure");
      }

      //this player will go until they have made a valid move
      //(placing no letters is a valid move (equiv to skip)
      while (true) {
        const Indv_Play& this_play = player.play();

        std::string err_str = evaluate_play(this_play);
        if (err_str == "") {
          if (this_play.get_size() > 0) {
            not_all_null = true;
          }

          //play was legit, process it
          process_legit_play(this_play, &player);
          if (output == GUI && player.is_human()) {
            bool worked = m_config.PY_CALLBACK()(CONFIRM_PLAY, 0, m_row_buff, m_col_buff, m_let_buff);
            my_static_assert(worked, "GUI failure");
          }
          break;
        }
        else {
          if (output == TEXT) {
            cout << err_str << endl;
          }
          else if (output == GUI && player.is_human()) {
            my_require(err_str.size() < 128, "Too big");
            for (unsigned e = 0; e < err_str.size(); ++e) {
              m_let_buff[e] = err_str[e];
            }
            bool worked = m_config.PY_CALLBACK()(CONFIRM_PLAY, err_str.size(), m_row_buff, m_col_buff, m_let_buff);
            my_static_assert(worked, "GUI failure");
          }
        }
      }
    }
    if (!not_all_null) {
      m_game_over = true;
    }

    if (m_game_over) {
      if (output == TEXT) {
        if (!not_all_null) {
          cout << "NO PLAYER COULD MOVE, GAME OVER" << endl;
        }
      }
      else if (output == GUI) {
        // TODO - notify python of game over?
      }
    }
  }

  //game is over. produce one final output of the ending state of the game
  if (output == TEXT) {
    cout << *this << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string Scrabble_Game::evaluate_play(const Indv_Play& the_play) const
////////////////////////////////////////////////////////////////////////////////
{
  const unsigned num_played_letters = the_play.get_size();
  const unsigned b_dim              = m_game_board->get_board_dim();

  m_potential_score = 0;
  m_potential_words = "";

  //null plays are always allowed
  if (num_played_letters == 0) {
    return "";
  }

  //if first play, one piece must cover the middle square
  if (m_first_play) {
    const unsigned mid_dim = b_dim / 2;
    bool hit_center = false;
    for (unsigned i = 0; i < num_played_letters; ++i) {
      if (the_play.get_ith_row(i) == mid_dim && the_play.get_ith_col(i) == mid_dim) {
        hit_center = true;
        break;
      }
    }
    if (!hit_center) {
      return std::string("MOVE ") + obj_to_str(the_play) + " REJECTED: the first move must cover the middle square";
    }
  }

  //all played letters must be along the same line
  if (num_played_letters > 1) {
    bool const_row = the_play.get_ith_row(0) == the_play.get_ith_row(1);
    bool const_col = the_play.get_ith_col(0) == the_play.get_ith_col(1);
    if (!const_row && !const_col) {
      return std::string("MOVE ") + obj_to_str(the_play) + " REJECTED: all played letters must be linear";
    }
    for (unsigned i = 0; i < num_played_letters; ++i) {
      if ( (const_row && (the_play.get_ith_row(0) != the_play.get_ith_row(i))) ||
           (const_col && (the_play.get_ith_col(0) != the_play.get_ith_col(i))) ) {
        return std::string("MOVE ") + obj_to_str(the_play) + " REJECTED: all played letters must be linear";
      }
    }
  }

  //all played letters must be connected to each other directly or via sequences of pieces already on the board
  if (num_played_letters > 1) {
    bool const_row = the_play.get_ith_row(0) == the_play.get_ith_row(1);
    bool const_col = the_play.get_ith_col(0) == the_play.get_ith_col(1);
    multiset<unsigned> row_plays, col_plays;
    for (unsigned i = 0; i < num_played_letters; ++i) {
      row_plays.insert(the_play.get_ith_row(i));
      col_plays.insert(the_play.get_ith_col(i));
    }
    unsigned row_begin = *(row_plays.begin()), col_begin = *(col_plays.begin());
    unsigned row_end   = *(--row_plays.end()), col_end   = *(--col_plays.end());
    unsigned row_dir = const_row ? 0 : 1;
    unsigned col_dir = const_col ? 0 : 1;
    multiset<unsigned>::const_iterator row_play_itr = row_plays.begin();
    multiset<unsigned>::const_iterator col_play_itr = col_plays.begin();
    for (unsigned r = row_begin, c = col_begin; r <= row_end && c <= col_end; r += row_dir, c += col_dir) {
      if (*row_play_itr == r && *col_play_itr == c) {
        //play component takes care of this piece in the sequence
        ++row_play_itr;
        ++col_play_itr;
      }
      else if (!m_game_board->is_free(r, c)) {
        //piece already on board takes care of this piece in the sequence
      }
      else {
        return std::string("MOVE ") + obj_to_str(the_play) + " REJECTED: your play was not fully connected along the play-line";
      }
    }
  }

  //all played letters must lie within the game board
  for (unsigned i = 0; i < num_played_letters; ++i) {
    unsigned row = the_play.get_ith_row(i), col = the_play.get_ith_col(i);
    if (row >= b_dim || col >= b_dim) {
      return std::string("MOVE ") + obj_to_str(the_play) + " REJECTED: your play lied outside the board";
    }
  }

  //player may not make two placements on the same square
  set<int> played_rows, played_cols;
  for (unsigned i = 0; i < num_played_letters; ++i) {
    if (played_rows.find(the_play.get_ith_row(i)) != played_rows.end() &&
        played_cols.find(the_play.get_ith_col(i)) != played_cols.end()) {
      return std::string("MOVE ") + obj_to_str(the_play) + " REJECTED: tried to make two plays on the same square";
    }
    played_rows.insert(the_play.get_ith_row(i));
    played_cols.insert(the_play.get_ith_col(i));
  }

  //no piece may be placed on top of another piece
  for (unsigned i = 0; i < num_played_letters; ++i) {
    if (!m_game_board->is_free(the_play.get_ith_row(i), the_play.get_ith_col(i))) {
      return std::string("MOVE ") + obj_to_str(the_play) + " REJECTED: tried to place on top of another piece";
    }
  }

  //if not first play, one of the pieces must be adjacent to a piece already on the board
  if (!m_first_play) {
    bool found_adjacent = false;
    for (unsigned i = 0; i < num_played_letters; ++i) {
      if (m_game_board->is_adjacent(the_play.get_ith_row(i), the_play.get_ith_col(i))) {
        found_adjacent = true;
        break;
      }
    }
    if (!found_adjacent) {
      return std::string("MOVE ") + obj_to_str(the_play) + " REJECTED: play did not connect with any words";
    }
  }

  // Every adjacent word must continue to be a valid word, calc hypothetical score while we're at it
  const std::vector<Scrabble_Word> new_words = m_game_board->get_created_words(the_play);
  for (unsigned i = 0; i < new_words.size(); ++i) {
    if (!the_play.is_forced() && m_valid_words.find(new_words[i].get_word_str()) == m_valid_words.end()) {
      return std::string(std::string("MOVE ") + obj_to_str(the_play) + " REJECTED: \"") + new_words[i].get_word_str() + "\" is not a valid word";
    }
    else {
      m_potential_score += new_words[i].score();
      m_potential_words += new_words[i].get_word_str() + " ";
    }
  }
  //don't forget the bonus for a full set of letters
  if (the_play.get_size() == m_config.NUM_PLAYER_PIECES()) {
    m_potential_score += 50;
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Game::process_legit_play(const Indv_Play& the_play, Player* player)
////////////////////////////////////////////////////////////////////////////////
{
  const unsigned num_played_letters = the_play.get_size();

  //calculate score, add it to that player's score
  player->add_score(m_potential_score);

  //add pieces to board
  for (unsigned i = 0; i < num_played_letters; ++i) {
    m_game_board->place_piece(the_play.get_ith_row(i), the_play.get_ith_col(i),
                              the_play.get_ith_piece(i));
  }

  //remove those letters from the player's set
  for (unsigned i = 0; i < num_played_letters; ++i) {
    player->remove_piece(the_play.get_ith_piece(i));
  }

  //replenish the player's set if possible
  for (unsigned i = 0; i < num_played_letters; ++i) {
    if (m_piece_source->is_empty()) {
      break;
    }
    player->add_piece(m_piece_source->get_piece());
  }

  //check if play was a non-null play
  if (num_played_letters > 0) {
    //add it to the log
    ostringstream ss;
    ss << player->get_name() << " scored " << m_potential_score
       << " with words: " << m_potential_words;
    m_msg_log.insert(m_msg_log.begin(), std::string(ss.str()));
    //we no longer have a 'virgin' board
    m_first_play = false;

    if (m_config.OUTPUT() == GUI && !player->is_human()) {
      for (unsigned i = 0; i < num_played_letters; ++i) {
        m_row_buff[i] = the_play.get_ith_row(i);
        m_col_buff[i] = the_play.get_ith_col(i);
        m_let_buff[i] = the_play.get_ith_piece(i)->get_letter();
      }
      bool worked = m_config.PY_CALLBACK()(PLAY, num_played_letters, m_row_buff, m_col_buff, m_let_buff);
      my_static_assert(worked, "GUI failure");
    }
  }

  //if player has 0 pieces, the game is over
  if (player->get_num_pieces() == 0) {
    m_game_over = true;
    //notify all players that the game is over. They will at least deduct the pts they're holding
    for (unsigned i = 0; i < m_players.size(); ++i) {
      m_players[i]->game_over();
    }
    //record this event in the log
    m_msg_log.insert(m_msg_log.begin(), player->get_name() + " has played their last piece. " +
                     "GAME OVER");
  }
}

////////////////////////////////////////////////////////////////////////////////
unsigned Scrabble_Game::get_potential_score(const Indv_Play& the_play) const
////////////////////////////////////////////////////////////////////////////////
{
#ifndef NDEBUG
  std::string rv =
#endif
    evaluate_play(the_play); //piggy back on evaluate_play
  //since this method is called by AIs, expect valid plays
  my_assert(rv == "", std::string("AI attempted play: ") + obj_to_str(the_play) +
            ", which failed because: " + rv);

  return m_potential_score;
}

////////////////////////////////////////////////////////////////////////////////
ostream& Scrabble_Game::operator<<(ostream& out) const
////////////////////////////////////////////////////////////////////////////////
{
  if (m_config.CLEAR_SCREEN_BEFORE_OUTPUT() && !m_game_over) {
    system("clear");
  }
  const bool add_color = m_config.COLOR_OUTPUT();

  out << "================================================================================\n";
  out << "Scores: \n";
  for (unsigned i = 0; i < m_players.size(); ++i) {
    out << m_players[i]->get_name() << ": " << m_players[i]->get_score() << "\n";
  }
  out << "================================================================================\n";
  out << *m_game_board << "\n";
  out << "================================================================================\n";
  out << "Recent Events: \n";
  //print the entire log when the game is over
  unsigned max_log_msgs = m_game_over ? 1000 : m_config.MAX_NUM_LOG_MSGS_TO_DISPL();
  for (unsigned i = 0; i < m_msg_log.size() && i < max_log_msgs; ++i) {
    if (add_color) {
      out << "\033[1;31m";
    }
    out << m_msg_log[i];
    if (add_color) {
      out << "\033[0m";
    }
    out << "\n";
  }
  out << "================================================================================\n";

  return out;
}

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Game::save(const std::string& filename) const
////////////////////////////////////////////////////////////////////////////////
{
  ofstream out(filename.c_str());
  my_assert(out, std::string("Unable to open save file: ") + filename);

  const bool orig_color = m_config.COLOR_OUTPUT();
  m_config.disable_colors();

  out << "Config\n";
  out << m_config << "\n";
  out << "Players\n";
  for (auto player : m_players) {
    out << *player << "\n";
  }
  out << "Board\n";
  out << *m_game_board << "\n";

  out.close();

  if (orig_color) m_config.enable_colors();
}

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Game::load(std::istream& in)
////////////////////////////////////////////////////////////////////////////////
{
  std::string line;

  getline(in, line);
  my_require(line == "Players", std::string("Bad line: ") + line);

  for (auto player : m_players) {
    in >> *player;
  }

  getline(in, line);
  my_require(line == "Board", std::string("Bad line: ") + line);

  // in >> *m_game_board;

  // Remove played pieces from source

  // If any pieces played, we are not on the first play
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Scrabble_Game& sg)
////////////////////////////////////////////////////////////////////////////////
{
  return sg.operator<<(out);
}
