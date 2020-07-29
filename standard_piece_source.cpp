#include "standard_piece_source.hpp"

#include <cstdlib>
#include <ctime>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
Standard_Piece_Source::Standard_Piece_Source(const Scrabble_Game& parent)
  : Piece_Source(parent)
////////////////////////////////////////////////////////////////////////////////
{
  m_source.reserve(100);
  const auto& scr_pm = get_point_map();

  //create and add the pieces to the vector
  for (unsigned i = 12; i > 0; i--) {
    m_source.push_back(new Scrabble_Piece(m_parent, 'E', scr_pm));
    if (i <= 9) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'A', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'I', scr_pm));
    }
    if (i <= 8) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'O', scr_pm));
    }
    if (i <= 6) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'N', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'R', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'T', scr_pm));
    }
    if (i <= 4) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'L', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'S', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'U', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'D', scr_pm));
    }
    if (i <= 3) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'G', scr_pm));
    }
    if (i <= 2) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'B', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'C', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'M', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'P', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'F', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'H', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'V', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'W', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'Y', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, '-', scr_pm));
    }
    if (i == 1) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'K', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'J', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'X', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'Q', scr_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'Z', scr_pm));
    }
  }
  my_static_assert(m_source.size() == 100,
                   std::string("m_source is size ") + obj_to_str(m_source.size()) +
                   ", expected size 100");

  //do a couple shuffles
  for (unsigned j = 0; j < 2; j++) {
    for (unsigned i = 0; i < m_source.size(); i++) {
      unsigned rand_idx = rand() % m_source.size();
      const Scrabble_Piece* tmp = m_source[rand_idx];
      m_source[rand_idx] = m_source[i];
      m_source[i] = tmp;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
const Point_Map& Standard_Piece_Source::get_point_map() const
////////////////////////////////////////////////////////////////////////////////
{
  return Scrabble_Point_Map::instance();
}
