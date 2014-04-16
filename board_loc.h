#ifndef board_loc_h
#define board_loc_h

#include <iostream>

/**
 * This class represents a position on the board. Since it is easy enough to 
 * work with 2 ints, this class is only used where I was using a pair before. 
 * The main point was to be able to use "row()" and "col()" instead of "first" 
 * and "second".
 */

////////////////////////////////////////////////////////////////////////////////
class Board_Loc
////////////////////////////////////////////////////////////////////////////////
{
 public:
  Board_Loc(unsigned r, unsigned c) : m_row(r), m_col(c) {}

  unsigned row() const {return m_row;}

  unsigned col() const {return m_col;}

  std::ostream& operator<<(std::ostream& out) const;

  bool operator<(const Board_Loc& rhs) const;

 private:
  unsigned m_row;
  unsigned m_col;
};

std::ostream& operator<<(std::ostream& out, const Board_Loc& bl);

#endif
