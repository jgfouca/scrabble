#include "standard_piece_source.hpp"

#include <cstdlib>
#include <ctime>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
Standard_Piece_Source::Standard_Piece_Source() : m_curr_idx(0)
////////////////////////////////////////////////////////////////////////////////
{
  m_source.reserve(100);
  const auto& scr_pm = Scrabble_Point_Map::instance();

  //create and add the pieces to the vector
  for (unsigned i = 12; i > 0; i--) {
    m_source.push_back(new Scrabble_Piece('E', scr_pm));
    if (i <= 9) {
      m_source.push_back(new Scrabble_Piece('A', scr_pm));
      m_source.push_back(new Scrabble_Piece('I', scr_pm));
    }
    if (i <= 8) {
      m_source.push_back(new Scrabble_Piece('O', scr_pm));
    }
    if (i <= 6) {
      m_source.push_back(new Scrabble_Piece('N', scr_pm));
      m_source.push_back(new Scrabble_Piece('R', scr_pm));
      m_source.push_back(new Scrabble_Piece('T', scr_pm));
    }
    if (i <= 4) {
      m_source.push_back(new Scrabble_Piece('L', scr_pm));
      m_source.push_back(new Scrabble_Piece('S', scr_pm));
      m_source.push_back(new Scrabble_Piece('U', scr_pm));
      m_source.push_back(new Scrabble_Piece('D', scr_pm));
    }
    if (i <= 3) {
      m_source.push_back(new Scrabble_Piece('G', scr_pm));
    }
    if (i <= 2) {
      m_source.push_back(new Scrabble_Piece('B', scr_pm));
      m_source.push_back(new Scrabble_Piece('C', scr_pm));
      m_source.push_back(new Scrabble_Piece('M', scr_pm));
      m_source.push_back(new Scrabble_Piece('P', scr_pm));
      m_source.push_back(new Scrabble_Piece('F', scr_pm));
      m_source.push_back(new Scrabble_Piece('H', scr_pm));
      m_source.push_back(new Scrabble_Piece('V', scr_pm));
      m_source.push_back(new Scrabble_Piece('W', scr_pm));
      m_source.push_back(new Scrabble_Piece('Y', scr_pm));
      m_source.push_back(new Scrabble_Piece('-', scr_pm));
    }
    if (i == 1) {
      m_source.push_back(new Scrabble_Piece('K', scr_pm));
      m_source.push_back(new Scrabble_Piece('J', scr_pm));
      m_source.push_back(new Scrabble_Piece('X', scr_pm));
      m_source.push_back(new Scrabble_Piece('Q', scr_pm));
      m_source.push_back(new Scrabble_Piece('Z', scr_pm));
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
Standard_Piece_Source::~Standard_Piece_Source()
////////////////////////////////////////////////////////////////////////////////
{
  for (unsigned i = 0; i < m_source.size(); i++) {
    delete m_source[i];
  }
}
