#ifndef scrabble_square_h
#define scrabble_square_h

#include "scrabble_piece.hpp"
#include "standard_board_builder.hpp"
#include "wwf_board_builder.hpp"

#include <iostream>
#include <string>

//The Bonus enum contains values for all possible bonuses on the board
enum Bonus {NONE, DBL_LET, TRP_LET, DBL_WRD, TRP_WRD};

/**
 * This class represents a single square on the scrabble board. In other words
 * a Scrabble_Board is made up of instances of Scrabble_Square. Scrabble_Squares
 * are responsible for knowing if a piece has been placed on them, if so, what
 * piece object has been placed on them, and what bonuses apply to this square.
 */

////////////////////////////////////////////////////////////////////////////////
class Scrabble_Square
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - Initializes this square with no piece and no bonus.
   */
  Scrabble_Square() : m_piece(NULL), m_bonus(NONE) {}

  /**
   * Destructor
   */
  ~Scrabble_Square() {}

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * add_piece - Adds a piece to this square. This is what ultimately happens
   *             when a player plays a piece.
   */
  void add_piece(const Scrabble_Piece* piece);

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// QUERIES /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * is_free - Returns true if no one has played a piece on this square.
   */
  bool is_free() const { return (!m_piece); }


  /**
   * get_bonus() - Returns the bonus associated with this square
   */
  Bonus get_bonus() const { return m_bonus; }

  /**
   * get_piece() - Returns the piece played on this square. We return NULL if
   *               this square is free.
   */
  const Scrabble_Piece* get_piece() const { return m_piece; }

  /**
   * operator<< - Produces a nice-looking output of the square's state. This is
   *              intended to be called by Scrabble_Board as it is producing an
   *              output for the entire board.
   */
  std::ostream& operator<<(std::ostream& out) const;

  // OUTPUT_LEN - The number of characters of output created when operator<<
  //              is called on a scrabble_square. This is difficult to enforce,
  //              but it's useful
  static const unsigned OUTPUT_LEN = 6;

 private: // ================== PRIVATE INTERFACE ==============================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// CONSTRUCTION METHODS //////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * set_bonus - Changes the bonus associated with this square to that of the
   *             argument. We don't want this method to be called by anyone,
   *             so we made it private and befriended Standard_Board_Builder.
   */
  void set_bonus(Bonus bonus);

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  // m_piece - A pointer to whatever piece has been played on this square. This
  //           is NULL if no piece has been played.
  const Scrabble_Piece* m_piece;

  // m_bonus - The bonus associated with this square.
  Bonus                 m_bonus;

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// FRIENDS //////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  friend class Standard_Board_Builder;
  friend class Wwf_Board_Builder;
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////// ASSCOCIATED OPERATIONS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& out, const Scrabble_Square& sq);

std::ostream& operator<<(std::ostream& out, const Bonus& b);

#endif
