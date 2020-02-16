#ifndef wwf_solo_board_builder_h
#define wwf_solo_board_builder_h

#include "board_builder.hpp"

class Scrabble_Game;
class Scrabble_Board;

/**
 * This class implements the Board_Builder interface and is designed to
 * construct a words-with-friends-2 vs AI scrabble board.
 *
 * Note that this class is a singleton.
 */

////////////////////////////////////////////////////////////////////////////////
class Wwf_Solo_Board_Builder : public Board_Builder
////////////////////////////////////////////////////////////////////////////////
{
 public:

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Wwf_Solo_Board_Builder(const Scrabble_Game& parent) : Board_Builder(parent) {}

  virtual void build_board(Scrabble_Board* board) const;

  static const unsigned BOARD_DIM = 11;

 private: // ================ PRIVATE INTERFACE ================================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Wwf_Solo_Board_Builder(const Wwf_Solo_Board_Builder&) = delete;
  Wwf_Solo_Board_Builder& operator=(const Wwf_Solo_Board_Builder&) = delete;


  void set_4way_symm(Scrabble_Board* board, unsigned hi_row, unsigned hi_col, int arg_bonus) const;
};

#endif
