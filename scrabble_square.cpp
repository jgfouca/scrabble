#include "scrabble_square.hpp"
#include "scrabble_exception.hpp"

#include <cassert>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Square::add_piece(const Scrabble_Piece* piece)
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(piece, "Tried to add NULL piece");
  my_assert(is_free(), "Tried to add piece to non-null square.");

  m_piece = piece;
  m_piece->played();
}

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Square::set_bonus(Bonus bonus)
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(m_bonus == NONE, "Bonus had already been set");
  my_assert(bonus != NONE, "No need to set bonus to NONE");

  m_bonus = bonus;
}

////////////////////////////////////////////////////////////////////////////////
ostream& Scrabble_Square::operator<<(ostream& out) const
////////////////////////////////////////////////////////////////////////////////
{
  out << "| ";
  if (m_piece) {
    out << " " << *m_piece << " ";
  }
  else {
#ifndef SCRABBLE_PLAIN_OUTPUT
    out << "\033[1;34m";
#endif
    out << m_bonus;
#ifndef SCRABBLE_PLAIN_OUTPUT
    out << "\033[0m";
#endif
  }
  out << " ";

  return out;
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Scrabble_Square& sq)
////////////////////////////////////////////////////////////////////////////////
{
  return sq.operator<<(out);
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Bonus& b)
////////////////////////////////////////////////////////////////////////////////
{
  switch(b) {
  case NONE:
    out << "   ";
    break;
  case DBL_LET:
    out << "b2l";
    break;
  case TRP_LET:
    out << "b3l";
    break;
  case DBL_WRD:
    out << "b2w";
    break;
  case TRP_WRD:
    out << "b3w";
    break;
  default:
    //cannot have asserts within operator<<; causes stack overflow because
    //my_assert calls << 
    //my_static_assert(false, "Missing case for some bonus.");
    assert(false);
  }
  return out;
}
