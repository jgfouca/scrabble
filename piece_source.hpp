#ifndef piece_source_h
#define piece_source_h

#include "scrabble_piece.hpp"

class Scrabble_Game;
class Point_Map;

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
  Piece_Source(const Scrabble_Game& parent) : m_parent(parent), m_source(), m_curr_idx(0) {}

  /**
   * Destructor
   */
  virtual ~Piece_Source();

  /**
   * get_piece - Returns a fresh, unused piece
   */
  virtual const Scrabble_Piece* get_piece();

  /**
   * get_piece - Returns a fresh, unused piece with a specific value
   */
  virtual const Scrabble_Piece* get_piece(char value, bool tolerate_overflow=false);

  /**
   * is_empty - Returns true if this source has run out of pieces
   */
  virtual bool is_empty() const { return m_curr_idx == m_source.size(); }

  /**
   * is_full - Returns true if this source has all its pieces
   */
  virtual bool is_full() const { return m_curr_idx == 0; }

  /**
   * get_point_map - Returns the point map used by this source
   */
  virtual const Point_Map& get_point_map() const = 0;

 protected:

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Piece_Source(const Piece_Source&) = delete;
  Piece_Source& operator=(const Piece_Source&) = delete;

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  const Scrabble_Game& m_parent;

  // m_source - The vector of piece objects
  std::vector<const Scrabble_Piece*> m_source;

  // m_curr_idx - The index of the next piece to be returned.
  unsigned m_curr_idx;

};

#endif
