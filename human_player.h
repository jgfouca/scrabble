#ifndef human_player_h
#define human_player_h

#include "player.h"

/**
 * This class represents a human-controlled player. This class is fairly simple.
 * It implements a make_play as an interactive function. It requests a command
 * from stdin, processes it, and loops until it has receieved a valid command.
 */

////////////////////////////////////////////////////////////////////////////////
class Human_Player : public Player
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - Simply handsoff args to the parent constructor
   */
  Human_Player(const std::string& name, Scrabble_Game* the_game) : Player(name, the_game) {}

 protected:
  /**
   * make_play - Processes stdin commands until it receives a valid command. The
   *             valid command is encoded into m_current_play and the method 
   *             terminates.
   */
  virtual void make_play();
};

#endif
