#ifndef indv_play_h
#define indv_play_h

#include "scrabble_piece.h"
#include "safe_vector.h"
#include "standard_board_builder.h"

#include <algorithm>
#include <iostream>

/**
 * This class represents a single play in the game of scrabble. A single play
 * consists of 0 or more piece placements. A single piece-placement has three
 * pieces of information: row, column, and piece. So, this class simply has
 * three vectors to hold the information of the placements. Equality of index
 * in these vectors implies association with the same placement.
 */

////////////////////////////////////////////////////////////////////////////////
class Indv_Play
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - Simply reserves memory in the three vectors.
   */
  Indv_Play()
  {
    //Note: it is not possible to have a play larger than the dimensions of the board
    m_rows.reserve(Standard_Board_Builder::BOARD_DIM);
    m_cols.reserve(Standard_Board_Builder::BOARD_DIM);
    m_pieces.reserve(Standard_Board_Builder::BOARD_DIM);
  }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  
  /**
   * place_piece - Adds a piece-placement to this play
   */
  void place_piece(unsigned row, unsigned col, const Scrabble_Piece* placed_piece) 
  {
    my_assert(placed_piece, "Tried to play a NULL piece");

    m_rows.push_back(row);
    m_cols.push_back(col);
    m_pieces.push_back(placed_piece);
  }

  /**
   * clear - Reset the state of the play to the empty play
   */
  void clear()
  {
    m_rows.clear();
    m_cols.clear();
    m_pieces.clear();
  }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// QUERIES /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  
  /**
   * get_size - Return the number of piece-placements involved in this play
   */
  unsigned get_size() const 
  {
    my_assert(m_rows.size() == m_cols.size(),   "Row, col vectors out of sync");
    my_assert(m_rows.size() == m_pieces.size(), "Row, piece vectors out of sync");
    
    return m_rows.size(); 
  }

  /**
   * get_ith_row - Return the row component of the i-th piece placement
   */
  unsigned get_ith_row(unsigned idx) const 
  {
    my_assert(idx < get_size(), "Recieved out of bounds index");

    return m_rows[idx];
  }

  /**
   * get_ith_col - Return the col component of the i-th piece placement
   */
  unsigned get_ith_col(unsigned idx) const 
  {
    my_assert(idx < get_size(), "Recieved out of bounds index");

    return m_cols[idx];
  }
  
  /**
   * get_ith_piece - Return the piece component of the i-th piece placement
   */
  const Scrabble_Piece* get_ith_piece(unsigned idx) const 
  {
    my_assert(idx < get_size(), "Recieved out of bounds index");

    return m_pieces[idx];
  }

  /**
   * operator<< - Produce a nice-looking output of this play
   */
  std::ostream& operator<<(std::ostream& out) const;

 private: // ================ PRIVATE INTERFACE ================================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  
  Indv_Play(const Indv_Play&);

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  //m_rows - The row components of the piece-placements
  Safe_Vector<unsigned>              m_rows;

  //m_cols - The col components of the piece-placements
  Safe_Vector<unsigned>              m_cols;

  //m_pieces - The piece components of the piece-placements
  Safe_Vector<const Scrabble_Piece*> m_pieces;
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////// ASSCOCIATED OPERATIONS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& out, const Indv_Play& ip);

#endif
