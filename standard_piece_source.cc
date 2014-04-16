#include "standard_piece_source.h"

#include <cstdlib>
#include <ctime>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
Standard_Piece_Source::Standard_Piece_Source() : m_curr_idx(0)
////////////////////////////////////////////////////////////////////////////////
{
  m_source.reserve(100);

  //create and add the pieces to the vector
  for (unsigned i = 12; i > 0; i--) {
    m_source.push_back(new Scrabble_Piece('E'));
    if (i <= 9) {
      m_source.push_back(new Scrabble_Piece('A'));
      m_source.push_back(new Scrabble_Piece('I'));
    }
    if (i <= 8) {
      m_source.push_back(new Scrabble_Piece('O'));
    }
    if (i <= 6) {
      m_source.push_back(new Scrabble_Piece('N'));
      m_source.push_back(new Scrabble_Piece('R'));
      m_source.push_back(new Scrabble_Piece('T'));
    }
    if (i <= 4) {
      m_source.push_back(new Scrabble_Piece('L'));
      m_source.push_back(new Scrabble_Piece('S'));
      m_source.push_back(new Scrabble_Piece('U'));
      m_source.push_back(new Scrabble_Piece('D'));
    }
    if (i <= 3) {
      m_source.push_back(new Scrabble_Piece('G'));
    }
    if (i <= 2) {
      m_source.push_back(new Scrabble_Piece('B'));
      m_source.push_back(new Scrabble_Piece('C'));
      m_source.push_back(new Scrabble_Piece('M'));
      m_source.push_back(new Scrabble_Piece('P'));
      m_source.push_back(new Scrabble_Piece('F'));
      m_source.push_back(new Scrabble_Piece('H'));
      m_source.push_back(new Scrabble_Piece('V'));
      m_source.push_back(new Scrabble_Piece('W'));
      m_source.push_back(new Scrabble_Piece('Y'));
      m_source.push_back(new Scrabble_Piece('-'));
    }
    if (i == 1) {
      m_source.push_back(new Scrabble_Piece('K'));
      m_source.push_back(new Scrabble_Piece('J'));
      m_source.push_back(new Scrabble_Piece('X'));
      m_source.push_back(new Scrabble_Piece('Q'));
      m_source.push_back(new Scrabble_Piece('Z'));
    }
  }
  my_static_assert(m_source.size() == 100,
                   Safe_String("m_source is size ") + obj_to_str(m_source.size()) + 
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
