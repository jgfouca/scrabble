#ifndef board_builder_h
#define board_builder_h

class Scrabble_Board;
class Scrabble_Game;

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
  Board_Builder(const Scrabble_Game& parent) : m_parent(parent) {}

  virtual ~Board_Builder() {}

  virtual void build_board(Scrabble_Board* board) const = 0;

  void finish(Scrabble_Board& board) const;

 protected:
  const Scrabble_Game& m_parent;
};

#endif
