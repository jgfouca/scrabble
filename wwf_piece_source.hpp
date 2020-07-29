#ifndef wwf_piece_source_h
#define wwf_piece_source_h

#include "scrabble_piece.hpp"
#include "scrabble_exception.hpp"
#include "piece_source.hpp"

#include <vector>

class Scrabble_Game;
class Point_Map;

/**
 * This class implements a piece source that you'd expect in a wwf scrabble
 * game. It has 100 pieces with the wwf letter distributions:
 *
 * 13 E
 * 9 A
 * 8 I O
 * 7 T
 * 6 R
 * 5 D N S
 * 4 H L U
 * 3 G
 * 2 B C F M P V W Y -
 * 1 J K Q X Z
 *
 * This class works by creating all the pieces above, placing them in a vector
 * and then shuffling the vector so that the piece order is random. As the
 * outside requests pieces, we simply iterate over this vector.
 */

////////////////////////////////////////////////////////////////////////////////
class Wwf_Piece_Source : public Piece_Source
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - Populates the piece source
   */
  Wwf_Piece_Source(const Scrabble_Game& parent);

  virtual const Point_Map& get_point_map() const;

 private: // ================ PRIVATE INTERFACE ================================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Wwf_Piece_Source(const Wwf_Piece_Source&) = delete;
  Wwf_Piece_Source& operator=(const Wwf_Piece_Source&) = delete;
};

#endif
