#ifndef scrabble_board_h
#define scrabble_board_h

#include "scrabble_square.hpp"
#include "board_builder.hpp"
#include "scrabble_word.hpp"
#include "indv_play.hpp"
#include "scrabble_piece.hpp"
#include "board_loc.hpp"

#include <iostream>
#include <algorithm>
#include <vector>

/*
 * This class represents the game board, a two dimension array of squares
 * indexed by [row][col]. See the Scrabble_Square class for more details on
 * the capabilities of an individual square.
 *
 * This class' primary responsibility is maintaining the state of the board
 * and being able to respond to a wide range of queries related to the state
 * of the board.
 *
 * An important concept for this class and other classes in the program is the
 * notion of a "play-line" or just "line". A line is a linear region of the
 * board where a play (or potential play) is located.
 *
 * Also important is the notion of "primary" vs "secondary" words. The primary
 * word is the word formed along your play-line. Secondary words are the words
 * formed incidentally when the pieces along the primary line are played.
 */

////////////////////////////////////////////////////////////////////////////////
class Scrabble_Board
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - This is primarily a handoff to a builder object. The
   *               building of a board is pretty independent of the rest of
   *               the class, making this a good candidate for the builder pattern.
   */
  Scrabble_Board(const Board_Builder& board_builder_instance)
  {
    board_builder_instance.build_board(this);
  }

  /**
   * Destructor
   */
  ~Scrabble_Board() {}

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * place_piece - Puts a new piece on the board. This is the outside world's
   *               only way to change the state of the board.
   *
   * row       - row location of the new piece
   * col       - col location of the new piece
   * the_piece - the new piece
   */
  void place_piece(unsigned row, unsigned col, const Scrabble_Piece* the_piece);

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// QUERIES /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * is_free - Returns true if square does not have a piece on it
   */
  bool is_free(unsigned row, unsigned col) const;

  /**
   * is_adjacent - Returns true if square is adjacent to a square with a piece on it
   */
  bool is_adjacent(unsigned row, unsigned col) const
  {
    return get_adjacent(row, col, true).size() > 0;
  }

  /**
   * is_unconstrained - Returns true is a square is unconstrained
   *                    (unconstrained -> no piece on it && not adjacent to any piece)
   */
  bool is_unconstrained(unsigned row, unsigned col) const;

  /**
   * get_created_words - Returns all the word objects hypothetically created by
   *                     the_play. This method is of significant complexity.
   */
  std::vector<Scrabble_Word> get_created_words(const Indv_Play& the_play) const;

  /**
   * get_piece - Returns the piece that was played at a certain location.
   */
  const Scrabble_Piece* get_piece(unsigned row, unsigned col) const;

  /**
   * get_square - Returns the square on the board at a certain location
   */
  Scrabble_Square get_square(unsigned row, unsigned col) const;

  /**
   * get_board_dim - Returns the size (aka dimension) of the board
   */
  unsigned get_board_dim() const { return m_board.size(); }

  /**
   * add_all_pieces_in_direction - With respect to my_row,my_col , add all pieces
   *                               in the occupied squares going in the direction
   *                               of adj_row, adj_col to word. In other words, this
   *                               adds a 'sequence' of letters looking in a certain direction.
   *
   * my_row  - The row of the perspective square
   * my_col  - The col of the perspective square
   * adj_row - The row where the search is beginning
   * adj_col - The col where the search is beginning
   * word    - The word we will be adding to
   * col_id  - Specifies that we should use a square's col as its piece's id in the word
   */
  void add_all_pieces_in_direction(unsigned my_row,  unsigned my_col,
                                   unsigned adj_row, unsigned adj_col,
                                   Scrabble_Word& word) const;

  /**
   * get_adjacent -  Returns a std::vector of coordinates for adjacent squares
   *                 with pieces (or no piece) on them.
   *
   * row              - The row of the square whose neighbors we are examining
   * col              - The col of the square whose neighbors we are examining
   * one_per_dim      - Specifies that we should only return at most one vertical
   *                    neighbor and one horizontal neighbor
   * looking_for_free - Specifies that we are actually looking for free neighboring
   *                    squares, not occupied ones.
   */
  std::vector<Board_Loc> get_adjacent(unsigned row, unsigned col,
                                      bool one_per_dim = false,
                                      bool looking_for_free = false) const;

  /**
   * operator<< - produces a nice-looking output that should convey the state
   *              of the board.
   */
  std::ostream& operator<<(std::ostream& out) const;

 private: // ================ PRIVATE INTERFACE ================================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Scrabble_Board(const Scrabble_Board&);
  Scrabble_Board& operator=(const Scrabble_Board&);

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  // m_board - A two dimensional vector of squares, index by [row][col]
  std::vector<std::vector<Scrabble_Square> > m_board;

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// FRIENDS //////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  friend class Standard_Board_Builder;
  friend class Wwf_Board_Builder;
  friend class Wwf_Solo_Board_Builder;
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////// ASSCOCIATED OPERATIONS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& out, const Scrabble_Board& sb);

#endif
