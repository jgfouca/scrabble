#include "human_player.h"

#include <iostream>
#include <stdio.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
void Human_Player::make_play()
////////////////////////////////////////////////////////////////////////////////
{
  //provide some info to the user as to how to formulate a command
  cout << "Commands: \"play <row> <col> <word> <is-horiz(y/n)>\"\n";
  cout << "  (Use '_' to represent components of your word that are already on the board\n";

  //show the user what pieces they have
  cout << "Available pieces: ";
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    if (m_pieces[i]) {
      cout << m_pieces[i]->get_letter() << "(" << m_pieces[i]->get_point_val() << ")  ";
    }
  }
  cout << endl;

  //loop until we see a valid command
  std::string command;
  bool flawed_command = false;
  while (true) {
    unsigned row, col; //row,col are where the player's move begin
    char is_horiz;     //is_horiz - specifies if the player is playing a horizontal word
    char word[50];     //buffer to hold <word>
    flawed_command = false; //specifies if we have confirmed the last command is invalid

    m_current_play.clear(); //ensure the last play is cleared
    remap();                //remapping refreshes the char-map

    cout << "> ";
    if (!getline(cin, command)) { //grab a line of text
      cout << "please enter command" << endl;
      continue;
    }
    else if (command.find("play ") == 0) {
      //scanf based on the expected command format
      int rv = sscanf(command.c_str(), "play %d %d %s %c", &row, &col, word, &is_horiz);
      if (rv != 4) {
        //the grabbing of one or more essential values from the command failed
        cout << "play command not formatted properly, try again" << endl;
      }
      else {
        if (is_horiz != 'y' && is_horiz != 'n') {
          //is_horiz did not have an expected value
          cout << "The fourth argument must be either 'y' or 'n'" << endl;
          continue;
        }

        //loop over the word they are trying to create
        std::string word_str(word);
        for (unsigned i = 0; i < word_str.size(); ++i) {
          //anything besides '_' requires a piece from the player
          if (word_str[i] != '_') {
            const Scrabble_Piece* piece = NULL;
            //attempt to get the piece from the player's stash
            try {
              piece = get_piece(word_str[i]);
            }
            catch (Scrabble_Exception& e) {
              cout << "INVALID_PLAY: " << e.message() << endl;
              flawed_command = true;
              break;
            }
            //this part of the play succeeded, add it to m_current_play
            m_current_play.place_piece(row + (is_horiz == 'y' ? 0 : i),
                                       col + (is_horiz == 'y' ? i : 0),
                                       piece);
          }
        }
        if (!flawed_command) {
          //if the last command was totally valid, we are done
          break;
        }
      }
    }
    else {
      cout << "Unknown command, try again" << endl;
    }
  }
}
