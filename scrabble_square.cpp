#include "scrabble_square.hpp"
#include "scrabble_exception.hpp"
#include "scrabble_game.hpp"

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
    const int color_code = get_color_code();
    const bool use_color = m_parent != nullptr && m_parent->get_config().COLOR_OUTPUT();
    if (use_color) {
      out << "\033[1;" << color_code << "m";
    }
    out << m_bonus;
    if (use_color) {
      out << "\033[0m";
    }
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
    my_static_assert(false, "Missing case for some bonus.");
  }
  return out;
}

////////////////////////////////////////////////////////////////////////////////
int Scrabble_Square::get_color_code() const
////////////////////////////////////////////////////////////////////////////////
{
  switch(m_bonus) {
  case NONE:
    return 29;
  case DBL_LET:
    return 34;
  case TRP_LET:
    return 32;
  case DBL_WRD:
    return 31;
  case TRP_WRD:
    return 33;
  default:
    my_static_assert(false, "Missing case for some bonus.");
  }
  return -1;
}
