#ifndef standard_piece_source_h
#define standard_piece_source_h

#include "scrabble_piece.hpp"
#include "scrabble_exception.hpp"
#include "piece_source.hpp"

#include <vector>

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
  Standard_Piece_Source();

  /**
   * Destructor - Deletes all the pieces
   */
  virtual ~Standard_Piece_Source();

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// QUERIES /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * get_piece - Return the next piece from this source
   */
  virtual const Scrabble_Piece* get_piece() const
  {
    my_static_assert(!is_empty(), "Tried to get_piece from an empty source");
    return m_source[m_curr_idx++]; //return current piece and iterate
  }

  /**
   * is_empty - Returns true if this source has run out of pieces
   */
  virtual bool is_empty() const { return m_curr_idx == m_source.size(); }

 private: // ================ PRIVATE INTERFACE ================================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Standard_Piece_Source(const Standard_Piece_Source&);
  Standard_Piece_Source& operator=(const Standard_Piece_Source&);

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  // m_source - The vector of piece objects
  std::vector<const Scrabble_Piece*> m_source;

  // m_curr_idx - The index of the next piece to be returned.
  mutable unsigned m_curr_idx;
};

#endif
