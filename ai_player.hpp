#ifndef ai_player_h
#define ai_player_h

#include "player.hpp"
#include "constraint.hpp"

#include <set>
#include <bitset>
#include <algorithm>
#include <tuple>
#include <string>
#include <vector>

struct UnitWrap;

/**
 * This class implements a scrabble-player that can find good (hopefully) moves
 * automatically. The most difficult code in the program is the make_play
 * method of this class.
 *
 * The algorithm for making a play is described here (at a high-level):
 * 1) Loop over every square on the board. Any square that has a piece on it
 *    is a piece around which we can potentially make moves.
 * 2) There are at most, 6 possible play-lines around a played-piece, we find
 *    out which of these is viable.
 * 3) We loop over each viable-play line, encoding the possible plays along this
 *    line that have a reasonable chance of success into Constraint objects.
 * 4) We loop over these Constraint object, seeing if any words in the dictionary
 *    that we can make satisfies the constraint. If so, we score the word; if
 *    this score is the best yet seen, we save the play.
 *
 * The algorithm for checking satisfiability of an individual constraint is also
 * fairly interesting. The need for this piece of the program to run fast is
 * essential.
 * 1) We start with the ~200,000 words in the dictionary. We need to find the
 *    ones that satisfy the constraint. The strategy is to apply cheap "filters"
 *    first to efficiently narrow down the set of potential words.
 * 2) The first filter is based on word-length. Obviously, a 10-letter word
 *    won't satisfy "_#_" and a 2-letter word won't satisfy "*#__#AS__#*"
 * 3) The second filter is based on bitmaps. We encode the letters that must be
 *    present in a word into a bitmap, then we encode the candidate word into
 *    another bitmap. If the first bitmap is not a subset of the second bitmap,
 *    the candidate will not satisfy the constraint. For example, the reg_expr
 *    "*THE*" will have a bitmap with '1' in the T,H, and E position. The word
 *    "TRUTH" will have a bitmap with '1' in the T,R,U, and H positions. "TRUTH"
 *    cannot possibly satisfy "*THE*" because it has no 'E'. The bitmap check
 *    detects this because the subset check will fail.
 * 4) The third filter is called the "possible-formulation" filter. We check to
 *    see if, given the mandatory-letters and the letters in the AI-player's
 *    tray, can the candidate word be formed. In the AI_Player constructor,
 *    one of the front-loaded computations is creating a vector of letter-counts
 *    for each dicitionary word and a vector of indeces pointing to non-zero
 *    items in the letter-count vector. We loop over the >0 letter counts and
 *    see if the AI-player has at least that many letters of that value available.
 * 5) If the candidate word has passed all these filters, we send it off to the
 *    Constraint to make the fairly expensive computation of if/how the word
 *    satisfies the constraint.
 */

////////////////////////////////////////////////////////////////////////////////
class AI_Player : public Player
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor
   */
  AI_Player(const std::string& name, Scrabble_Game* the_game) :
    Player(name, the_game),
    m_valid_words(the_game->get_valid_words()) { }

  /**
   * Destructor
   */
  virtual ~AI_Player();

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * initialize - Front-loads the bitmap and letter-count computations for
   *              every word in the dictionary.
   */
  virtual void initialize();

 protected: // ================ PROTECTED INTERFACE ============================

  /**
   * make_play - AI will examine the state of the board and find a play. This
   *             is the main method of this class. It's algorithm is described
   *             in the class description.
   */
  virtual void make_play();

  /**
   * find_all_satisfying_strings - AI goes through dictionary, finding all words
   *                               that satisfy the constraint AND that it can
   *                               make given the contents of its tray.
   *
   * constraint - The constraint we are trying to satisfy
   * min_length - The minimum length of words to search
   * max_length - The maximum length of words to search
   */
  //find all words that can be placed in a specific spot (defined by the constraint and min/max len)
  void find_all_satisfying_strings(const Constraint& constraint, unsigned min_length,
                                   unsigned max_length) const;

 private: // =================== PRIVATE INTERFACE =============================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// INTERNAL METHODS //////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  static bool is_subset(const std::vector<unsigned char>& mandatory_bits,
                        const std::bitset<26>& superset);

  static void add_to_reg_expr(std::string& reg_expr, bool negative_dir,
                              const std::string& new_component,
                              unsigned& critical_sect_expansion);

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  // m_valid_words - All valid words in the dictionary
  const std::set<std::string>& m_valid_words;

  // m_bitsets - All valid words with their letter-containment bitset and length
  std::vector<std::tuple<std::bitset<26>, uint8_t, const std::string*> > m_bitsets;

  // m_word_reqs - Letter requirements for all valid words
  std::vector<std::vector<unsigned char> > m_word_reqs;

  // m_word_req_indeces - vectors of relevant (1 >=) word-req indices
  std::vector<std::vector<unsigned char> > m_word_req_indeces;

  // m_recent_result - contains the most recent result for
  //                   find_all_satisfying_strings. Each item contains a string
  mutable std::vector<const std::string*> m_recent_result;

  // m_result_placements - we also have a vector of all-possible-placement-vectors
  //                       (relative to the first piece of the mandatory section).
  mutable std::vector<std::vector<unsigned> > m_result_placements;

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// FRIENDS //////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  friend struct UnitWrap;
};

#endif
