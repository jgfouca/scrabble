#include "wwf_piece_source.hpp"
#include "scrabble_point_map.hpp"

#include <cstdlib>
#include <ctime>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
Wwf_Piece_Source::Wwf_Piece_Source(const Scrabble_Game& parent)
  : Piece_Source(parent),
    m_curr_idx(0)
////////////////////////////////////////////////////////////////////////////////
{
  m_source.reserve(104);
  const auto& wwf_pm = Wwf_Point_Map::instance();

  //create and add the pieces to the vector
  for (unsigned i = 13; i > 0; i--) {
    m_source.push_back(new Scrabble_Piece(m_parent, 'E', wwf_pm));
    if (i <= 9) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'A', wwf_pm));
    }
    if (i <= 8) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'I', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'O', wwf_pm));
    }
    if (i <= 7) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'T', wwf_pm));
    }
    if (i <= 6) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'R', wwf_pm));
    }
    if (i <= 5) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'D', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'N', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'S', wwf_pm));
    }
    if (i <= 4) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'H', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'L', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'U', wwf_pm));
    }
    if (i <= 3) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'G', wwf_pm));
    }
    if (i <= 2) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'B', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'C', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'F', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'M', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'P', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'V', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'W', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'Y', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, '-', wwf_pm));
    }
    if (i == 1) {
      m_source.push_back(new Scrabble_Piece(m_parent, 'J', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'K', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'Q', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'X', wwf_pm));
      m_source.push_back(new Scrabble_Piece(m_parent, 'Z', wwf_pm));
    }
  }
  my_static_assert(m_source.size() == 104,
                   std::string("m_source is size ") + obj_to_str(m_source.size()) +
                   ", expected size 104");

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
Wwf_Piece_Source::~Wwf_Piece_Source()
////////////////////////////////////////////////////////////////////////////////
{
  for (unsigned i = 0; i < m_source.size(); i++) {
    delete m_source[i];
  }
}
