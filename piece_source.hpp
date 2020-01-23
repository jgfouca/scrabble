#ifndef piece_source_h
#define piece_source_h

#include "scrabble_piece.hpp"

/**
 * This abstract class defines the interface that needs to be supported by any
 * piece-source (AKA the "bag" in scrabble). Piece sources must be able to
 * provide fresh pieces and be able to say if the source has run dry.
 */

////////////////////////////////////////////////////////////////////////////////
class Piece_Source
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Destructor
   */
  virtual ~Piece_Source() {}

  /**
   * get_piece - Returns a fresh, unused piece
   */
  virtual const Scrabble_Piece* get_piece() const = 0;

  /**
   * is_empty - Returns true if this source has run out of pieces
   */
  virtual bool is_empty() const = 0;
};

#endif
