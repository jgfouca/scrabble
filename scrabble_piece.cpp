#include "scrabble_piece.hpp"
#include "scrabble_game.hpp"

#include <cstdio>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
std::ostream& Scrabble_Piece::operator<<(std::ostream& out) const
////////////////////////////////////////////////////////////////////////////////
{
  int color_code = (m_been_output) ? 29 : 31;
  const char letter = get_letter();
  if (is_wildcard()) {
    color_code = 32;
  }
  const bool use_color = m_parent.get_config().COLOR_OUTPUT();
  if (use_color) {
    out << "\033[1;" << color_code << "m";
  }
  if (use_color) {
    out << letter;
    out << "\033[0m";
  }
  else {
    out << get_encoded_letter();
  }
  m_been_output = true;
  return out;
}

////////////////////////////////////////////////////////////////////////////////
char Scrabble_Piece::get_encoded_letter() const
////////////////////////////////////////////////////////////////////////////////
{
  if (is_wildcard() && m_wildcard_choice != '-') {
    return std::tolower(get_letter());
  }
  else {
    return get_letter();
  }
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Scrabble_Piece& sp)
////////////////////////////////////////////////////////////////////////////////
{
  return sp.operator<<(out);
}
