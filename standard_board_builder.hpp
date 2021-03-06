#ifndef standard_board_builder_h
#define standard_board_builder_h

#include "board_builder.hpp"

class Scrabble_Game;
class Scrabble_Board;

/**
 * This class implements the Board_Builder interface and is designed to
 * construct a standard scrabble board.
 *
 * Note that this class is a singleton.
 */

////////////////////////////////////////////////////////////////////////////////
class Standard_Board_Builder : public Board_Builder
////////////////////////////////////////////////////////////////////////////////
{
 public:

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Standard_Board_Builder(const Scrabble_Game& parent) : Board_Builder(parent) {}

  virtual void build_board(Scrabble_Board* board) const;

  static const unsigned BOARD_DIM = 15;

 private: // ================ PRIVATE INTERFACE ================================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Standard_Board_Builder(const Standard_Board_Builder&);
  Standard_Board_Builder& operator=(const Standard_Board_Builder&);
};

#endif
