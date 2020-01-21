#ifndef constraint_h
#define constraint_h

#include <set>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

class Player;

/**
 * This class represents a single search through the dictionary. The
 * search is constrained by string that is similar to a regular expression.
 * Unlike regular expressions, these strings come with a set of external
 * state. This search language is defined here:
 *
 *************** SEARCH LANGUAGE DEFINITION ************************
 *
 * Constraints are primary defined by their reg-expr string. We use our own special reg-expr
 * language here. The allowed characters are:
 * '*'     -> Matches any character 0 or more times.
 * '[A-Z]' -> Matches the exact character specified
 * '#'     -> Matches any character 1 time that is compatible with a secondary word
 * '_'     -> Matches any character 1 time.
 *
 * Rules:
 * -> '*' may only occur at the beginning and/or end of the reg-expr
 * -> Each '*' must have a corresponding item in the max_lengths vector specifying the maximum number
 *    of characters the '*' can match
 * -> Each '#' must have a corresponding item in the req_sets vector. This item specifies
 *    the word and the index where the new character will go.
 * -> A reg-expr cannot contain any character outside the set of allowed characters.
 *
 * Constraints are intended to represent the requirements a word needs to pass in order to
 * be placeable in a specific location.
 *
 ********************** KEY CONCEPTS ********************************
 *
 * "Compatibility" - in scrabble, not only do the letters along your primary play-line
 * have to be a valid word, but any secondary words that get formed by the play
 * must be words as well. We express this concept as compatibility and we use
 * the '#' character to represent this in a reg expr. The '#' comes with extra
 * bagage: it will have set<char> in the requirement-sets vector.
 * For example, say we have the reg_expr "*#_A*" and the compatibility pair:
 * (3, "CA#"). This means that, for whatever word we chose to satisfy "*#_A*, the
 * character of the word that happens to fall in the # location must also be
 * compatible with "CA#" when it is ALSO inserted in the # location of that word.
 * We optimize this process by pre-computing all possible valid letters for
 * "CA#" and storing them in a set. This will avoid expensive dictionary searches
 * in the critical path of the program.
 *
 * We consider each part of the reg-expr that is associated with a
 * "highly-constrained square" to be a "difficult component" of the reg expr.
 * Highly-constrained squares are places on the board that either have a piece
 * played on them, or are immediately adjacent to a square with a piece on it.
 * In the reg_expr, the former will be [A-Z] and the latter '#'. The other
 * pieces of a reg_expr ('*', '_') we call "easy components" and are correlated
 * with "unconstrained squares".
 *
 * Each constraint has a "mandatory section". This is the part of the reg-expr which
 * MUST be matched in order for a word to meet the constraint. For example, for
 * reg-expr "*E*", the mandatory section is simply "E". For "*_#E_*" the mandatory
 * section is "_#E_". Simplifying things, the mandatory section is the reg-expr
 * with the '*' components removed.
 *
 * Each constraint has a "critical square". This is the square around which the
 * prospective move is centered. No matter how much the constraint is "eased"
 * (see below), it will always include the critical square. See the AI-player
 * documentation for more on critical squares.
 *
 * We need to have a way of making a constraint easier to satisfy in the case that
 * we find no matching words for the original constraint. We use a tree-based
 * algorithm when R, L children are formed by removing the rightmost or leftmost
 * difficult component. We can iterate over the easings by having a simple
 * way of converting a number into a set of R,L moves.
 *
 *********************** OTHER INFO *********************************
 *
 * In order to give some help to whomever has to place this word, if a
 * word satisfies the constraint, we will also determine the locations
 * of all valid placements of this word. In order to convey this
 * information, we need a point of reference. We will use the first
 * item in the mandatory section of the search std::string as this
 * point of reference. So, if the search std::string is "*THE*" and
 * the word is "LOATHE", then we will add -3 to the
 * potential_placements std::vector.
 */

////////////////////////////////////////////////////////////////////////////////
class Constraint
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * Constructor - creates the constraint. Checks that the arguments provided
   *               follow the rules explained above. Does as much front-loaded
   *               computation as possible in order to speed up the critical
   *               "satisfies" method and to support early-filtering in the
   *               AIs.
   *
   * reg_expr    - A regular expression (from our customized reg_expr language)
   * req_sets    - Contains the necessary info on any compatibilites in the reg_expr
   * max_lengths - Contains info on the '*' items in the reg_expr
   * valid_words - The set of all valid words in the dictionary.
   */
  Constraint(const std::string& reg_expr,
             const std::vector<std::set<char> >& req_sets,
             const std::vector<unsigned>& max_lengths);

  /**
   * Destructor
   */
  ~Constraint() {}

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PRIMARY INTERFACE ///////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * ease - Perform an easing of this constraint. It is intended that this method
   *        always be called on the ORIGINAL constraint object. We keep track
   *        of the number of easings internally, so we know what this particular
   *        easing should do. This gives us a consisted "root" of the easing
   *        tree to start from. As a side note, the object returned is on the
   *        heap and is the the callers responsibility to delete.
   */
  Constraint* ease(unsigned& mand_sect_offset);

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// QUERIES /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * satisfies - Returns true if a word satisfies this constraint. The ability
   *             to respond to this query is this class' primary duty. It checks
   *             that all the "difficult components" of the reg-expr are
   *             satisfied. IE the word has the mandatory letters in the correct
   *             spots and meets all the compatibility requirements.
   *
   * word                 - We are determining if the string pointed to by this
   *                        arg satisfies the constraint
   * potential_placements - In some cases, when dealing with a BOTH_FREE constraint,
   *                        it is possible that several different placements of a
   *                        word will be satisfactory. We want to keep track of all
   *                        of these so that the one with the highest pt return is
   *                        discovered. We use this vector to store that info.
   *                        See OTHER INFO in class description for more.
   */
  bool satisfies(const std::string* word, std::vector<unsigned>& potential_placements) const;

  /**
   * get_mandatory_bitset - Examines the mandatory section, looking at difficult
   *                        components with [A-Z] values. The letters found this way
   *                        can be encoded in a bitset. Any word which does not
   *                        have those letters can be ignored. This will be used
   *                        for early-filtering by the AIs.
   */
  const std::bitset<26>& get_mandatory_bitset() const { return m_mandatory_bitset; }

  /**
   * get_mandatory_letters - Examines the mandatory section, looking at difficult
   *                         components with [A-Z] values. These letters must be
   *                         used by the AI to construct any potential satisfying
   *                         word.
   */
  const std::vector<char>& get_mandatory_letters() const { return m_mandatory_letters; }

  /**
   * operator<< - Produces a nice-looking output of the reg_expr along with
   *              the supplementary state.
   */
  std::ostream& operator<<(std::ostream& out) const;

  /**
   * operator== - Equality of two constraints is determined solely by the reg_expr
   *              strings. This isn't totally safe, but it's intended to be used
   *              to compare Constraints associated with the same play-line. In that
   *              case, I believe this implementation is adequate.
   */
  bool operator==(const Constraint& rhs) const;

  /**
   * min_word_length - Return the length of the smallest possible word that
   *                   could still satisfy this constraint.
   */
  unsigned min_word_length() const;

  /**
   * max_word_length - Return the length of the biggest possible word the
   *                   player could create that satisfies this constraint. This
   *                   takes into account the fact that players have at most
   *                   all their pieces to play.
   */
  unsigned max_word_length(const Player& player) const;

  /**
   * mand_sect_size - Returns the size/length of the mandatory section.
   */
  unsigned mand_sect_size() const { return m_mandatory_sect_size; }

  /**
   * can_be_eased - Returns true if this constraint can be further eased. If
   *                the only remaining easing would remove the critical square,
   *                then we cannot ease anymore.
   */
  bool can_be_eased() const;

  /**
   * is_mandatory_sect_critical_span - Returns true if the entire mandatory
   *                                   section is the "critical span", the
   *                                   span of [A-Z] components connected to
   *                                   an [A-Z] critical square.
   */
  bool is_mandatory_sect_critical_span() const;

  /**
   * Placement_Type is a useful enum for categorizing the type of constraint
   * we are dealing with.
   *
   * TOTAL_FREEDOM = "*" (only occurs first play of the game)
   * LEFT_FREE     = "*[A-Z_#]+"
   * RIGHT_FREE    = "[A-Z_#]+*"
   * BOTH_FREE     = "*[A-Z_#]+*"
   * NO_FREE       = "[A-Z_#]+"
   */
  enum Placement_Type {INVALID, TOTAL_FREEDOM, LEFT_FREE, RIGHT_FREE, BOTH_FREE, NO_FREE};

  /**
   * covert_compat_req_to_set - Converts compatibility requirements of the form
   *                            (string, index) to a set of characters.
   *
   * valid_word          - The set of all valid words
   * req_compatibilities - The compatibilities of the form (string, index)
   * compatibility_sets  - We will fill this vector with the computed sets
   */
  static void convert_compat_req_to_set(const std::set<std::string>& valid_words,
                                        const std::vector<std::pair<std::string, unsigned> >& req_compatibilities,
                                        std::vector<std::set<char> >& compatibility_sets);

 private: // ================ PRIVATE INTERFACE ================================

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Constraint(const Constraint&);
  Constraint& operator=(const Constraint&);

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// INTERNAL METHODS //////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * count_occurences - Returns how many times the character specified by the
   *                    arguments occurs in the reg_expr string.
   */
  unsigned count_occurences(char c) const;

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// DATA MEMBERS ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  // m_reg_expr - The regular expression in our custom regex language
  std::string m_reg_expr;

  // m_mandatory_bitset - see get_mandatory_bitset
  std::bitset<26> m_mandatory_bitset;

  // m_mandatory_letters - see get_mandatory_letters
  std::vector<char> m_mandatory_letters;

  // m_req_sets - compatibility info for '#' components
  std::vector<std::set<char> > m_req_sets;

  // m_max_lengths - max number of matching for '*' components
  std::vector<unsigned> m_max_lengths;

  // m_placement_type - see Placement_Type enum. This member contains the
  //                    appropriate placement-type of this constraint.
  Placement_Type m_placement_type;

  // m_num_easings - The number of iterations of easing to go through
  //                 when "ease" is called.
  unsigned m_num_easings;

  // m_mandatory_sect_begin - The index within m_reg_expr where the
  //                          mandatory section begins.
  unsigned m_mandatory_sect_begin;

  // m_mandatory_sect_end - The index within m_reg_expr where the
  //                        mandatory section ends (non-inclusive).
  unsigned m_mandatory_sect_end;

  // m_mandatory_sect_size - The size of the mandatory section
  unsigned m_mandatory_sect_size;

  // m_mandatory_letter_indeces - The indeces, from the point of view of the
  //                              start of the mandatory section, at which the
  //                              mandatory letters lie.
  std::vector<unsigned> m_mandatory_letter_indeces;

  // m_compat_indeces - The indeces, from the point of view of the start of the
  //                    mandatory section, at which the '#'s lie.
  std::vector<unsigned> m_compat_indeces;

};

std::ostream& operator<<(std::ostream& out, const Constraint& c);

#endif
