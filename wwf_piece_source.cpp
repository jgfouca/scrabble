#include "wwf_piece_source.hpp"
#include "scrabble_point_map.hpp"

#include <cstdlib>
#include <ctime>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
Wwf_Piece_Source::Wwf_Piece_Source() : m_curr_idx(0)
////////////////////////////////////////////////////////////////////////////////
{
  m_source.reserve(104);
  const auto& wwf_pm = Wwf_Point_Map::instance();

  //create and add the pieces to the vector
  for (unsigned i = 13; i > 0; i--) {
    m_source.push_back(new Scrabble_Piece('E', wwf_pm));
    if (i <= 9) {
      m_source.push_back(new Scrabble_Piece('A', wwf_pm));
    }
    if (i <= 8) {
      m_source.push_back(new Scrabble_Piece('I', wwf_pm));
      m_source.push_back(new Scrabble_Piece('O', wwf_pm));
    }
    if (i <= 7) {
      m_source.push_back(new Scrabble_Piece('T', wwf_pm));
    }
    if (i <= 6) {
      m_source.push_back(new Scrabble_Piece('R', wwf_pm));
    }
    if (i <= 5) {
      m_source.push_back(new Scrabble_Piece('D', wwf_pm));
      m_source.push_back(new Scrabble_Piece('N', wwf_pm));
      m_source.push_back(new Scrabble_Piece('S', wwf_pm));
    }
    if (i <= 4) {
      m_source.push_back(new Scrabble_Piece('H', wwf_pm));
      m_source.push_back(new Scrabble_Piece('L', wwf_pm));
      m_source.push_back(new Scrabble_Piece('U', wwf_pm));
    }
    if (i <= 3) {
      m_source.push_back(new Scrabble_Piece('G', wwf_pm));
    }
    if (i <= 2) {
      m_source.push_back(new Scrabble_Piece('B', wwf_pm));
      m_source.push_back(new Scrabble_Piece('C', wwf_pm));
      m_source.push_back(new Scrabble_Piece('F', wwf_pm));
      m_source.push_back(new Scrabble_Piece('M', wwf_pm));
      m_source.push_back(new Scrabble_Piece('P', wwf_pm));
      m_source.push_back(new Scrabble_Piece('V', wwf_pm));
      m_source.push_back(new Scrabble_Piece('W', wwf_pm));
      m_source.push_back(new Scrabble_Piece('Y', wwf_pm));
      m_source.push_back(new Scrabble_Piece('-', wwf_pm));
    }
    if (i == 1) {
      m_source.push_back(new Scrabble_Piece('J', wwf_pm));
      m_source.push_back(new Scrabble_Piece('K', wwf_pm));
      m_source.push_back(new Scrabble_Piece('Q', wwf_pm));
      m_source.push_back(new Scrabble_Piece('X', wwf_pm));
      m_source.push_back(new Scrabble_Piece('Z', wwf_pm));
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
