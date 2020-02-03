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
  std::string command;

  while(!m_the_game->get_config().PY_CALLBACK()(CHECK_PLAY, 0, m_row_buff, m_col_buff, m_let_buff)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  int cmd_len = m_row_buff[0];
  command = string(&m_let_buff[0], &m_let_buff[0] + cmd_len);

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
  // else if (command.find("set-tray ") == 0) {
  //   int rv = sscanf(command.c_str(), "set-tray %s", word);
  //   if (rv != 1) {
  //     //the grabbing of one or more essential values from the command failed
  //     cout << "set-tray command not formatted properly, try again" << endl;
  //   }
  //   else {
  //     std::string new_letters(word);

  //     if (new_letters.size() > get_num_pieces()) {
  //       cout << "You tried to set too many pieces, player only has "
  //            << get_num_pieces() << " left to set." << endl;
  //       continue;
  //     }

  //     //loop over the letters they specified
  //     unsigned piece_itr = 0;
  //     for (unsigned i = 0; i < new_letters.size(); ++i) {
  //       char letter = new_letters[i];
  //       if (!(Scrabble_Piece::is_valid_letter(letter) || letter == '-')) {
  //         cout << "'" << letter << "' is not a valid letter." << endl;
  //         break;
  //       }
  //       //skip NULL pieces
  //       while (!m_pieces[piece_itr]) {
  //         ++piece_itr;
  //       }
  //       //force piece change
  //       Scrabble_Piece* piece = const_cast<Scrabble_Piece*>(m_pieces[piece_itr++]);
  //       piece->force_letter_change(letter);
  //     }
  //   }
  // }
  // else if (command.find("recommend-play") == 0) {
  //   AI_Player::make_play();
  //   cout << "AI recommends: " << m_current_play << endl;
  // }
  // else if (command.find("quit") == 0 || command.find("exit") == 0 || cin.eof()) {
  //   break;
  // }
  else {
    cout << "Unknown command, try again" << endl;
  }
}
