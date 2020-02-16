#include "scrabble_piece.hpp"
#include "scrabble_game.hpp"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
std::ostream& Scrabble_Piece::operator<<(std::ostream& out) const
////////////////////////////////////////////////////////////////////////////////
{
  int color_code = (m_been_output) ? 29 : 31;
  if (is_wildcard()) {
    color_code = 32;
  }
  const bool use_color = m_parent.get_config().COLOR_OUTPUT();
  if (use_color) {
    out << "\033[1;" << color_code << "m";
  }
  out << get_letter();
  if (use_color) {
    out << "\033[0m";
  }
  m_been_output = true;
  return out;
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Scrabble_Piece& sp)
////////////////////////////////////////////////////////////////////////////////
{
  return sp.operator<<(out);
}
