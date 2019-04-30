#ifndef superuser_player_h
#define superuser_player_h

#include "ai_player.hpp"

/**
 * This class represents a superuser-controlled player. This class is similar to Human_Player.
 * It implements a make_play as an interactive function. It requests a command
 * from stdin, processes it, and loops until it has receieved a valid command.
 * The difference is that this type of player has many more commands available
 * to it, including many "cheating" ones. 
 */

////////////////////////////////////////////////////////////////////////////////
class Superuser_Player : public AI_Player
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - Simply handsoff args to the parent constructor
   */
  Superuser_Player(const std::string& name, Scrabble_Game* the_game) : AI_Player(name, the_game) {}

 protected:
  /**
   * make_play - Processes stdin commands until it receives a valid command. The
   *             valid command is encoded into m_current_play and the method 
   *             terminates.
   */
  virtual void make_play();
};

#endif
