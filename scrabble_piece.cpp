#include "scrabble_piece.hpp"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Scrabble_Piece& sp)
////////////////////////////////////////////////////////////////////////////////
{
  return sp.operator<<(out);
}
