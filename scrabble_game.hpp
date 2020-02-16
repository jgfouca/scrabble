#ifndef scrabble_game_h
#define scrabble_game_h

#include "scrabble_board.hpp"
#include "scrabble_config.hpp"
#include "piece_source.hpp"
#include "indv_play.hpp"
#include "scrabble_exception.hpp"

#include <iostream>
#include <set>
#include <string>
#include <vector>

class Player;
struct UnitWrap;

/**
 * The Scrabble_Game class is the mediator class of the program. Once the play
 * method has been called, this class is the "root" of all the action. This
 * class' primary duties are letting players know when it is their turn to play,
 * processing the plays players give it, enforcing the rules of scrabble, and
 * being able to answer certain questions regarding rules or state of the game.
 *
 * Current enforced rule-set:
 * 1) If first play, one piece must cover the 7,7 square
 * 2) All played letters must be along the same line
 * 3) All played letters must be connected to each other directly or via sequences of pieces already on the board
 * 4) All played letters must lie within the game board
 * 5) Player may not make two placements on the same square
 * 6) No piece may be placed on top of another piece
 * 7) If not first play, one of the pieces must be adjacent to a piece already on the board
 * 8) Every adjacent word must continue to be a valid word
 */

////////////////////////////////////////////////////////////////////////////////
class Scrabble_Game
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - sets up a "null" game state that is not playable.
   */
  Scrabble_Game(const Scrabble_Config& config) :
    m_config(config),
    m_game_board(NULL),
    m_piece_source(NULL),
    m_first_play(true),
    m_game_over(false)
  {}

  /**
   * Destructor - cleans up memory
   */
  ~Scrabble_Game();

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * play - The loop that drives the progress of the game.
   */
  void play();

  /**
   * save - Same the game state as a file that can be loaded later
   */
  void save(const std::string& filename) const;

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// QUERIES /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * get_potential_score - Evaluates the_play and returns what the_play would
   *                       score. This method is intended to be called by the
   *                       AIs, so only valid plays are expected.
   */
  unsigned get_potential_score(const Indv_Play& the_play) const;

  /**
   * is_complete - returns true if the game has been built and is now playable
   */
  bool is_complete() const
  {
    return (m_game_board && m_piece_source && !m_players.empty());
  }

  /**
   * is_first_play - returns true if no play has been made yet on this game
   */
  bool is_first_play() const { return m_first_play; }

  /**
   * get_board - returns a ptr to the board being used in the game
   *             (players/AI's need the ability to examine the board)
   */
  const Scrabble_Board* get_board() const { return m_game_board; }

  /**
   * get_valid_words - returns a reference to the dictionary.
   *                   (AIs need to know what words are valid)
   */
  const std::set<std::string>& get_valid_words() const { return m_valid_words; }

  /**
   * get_config - returns a references to the game configuration
   */
  const Scrabble_Config& get_config() const { return m_config; }

  /**
   * operator<< - produces a nice-looking output that should convey the state
   *              of the game.
   */
  std::ostream& operator<<(std::ostream& out) const;

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// CONSTRUCTION METHODS //////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * add_board - called by the builders, this method sets what board object will
   *             be used during play.
   */
  void add_board(Scrabble_Board* board)
  {
    my_assert(!m_game_board,
              "add_board was called twice on the same scrabble_game obj");
    m_game_board = board;
  }

  /**
   * add_player - called by the builders, this method adds a player to the game
   */
  void add_player(Player* player) { m_players.push_back(player); }

  /**
   * add_piece_source - called by the builders, this method sets what letter
   *                    source will be used during play.
   */
  void add_piece_source(Piece_Source* ls)
  {
    my_assert(!m_piece_source,
              "add_piece_source was called twice on the same scrabble_game obj");
    m_piece_source = ls;
  }

 private: // ================ PRIVATE INTERFACE ================================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Scrabble_Game(const Scrabble_Game&) = delete;
  Scrabble_Game& operator=(const Scrabble_Game&) = delete;

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// INTERNAL METHODS //////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * evaluate_play - Checks the the_play does not violate any rules. If the play
   *                 does violate a rule, we return a string describing why the
   *                 play was invalid. Returning an empty string implies that
   *                 the play was valid.
   */
  std::string evaluate_play(const Indv_Play& the_play) const;

  /**
   * process_legit_play - Takes the_play and applies it to the game, changing
   *                      many peices of game-state and player-state.
   *
   * the_play - The valid play that is being made
   * player   - The player who is making the play
   */
  void process_legit_play(const Indv_Play& the_play, Player* player);

  /**
   * initialize - Called after the game has been fully built... sets up the
   *              dictionary and gives all players their initial pieces.
   */
  void initialize();

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  // m_config - The configuration for this game
  Scrabble_Config          m_config;

  // m_players - A vector of all players in the game
  std::vector<Player*>     m_players;

  // m_game_board - The board object being used by the game
  Scrabble_Board*          m_game_board;

  // m_piece_source - Where we can ask for additional pieces
  Piece_Source*            m_piece_source;

  // m_first_play - Is the board in a "virgin" state (no plays on it)
  bool                     m_first_play;

  // m_valid_word - A set of all the words in the dictionary
  std::set<std::string>    m_valid_words;

  // m_msg_log - A vector of strings that describe recent happenings in the
  //             game (usually recent plays).
  std::vector<std::string> m_msg_log;

  // m_potential_score - The score that would be made if the most recently
  //                     evaluated play were made.
  mutable unsigned         m_potential_score;

  // m_potential_words - The words that would be created if the most recently
  //                     evaluated play were made.
  mutable std::string      m_potential_words;

  // m_game_over - True if this game has been completed.
  bool                     m_game_over;

  // buffers for talking to python
  unsigned m_row_buff[64];
  unsigned m_col_buff[64];
  char     m_let_buff[128];

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// FRIENDS //////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  friend struct UnitWrap;
};

////////////////////////////////////////////////////////////////////////////////
///////////////////////// ASSCOCIATED OPERATIONS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& out, const Scrabble_Game& sg);

#endif
