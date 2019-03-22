#ifndef board_builder_h
#define board_builder_h

class Scrabble_Board;

/**
 * This abstract class defines the interface that must be followed by any
 * board-building class. Simply, they must be able to take a board object
 * and build it up.
 */

////////////////////////////////////////////////////////////////////////////////
class Board_Builder
////////////////////////////////////////////////////////////////////////////////
{
 public:
  virtual ~Board_Builder() {}

  virtual void build_board(Scrabble_Board* board) const = 0;
};

#endif
