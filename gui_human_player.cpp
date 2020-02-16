#include "gui_human_player.hpp"
#include "scrabble_piece.hpp"

#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
void GUI_Human_Player::make_play()
////////////////////////////////////////////////////////////////////////////////
{
  //loop until we see a valid command
  std::string command, tiles;

  while(!m_the_game->get_config().PY_CALLBACK()(CHECK_PLAY, 0, m_row_buff, m_col_buff, m_let_buff)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check for non-standard events here
    if (m_the_game->get_config().PY_CALLBACK()(CHECK_HINT, 0, m_row_buff, m_col_buff, m_let_buff)) {
      int tile_len = m_row_buff[0];
      tiles = string(&m_let_buff[0], &m_let_buff[tile_len]);
      set_tray(tiles);

      m_current_play.clear(); //ensure the last play is cleared
      remap();                //remapping refreshes the char-map

      AI_Player::make_play();

      const unsigned num_played_letters = m_current_play.get_size();
      for (unsigned i = 0; i < num_played_letters; ++i) {
        m_row_buff[i] = m_current_play.get_ith_row(i);
        m_col_buff[i] = m_current_play.get_ith_col(i);
        m_let_buff[i] = m_current_play.get_ith_piece(i)->get_letter();
      }
      bool worked = m_the_game->get_config().PY_CALLBACK()(GIVE_HINT, num_played_letters, m_row_buff, m_col_buff, m_let_buff);
      my_static_assert(worked, "GUI failure");
    }
    else if (m_the_game->get_config().PY_CALLBACK()(CHECK_SAVE, 0, m_row_buff, m_col_buff, m_let_buff)) {
      int file_len = m_row_buff[0];
      string filename = string(&m_let_buff[0], &m_let_buff[file_len]);
      m_the_game->save(filename);
    }
  }

  int cmd_len = m_row_buff[0];
  command = string(&m_let_buff[0], &m_let_buff[0] + cmd_len);

  // GUI user may have change their tiles
  int tile_len = m_row_buff[1];
  tiles = string(&m_let_buff[cmd_len], &m_let_buff[cmd_len] + tile_len);
  set_tray(tiles);

  unsigned row, col; //row,col are where the player's move begin
  char is_horiz;     //is_horiz - specifies if the player is playing a horizontal word
  char force;        //force - specifies that the word should be played even if it's not recognized
  char word[64];     //buffer to hold <word>

  m_current_play.clear(); //ensure the last play is cleared
  remap();                //remapping refreshes the char-map

  if (command.find("play ") == 0) {
    //scanf based on the expected command format
    int rv = sscanf(command.c_str(), "play %d %d %s %c %c", &row, &col, word, &is_horiz, &force);
    my_require(rv == 5, "play command not formatted properly, try again");
    my_require(is_horiz == 'y' || is_horiz == 'n', "The fourth argument must be either 'y' or 'n'");
    my_require(force == 'y' || force == 'n', "The fifth argument must be either 'y' or 'n'");

    if (force == 'y') {
      m_current_play.force();
    }

    //loop over the word they are trying to create
    std::string word_str(word);
    for (unsigned i = 0; i < word_str.size(); ++i) {
      //anything besides '_' requires a piece from the player
      if (word_str[i] != '_') {
        const Scrabble_Piece* piece = NULL;
        piece = get_piece(word_str[i]);
        //this part of the play succeeded, add it to m_current_play
        m_current_play.place_piece(row + (is_horiz == 'y' ? 0 : i),
                                   col + (is_horiz == 'y' ? i : 0),
                                   piece);
      }
    }
  }
  else {
    cout << "Unknown command, try again" << endl;
  }
}
