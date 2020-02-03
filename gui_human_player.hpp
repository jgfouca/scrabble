#ifndef gui_human_player_h
#define gui_human_player_h

#include "ai_player.hpp"
#include "player.hpp"

/**
 * This class represents a human-controlled player. This class is fairly simple.
 * It implements a make_play as an interactive function. It requests a command
 * from the GUI
 */

////////////////////////////////////////////////////////////////////////////////
class GUI_Human_Player : public AI_Player
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - Simply hands-off args to the parent constructor
   */
  GUI_Human_Player(const std::string& name, Scrabble_Game* the_game) : AI_Player(name, the_game) {}

 protected:
  /**
   * make_play
   */
  virtual void make_play();

 private:

  unsigned m_row_buff[64];
  unsigned m_col_buff[64];
  char     m_let_buff[64];


};

#endif
