#ifndef scrabble_piece_h
#define scrabble_piece_h

#include "scrabble_point_map.hpp"
#include "scrabble_common.hpp"
#include "scrabble_exception.hpp"

#include <iostream>
#include <string>

class Human_Player;
class Scrabble_Game;
struct UnitWrap;

/**
 * This class represents a single piece that can be played on the board. Pieces
 * have a letter and a point-value associated them. Pieces can only be played
 * once.
 */
////////////////////////////////////////////////////////////////////////////////
class Scrabble_Piece
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - Initializes state.
   */
  Scrabble_Piece(const Scrabble_Game& parent, char letter, const Point_Map& point_map) :
    m_parent(parent), m_point_map(point_map)
  {
    my_assert(is_valid_letter(letter) || letter == '-',
              std::string("'") + obj_to_str(letter) + "' is not a valid letter" );

    m_letter          = letter;
    //we use the point map to figure out how many point we are worth
    m_point_val       = m_point_map.get_point_val(m_letter);
    m_played          = false;
    m_been_output     = false;
    m_wildcard_choice = letter;
  }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * played - Tells this piece object that it has been played. We use this method
   *          to ensure that a piece is never played twice.
   */
  void played() const
  {
    my_assert(!m_played,
              std::string("Attempted to play piece '") + m_letter +  "' twice");

    m_played = true;
  }

  /**
   * set_wildcard_value - For wildcard pieces, this method changes the value it
   *                      is assuming.
   */
  void set_wildcard_value(char c) const
  {
    my_assert(is_valid_letter(c),
              std::string("Attempted to set wild-card to invalid char '") + m_letter + "'");
    my_assert(is_wildcard(), "Attempted to set wild-card on non-wild-card piece");

    m_wildcard_choice = c;
  }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// QUERIES /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * get_letter - Returns the letter of this piece (reflect wildcard changes).
   */
  char get_letter() const { return m_wildcard_choice; }

  /**
   * get_encoded_letter - Returns the letter of this piece, using capitalization to
   *                      encode whether it's a wildcard piece or not.
   */
  char get_encoded_letter() const;

  /**
   * get_point_val - Returns the point value of this piece
   */
  unsigned get_point_val() const { return m_point_val; }

  /**
   * is_played - Returns if this piece has been played or not
   */
  bool is_played() const { return m_played; }

  /**
   * operator<< - Outputs the piece (letter value only).
   */
  std::ostream& operator<<(std::ostream& out) const;

  /**
   * is_wildcard - Returns true is this piece is a wild-card piece
   */
  bool is_wildcard() const { return m_letter == '-'; }

  /**
   * is_valid_letter - A static method that returns true if the argument
   *                   is a valid normal letter.
   */
  static bool is_valid_letter(char letter) { return ( (letter >= 'A') && (letter <= 'Z') ); }

 private: // ================ PRIVATE INTERFACE ================================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Scrabble_Piece(const Scrabble_Piece&) = delete;
  Scrabble_Piece& operator=(const Scrabble_Piece&) = delete;

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// INTERNAL METHODS //////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * force_letter_change - Artificially forces a piece to change value. This
   *                       should only be called by Scrabble_Tester or
   *                       Human_Player.
   */
  void force_letter_change(char letter)
  {
    my_assert(is_valid_letter(letter) || letter == '-',
              std::string("'") + obj_to_str(letter) + "' is not a valid letter" );
    my_assert(!m_played, "Should not call force_letter_change on played piece");
    my_assert(!m_been_output, "Should not have been output already");

    m_letter          = letter;
    m_point_val       = m_point_map.get_point_val(m_letter);
    m_wildcard_choice = letter;
  }

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  // m_parent - Parent game
  const Scrabble_Game& m_parent;

  // m_letter - The letter of this piece
  char         m_letter;

  // m_wildcard_choice - The letter the wildcard piece is assuming
  mutable char m_wildcard_choice;

  // m_point_val - The point-value of this piece
  unsigned     m_point_val;

  // m_been_output - Specifies if this piece has been output before. Pieces that
  //                 are output for the first time appear red.
  mutable bool m_been_output;

  // m_played - Specifies if this piece is on the board.
  mutable bool m_played;

  // m_point_map - Reference to point map to use
  const Point_Map& m_point_map;

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// FRIENDS //////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  friend class Human_Player;
  friend struct UnitWrap;
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////// ASSCOCIATED OPERATIONS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& out, const Scrabble_Piece& sp);

#endif
