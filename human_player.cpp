#include "human_player.hpp"
#include "scrabble_piece.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
void Human_Player::make_play()
////////////////////////////////////////////////////////////////////////////////
{
  //provide some info to the user as to how to formulate a command
  cout << "Commands: \n";
  cout << "  \"play <row> <col> <word> <is-horiz(y/n)> <force(y/n)>\"\n";
  cout << "  (Use '_' to represent components of your word that are already on the board\n\n";
  cout << "  \"set-tray <letters>\"\n";
  cout << "  Example: 'set-tray AES-TRU'\n\n";
  cout << "  \"recommend-play\"\n";
  cout << "  AI will search board and find the best play." << endl;

  //loop until we see a valid command
  std::string command;
  bool flawed_command = false;
  while (true) {
    //show the user what pieces they have
    cout << "Available pieces: ";
    for (unsigned i = 0; i < m_pieces.size(); i++) {
      if (m_pieces[i]) {
        cout << m_pieces[i]->get_letter() << "(" << m_pieces[i]->get_point_val() << ")  ";
      }
    }
    cout << endl;

    unsigned row, col; //row,col are where the player's move begin
    char is_horiz;     //is_horiz - specifies if the player is playing a horizontal word
    char force;        //force - specifies that the word should be played even if it's not recognized
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
      //The code below was copied from human_player.cc. There didn't seem to be any elegant
      //way to encapsulate and reuse it.

      //scanf based on the expected command format
      int rv = sscanf(command.c_str(), "play %d %d %s %c %c", &row, &col, word, &is_horiz, &force);
      if (rv != 5) {
        //the grabbing of one or more essential values from the command failed
        cout << "play command not formatted properly, try again" << endl;
      }
      else {
        if (is_horiz != 'y' && is_horiz != 'n') {
          //is_horiz did not have an expected value
          cout << "The fourth argument must be either 'y' or 'n'" << endl;
          continue;
        }

        if (force != 'y' && force != 'n') {
          //is_horiz did not have an expected value
          cout << "The fifth argument must be either 'y' or 'n'" << endl;
          continue;
        }

        if (force == 'y') {
          m_current_play.force();
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
    else if (command.find("set-tray ") == 0) {
      int rv = sscanf(command.c_str(), "set-tray %s", word);
      if (rv != 1) {
        //the grabbing of one or more essential values from the command failed
        cout << "set-tray command not formatted properly, try again" << endl;
      }
      else {
        std::string new_letters(word);

        if (new_letters.size() > get_num_pieces()) {
          cout << "You tried to set too many pieces, player only has "
               << get_num_pieces() << " left to set." << endl;
          continue;
        }

        //loop over the letters they specified
        unsigned piece_itr = 0;
        for (unsigned i = 0; i < new_letters.size(); ++i) {
          char letter = new_letters[i];
          if (!(Scrabble_Piece::is_valid_letter(letter) || letter == '-')) {
            cout << "'" << letter << "' is not a valid letter." << endl;
            break;
          }
          //skip NULL pieces
          while (!m_pieces[piece_itr]) {
            ++piece_itr;
          }
          //force piece change
          Scrabble_Piece* piece = const_cast<Scrabble_Piece*>(m_pieces[piece_itr++]);
          piece->force_letter_change(letter);
        }
      }
    }
    else if (command.find("recommend-play") == 0) {
      AI_Player::make_play();
      cout << "AI recommends: " << m_current_play << endl;
    }
    else {
      cout << "Unknown command, try again" << endl;
    }
  }
}
