#ifndef standard_piece_source_h
#define standard_piece_source_h

#include "scrabble_piece.hpp"
#include "scrabble_exception.hpp"
#include "piece_source.hpp"

#include <vector>

class Scrabble_Game;

/**
 * This class implements a piece source that you'd expect in a standard scrabble
 * game. It has 100 pieces with the standard letter distributions:
 * 12 E
 * 9  A I
 * 8  O
 * 6  N R T
 * 4  L S U D
 * 3  G
 * 2  B C M P F H V W Y -
 * 1  K J X Q Z
 *
 * This class works by creating all the pieces above, placing them in a vector
 * and then shuffling the vector so that the piece order is random. As the
 * outside requests pieces, we simply iterate over this vector.
 */

////////////////////////////////////////////////////////////////////////////////
class Standard_Piece_Source : public Piece_Source
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - Populates the piece source
   */
  Standard_Piece_Source(const Scrabble_Game& parent);

 private: // ================ PRIVATE INTERFACE ================================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Standard_Piece_Source(const Standard_Piece_Source&) = delete;
  Standard_Piece_Source& operator=(const Standard_Piece_Source&) = delete;
};

#endif
