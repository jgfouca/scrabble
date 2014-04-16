#ifndef scrabble_word_h
#define scrabble_word_h

#include "scrabble_square.h"
#include "scrabble_piece.h"
#include "safe_string.h"

#include <iostream>
#include <algorithm>

/**
 * This class represents a word that has been (or will be) formed on the board.
 * Scrabble_Words are usually associated with a piece-sequence on the board. 
 * This class knows how to add to itself based on incoming pieces. It knows how
 * to score itself and transform itself into a standard string. 
 *
 * An important concept for this class is the "order" of a piece within a 
 * Scrabble_Word. Recall that, in scrabble, words always read from top to 
 * bottom for vertical words and left to right for horizontal words. So, for
 * vertical words, we use a piece's row-position as it's order; for horizontal
 * words, we use it's column-position. Therefore, if we sort pieces of a word
 * from lowest to highest order, the string produced will be the real word that 
 * is created.
 */

////////////////////////////////////////////////////////////////////////////////
class Scrabble_Word
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor 
   */
  Scrabble_Word() {}

  /**
   * Destructor 
   */
  ~Scrabble_Word() {}

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * add_component - Adds to this word. Note that components can be added in any 
   *                 order, just make sure the order arg is correct.
   *
   * piece - The piece that's being added as a part of this word
   * bonus - The bonus associated with this part of the word
   * order - The order of the piece (see class description if this doesnt make sense)
   */
  void add_component(const Scrabble_Piece* piece, Bonus bonus, unsigned order);

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// QUERIES /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * score - Returns the score that would be earned if this word were created.
   */
  unsigned score() const;

  /**
   * get_word_str - Returns the word in string form.
   */
  Safe_String get_word_str() const;

  /**
   * has_piece - Returns true if the word has a piece with a certain order
   */
  bool has_piece(unsigned order) const { return m_word.find(order) != m_word.end(); }
  
  /**
   * operator<< - Produces output for this Scrabble_Word
   */
  std::ostream& operator<<(std::ostream& out) const;

 private: // ================== PRIVATE INTERFACE ==============================

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  // m_word - Maps order to the respective piece and associated bonus
  std::map<unsigned, std::pair<const Scrabble_Piece*, Bonus> > m_word;  
};

std::ostream& operator<<(std::ostream& out, const Scrabble_Word& sw);

#endif
