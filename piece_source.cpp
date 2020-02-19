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
const Scrabble_Piece* Piece_Source::get_piece(char value)
///////////////////////////////////////////////////////////////////////////////
{
  unsigned swap_idx = static_cast<unsigned>(-1);
  for (unsigned i = m_curr_idx; i < m_source.size(); ++i) {
    if (m_source[i]->get_letter() == value) {
      swap_idx = i;
    }
  }

  my_static_assert(swap_idx != static_cast<unsigned>(-1), std::string("Requested unavailable value: ") + value);

  const Scrabble_Piece* tmp = m_source[m_curr_idx];
  m_source[m_curr_idx] = m_source[swap_idx];
  m_source[swap_idx] = tmp;

  return m_source[m_curr_idx++];
}

