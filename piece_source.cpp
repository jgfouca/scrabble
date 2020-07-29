#include "piece_source.hpp"

///////////////////////////////////////////////////////////////////////////////
Piece_Source::~Piece_Source()
///////////////////////////////////////////////////////////////////////////////
{
  for (unsigned i = 0; i < m_source.size(); i++) {
    delete m_source[i];
  }
}

///////////////////////////////////////////////////////////////////////////////
const Scrabble_Piece* Piece_Source::get_piece()
///////////////////////////////////////////////////////////////////////////////
{
  my_static_assert(!is_empty(), "Tried to get_piece from an empty source");
  return m_source[m_curr_idx++]; //return current piece and iterate
}

///////////////////////////////////////////////////////////////////////////////
const Scrabble_Piece* Piece_Source::get_piece(char value, bool tolerate_overflow)
///////////////////////////////////////////////////////////////////////////////
{
  static constexpr unsigned not_found = static_cast<unsigned>(-1);
  unsigned swap_idx = not_found;
  for (unsigned i = m_curr_idx; i < m_source.size(); ++i) {
    if (m_source[i]->get_letter() == value) {
      swap_idx = i;
    }
  }

  if (tolerate_overflow) {
    if (swap_idx == not_found) {
      // Create the missing piece and add it to the source. This creates a
      // situation where the game now has more pieces that it normally would,
      // but this is the best we can do.
      const auto& scr_pm = get_point_map();
      m_source.push_back(new Scrabble_Piece(m_parent, value, scr_pm));
      swap_idx = m_source.size() - 1;
    }
  }
  else {
    my_static_assert(swap_idx != not_found, std::string("Requested unavailable value: ") + value);
  }

  const Scrabble_Piece* tmp = m_source[m_curr_idx];
  m_source[m_curr_idx] = m_source[swap_idx];
  m_source[swap_idx] = tmp;

  return m_source[m_curr_idx++];
}

