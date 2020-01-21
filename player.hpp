#ifndef player_h
#define player_h

#include "scrabble_game.hpp"
#include "indv_play.hpp"
#include "scrabble_piece.hpp"

#include <set>
#include <map>
#include <iostream>
#include <string>
#include <vector>

struct UnitWrap;

/**
 * This class represents an individual player. Responsibilities of this class
 * are maintaining your standard player state (name, score, pieces etc.). This
 * class is abstract: how the player actually makes an individual play will
 * vary greatly depending on the subclass. Basically all other functionality
 * of a player is handled in this base class.
 *
 * Worth an extra note is this class' duty of maintaining a character->piece
 * multimap. This map is essential in ease-of-use for both human and AI players.
 * It allows players to make general requests for pieces via the desired
 * character. This is nice because we usually don't care exactly which piece
 * object we get. Also, this helps manage the state of any wildcard pieces.
 *
 * You can think of the act of playing a piece as a two-phase process. First,
 * the get_piece method is called when the player thinks about making a play
 * with a certain letter. Second, when the player submits the play, the pieces
 * are removed from the vector, which makes the transaction permanent. Simply
 * removing a peice from the map is not permanent and can be undone by calling
 * remap.
 */

////////////////////////////////////////////////////////////////////////////////
class Player
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - Initializes basic state. Starts player with an empty 'tray'
   *               of pieces.
   *
   * name     - The name of this player
   * the_game - The game this player is taking part in
   */
  Player(const std::string& name, Scrabble_Game* the_game);

  /**
   * Destructor
   */
  virtual ~Player() {}

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * initialize - Prepares a player's state as the game begins
   */
  virtual void initialize() {}


  /**
   * game_over - A notification to this player that the game has ended. This will
   *             cause the player to deduct points based on what remains in their
   *             tray.
   */
  void game_over();

  /**
   * play - The outside world's way of telling us to make a play. This method is
   *        actually a template-alg pattern that performs some state clean-up
   *        before calling the abs-virtual method that actually makes the play
   *        'make_play'.
   */
  const Indv_Play& play();

  /**
   * add_piece - Adds a piece to the player's tray.
   */
  void add_piece(const Scrabble_Piece* new_piece);

  /**
   * remove_piece - Removes a piece from the player's tray
   */
  void remove_piece(const Scrabble_Piece* piece);

  /**
   * add_score - Gives this player some additional points
   */
  void add_score(unsigned score) { m_score += score; }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// QUERIES /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * get_name - Return name of player
   */
  const std::string& get_name() const { return m_name; }

  /**
   * get_score - Return player's score
   */
  unsigned get_score() const { return m_score; }

  /**
   * operator<< - Produces a nice-looking output of the player's state
   */
  std::ostream& operator<<(std::ostream&) const;

  /**
   * get_num_pieces - Returns the number of pieces in the player's tray
   */
  unsigned get_num_pieces() const;

 protected: // ================ PROTECTED INTERFACE ============================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Player(const Player&);
  Player& operator=(const Player&);

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// INTERNAL METHODS //////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * make_play - The method that actually computes what this player's next move
   *             will be. This method single-handed defines what type of player
   *             you are.
   */
  virtual void make_play() = 0;

  /**
   * get_piece - Returns a piece that has letter matching the argument. This
   *             removes the piece from the map (but not the tray) since the
   *             piece cannot be used again in the current play. This method
   *             can also potentially change the state of wildcards.
   */
  const Scrabble_Piece* get_piece(char c) const;

  /**
   * remap - Recreates the map, essentially undoing any get_piece calls that
   *         were made.
   */
  void remap() const;

  /**
   * has_piece - Returns true if player has 'exact' piece provided by argument.
   *             (Addresses must match).
   */
  bool has_piece(const Scrabble_Piece* piece) const;

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  //m_name - The player's name
  std::string                                        m_name;

  //m_score - The player's current score
  unsigned                                           m_score;

  //m_the_game - A pointer to the game the player is taking part in
  Scrabble_Game*                                     m_the_game;

  //m_current_play - A handle to the play-object being created by make_play
  Indv_Play                                          m_current_play;

  //m_pieces - The player's tray of pieces
  std::vector<const Scrabble_Piece*>                 m_pieces;

  //m_char_piece_map - Maps characters to piece objects. This map represent what
  //                   letters are available to be played. Note: this map will
  //                   often have things removed from it as a player puts together
  //                   a play, but this has no affect on m_pieces.
  mutable std::multimap<char, const Scrabble_Piece*> m_char_piece_map;

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// FRIENDS //////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  friend struct UnitWrap;
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////// ASSCOCIATED OPERATIONS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& out, const Player& player);

#endif
