#include "scrabble_square.hpp"
#include "scrabble_exception.hpp"
#include "scrabble_game.hpp"

#include <algorithm>
#include <sstream>

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
istream& Scrabble_Square::operator>>(istream& in)
////////////////////////////////////////////////////////////////////////////////
{
  auto& piece_source = const_cast<Scrabble_Game*>(m_parent)->get_piece_source();
  constexpr unsigned width_without_sep = OUTPUT_LEN - 1;

  std::string line;
  for (unsigned i = 0; i < width_without_sep; ++i) {
    char c;
    in.read(&c, 1);
    line += c;
  }

  unsigned blanks = std::count(line.begin(), line.end(), ' ');
  if (blanks == width_without_sep) {
    // blank square, do nothing
  }
  else if (blanks == width_without_sep - 1) {
    // it has a piece on it
    char piece_val = line[OUTPUT_LEN / 2 - 1];
    if (islower(piece_val)) {
      auto piece = piece_source.get_piece('-');
      piece->set_wildcard_value(toupper(piece_val));
      add_piece(piece);
    }
    else {
      // We need to tolerate overflow here since the user could have used
      // admin more to create words with more letters of type X than exist
      // in the piece source.
      add_piece(piece_source.get_piece(piece_val, true /*tolerate_overflow*/));
    }
  }
  else if (blanks == width_without_sep - 3) {
    // bonus
    Bonus bonus;
    std::string bonus_str = line.substr(1, 3);
    istringstream iss(bonus_str);
    iss >> bonus;
    //set_bonus(bonus); // should already be set by builder
  }
  else {
    my_static_assert(false, std::string("Could not recognize square: ") + line);
  }

  return in;
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Scrabble_Square& sq)
////////////////////////////////////////////////////////////////////////////////
{
  return sq.operator<<(out);
}

////////////////////////////////////////////////////////////////////////////////
istream& operator>>(istream& in, Scrabble_Square& sq)
////////////////////////////////////////////////////////////////////////////////
{
  return sq.operator>>(in);
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
istream& operator>>(istream& in, Bonus& b)
////////////////////////////////////////////////////////////////////////////////
{
  std::string bonus_str;
  for (unsigned i = 0; i < 3; ++i) {
    char c;
    in >> c;
    bonus_str += c;
  }

  if (bonus_str == "   ") {
    b = NONE;
  }
  else if (bonus_str == "b2l") {
    b = DBL_LET;
  }
  else if (bonus_str == "b3l") {
    b = TRP_LET;
  }
  else if (bonus_str == "b2w") {
    b = DBL_WRD;
  }
  else if (bonus_str == "b3w") {
    b = TRP_WRD;
  }
  else {
    my_static_assert(false, std::string("Unrecognized bonus string: ") + bonus_str );
  }
  return in;
}
