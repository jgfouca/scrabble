#include "scrabble_game.h"
#include "scrabble_word.h"
#include "player.h"
#include "scrabble_config.h"

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
  std::string dict_name = Scrabble_Config::instance().DICTIONARY();
  ifstream in(dict_name.c_str());
  my_assert(in, std::string("Unable to open dictionary-file: ") + dict_name);
  std::string word;
  while (in >> word) {
    m_valid_words.insert(word);
  }
  in.close();

  //give players their initial set of pieces
  unsigned num_pieces_per_player = Scrabble_Config::instance().NUM_PLAYER_PIECES();
  for (unsigned i = 0; i < m_players.size(); i++) {
    m_players[i]->initialize();
    for (unsigned p = 0; p < num_pieces_per_player; p++) {
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
  bool produce_output = Scrabble_Config::instance().PRODUCE_OUTPUT();

  //continue having players make plays until the game is over
  while (!m_game_over) {
    //need to track if all players make a null move, this implies the game is stuck and needs to end
    bool not_all_null = false; 
    //begin a new round of plays (loop over each player, have them play once)
    for (unsigned i = 0; i < m_players.size() && !m_game_over; i++) {
      //display the state of the game
      if (produce_output) {
        cout << *this << endl;
      }
      
      //this player will go until he has made a valid move
      //(placing no letters is a valid move (equiv to skip)
      while (true) {
        const Indv_Play& this_play = m_players[i]->play();
        
        std::string err_str = evaluate_play(this_play);
        if (err_str == "") {
          if (this_play.get_size() > 0) {
            not_all_null = true;
          }

          //play was legit, process it
          process_legit_play(this_play, m_players[i]);
          break;
        }
        else {
          if (produce_output) {
            cout << err_str << endl;
          }
        }
      }
    }
    if (!not_all_null) {
      cout << "NO PLAYER COULD MOVE, GAME OVER" << endl;
      m_game_over = true;
    }
  }

  //game is over. produce one final output of the ending state of the game
  if (produce_output) {
    cout << *this << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string Scrabble_Game::evaluate_play(const Indv_Play& the_play) const
////////////////////////////////////////////////////////////////////////////////
{
  const unsigned num_played_letters = the_play.get_size();
  const unsigned b_dim              = m_game_board->get_board_dim();

  //null plays are always allowed
  if (num_played_letters == 0) {
    return "";
  }

  //if first play, one piece must cover the 7,7 square
  if (m_first_play) {
    bool hit_center = false;
    for (unsigned i = 0; i < num_played_letters; ++i) {
      if (the_play.get_ith_row(i) == 7 && the_play.get_ith_col(i) == 7) {
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
  m_potential_score = 0;
  m_potential_words = "";
  const std::vector<Scrabble_Word> new_words = m_game_board->get_created_words(the_play);
  for (unsigned i = 0; i < new_words.size(); ++i) {
    if (m_valid_words.find(new_words[i].get_word_str()) == m_valid_words.end()) {
      return std::string(std::string("MOVE ") + obj_to_str(the_play) + " REJECTED: \"") + new_words[i].get_word_str() + "\" is not a valid word";
    }
    else {
      m_potential_score += new_words[i].score();
      m_potential_words += new_words[i].get_word_str() + " ";
    }
  }
  //don't forget the bonus for playing all 7 letters
  if (the_play.get_size() == 7) {
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
  if (Scrabble_Config::instance().CLEAR_SCREEN_BEFORE_OUTPUT() && !m_game_over) {
    system("clear");
  }
  const bool add_color = Scrabble_Config::instance().COLOR_OUTPUT();

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
  unsigned max_log_msgs = m_game_over ? 1000 : Scrabble_Config::instance().MAX_NUM_LOG_MSGS_TO_DISPL();
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
ostream& operator<<(ostream& out, const Scrabble_Game& sg)
////////////////////////////////////////////////////////////////////////////////
{
  return sg.operator<<(out);
}
