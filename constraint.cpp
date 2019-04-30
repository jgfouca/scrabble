#include "constraint.hpp"
#include "scrabble_piece.hpp"
#include "scrabble_exception.hpp"

#include <stack>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
Constraint::Constraint(const std::string& reg_expr,
                       const std::vector<set<char> >& req_sets,
                       const std::vector<unsigned>& max_lengths)
////////////////////////////////////////////////////////////////////////////////
  : m_reg_expr(reg_expr),
    m_req_sets(req_sets),
    m_max_lengths(max_lengths),
    m_placement_type(INVALID),
    m_num_easings(0)
{
  //Lots of asserts to check that the rules are being followed
  my_assert(count_occurences('*') == max_lengths.size(),
            "Number of '*' characters did not match number of entries in max_lengths.");
  my_assert(count_occurences('*') <= 2,
            "Number of '*' characters should never be more than two.");
  my_assert(count_occurences('#') == req_sets.size(),
            "Number of '#' characters did not match number of entries in req_sets.");
#ifndef NDEBUG
  for (unsigned i = 0; i < reg_expr.size(); i++) {
    //'*' should only be at the first or last character of the string
    if (reg_expr[i] == '*') {
      my_assert(i == 0 || i == (reg_expr.size() - 1),
                "Found '*' character not at beginning or end of reg_expr");
    }
    //assert that character is recognized
    my_assert(reg_expr[i] == '*' || reg_expr[i] == '#' || reg_expr[i] == '_' ||
              Scrabble_Piece::is_valid_letter(reg_expr[i]),
              std::string("Character at index") + obj_to_str(i) + " not recognized");
  }
#endif 

  //front-load as much work as possible to keep it out of the satifies method. We anticipate
  //with near-certainty that the satisfies method will be the "critical path" of this program
  if (reg_expr.size() == 1 && reg_expr[0] == '*') {
    m_placement_type = TOTAL_FREEDOM;
    m_mandatory_sect_size  = 0;
    m_mandatory_sect_begin = 0;
    m_mandatory_sect_end   = 0;
  }
  else {
    //the word is not totally open, it has a mandatory section
    //figure out the bounds of the mandatory section
    m_mandatory_sect_begin = 0 + (reg_expr[0] == '*');
    m_mandatory_sect_end   = reg_expr.size() - (reg_expr[reg_expr.size()-1] == '*'); //non-inclusive

    //calculate mandatory-bitset, mandatory-letters, mandatory-letter-indeces,
    //and compat-indeces
    for (unsigned i = m_mandatory_sect_begin; i < m_mandatory_sect_end; i++) {
      if (Scrabble_Piece::is_valid_letter(reg_expr[i])) {
        m_mandatory_bitset.set(reg_expr[i] - 'A');
        m_mandatory_letters.push_back(reg_expr[i]);
        m_mandatory_letter_indeces.push_back(i - m_mandatory_sect_begin);
      }
      if (reg_expr[i] == '#') {
        m_compat_indeces.push_back(i - m_mandatory_sect_begin);
      }
    }

    //figure out what type of placement we are dealing with
    if (reg_expr[0] == '*') {
      if (reg_expr[reg_expr.size()-1] == '*') {
        my_assert(reg_expr.size() > 2, "Reg_expr too small");
        m_placement_type = BOTH_FREE;
      }
      else {
        my_assert(reg_expr.size() > 1, "Reg_expr too small");
        m_placement_type = LEFT_FREE;
      }
    }
    else {
      if (reg_expr[reg_expr.size()-1] == '*') {
        my_assert(reg_expr.size() > 1, "Reg_expr too small");
        m_placement_type = RIGHT_FREE;
      }
      else {
        my_assert(reg_expr.size() >= 1, "Reg_expr too small");
        m_placement_type = NO_FREE;
      }
    }
  }
  m_mandatory_sect_size = m_mandatory_sect_end - m_mandatory_sect_begin;

  //some post-condition/sanity checks
  my_assert(m_mandatory_sect_size == (m_mandatory_letters.size() + 
                                      count_occurences('#') + count_occurences('_')),
            "Mandatory section should consist of [A-Z#_]");
  my_assert(m_mandatory_sect_end >= m_mandatory_sect_begin,
            "Mandatory section begin/end values did not make sense");
  my_assert(m_mandatory_letters.size() == m_mandatory_letter_indeces.size(),
            "Mandatory-letters vector was not same size as mandatory-letter-indeces");
  my_assert(m_compat_indeces.size() == m_req_sets.size(),
            "Compat-indeces vector was not same size as req-sets");
  my_assert(m_placement_type != INVALID, "placement-type was INVALID");
}

////////////////////////////////////////////////////////////////////////////////
bool Constraint::satisfies(const std::string* word, std::vector<unsigned>& potential_placements) const
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(word->size() >= m_mandatory_sect_size, 
            "word argument was smaller than mandatory section");

  //these two offset variables are important. They represent the index within 
  //the word at which the mandatory section will begin being covered. starting_offset
  //is the latest possible index, ending_offset is the earliest possible index.
  int starting_offset = 0, ending_offset = 0;

  unsigned diff_size = word->size() - m_mandatory_sect_size;

  switch (m_placement_type) {
  case TOTAL_FREEDOM:
    //there are no constraints on this word
    return true;
  case NO_FREE: 
    my_assert(word->size() == m_mandatory_sect_size, 
              "For case NO_FREE, expect word to be same size as mandatory section");
    starting_offset = 0;
    ending_offset   = 0;
    break;
  case RIGHT_FREE:
    starting_offset = 0;
    ending_offset   = 0;
    break;
  case LEFT_FREE:
    starting_offset = word->size() - m_mandatory_sect_size;
    ending_offset   = starting_offset;
    break;
  case BOTH_FREE:
    starting_offset = diff_size;
    ending_offset   = 0;
    //Note: both_free is the only case where m_max_lengths is relevant. In all the other cases,
    //the min/max length filtering done in AI_Player should have already filtered words that
    //don't fit.
    //diff_size is the maximum overrun on either side. We may need to trim the starting/ending
    //range if it means too many characters go off on either side
    my_assert(m_max_lengths.size() == 2,
              "BOTH_FREE implies two '*' which should imply two entries in max-lengths vector");
    if (diff_size > m_max_lengths[0]) {
      starting_offset = m_max_lengths[0];
    }
    if (diff_size > m_max_lengths[1]) {
      ending_offset = (diff_size - m_max_lengths[1]);
    }
    break;
  case INVALID:
    my_assert(false, "Invalid placement-type");
  }

  bool any_found = false;

  //loop from earliest possible placement to latest possible placement of word.
  //if none fits, this word does not satisfy the constraint
  for (int offset = starting_offset; offset >= ending_offset; --offset) {
    bool break_out = false;
    //check that mandatory letters are matched
    for (unsigned i = 0; i < m_mandatory_letters.size(); ++i) {
      if ( (*word)[m_mandatory_letter_indeces[i] + offset] != m_mandatory_letters[i] ) {
        //we did not have the mandatory charater at the required spot
	break_out = true;
        break;
      }
    }
    if (break_out) {
      continue;
    }
    //they did, now check compatibilities, each compatibility may have multiple components
    for (unsigned i = 0; i < m_compat_indeces.size(); ++i) {
      if (m_req_sets[i].find( (*word)[m_compat_indeces[i] + offset] ) ==
          m_req_sets[i].end()) {
        //we failed to meet that compatibility requirement
	break_out = true;
        break;
      }
    }
    if (break_out) {
      continue;
    }
    any_found = true;
    potential_placements.push_back(offset);
  }

  return any_found;
}

////////////////////////////////////////////////////////////////////////////////
unsigned Constraint::count_occurences(char c) const
////////////////////////////////////////////////////////////////////////////////
{
  unsigned count = 0;
  for (unsigned i = 0; i < m_reg_expr.size(); ++i) {
    count += (m_reg_expr[i] == c);
  }
  return count;
}

////////////////////////////////////////////////////////////////////////////////
ostream& Constraint::operator<<(ostream& out) const
////////////////////////////////////////////////////////////////////////////////
{
  unsigned max_len_itr = 0;
  for (unsigned i = 0; i < m_reg_expr.size(); i++) {
    if (m_reg_expr[i] == '*') {
      out << '*' << '{' << m_max_lengths[max_len_itr++] << '}';
    }
    else if (m_reg_expr[i] == '#') {
      out << '#';
    }
    else {
      out << m_reg_expr[i];
    }
  }
  return out;
}

////////////////////////////////////////////////////////////////////////////////
unsigned Constraint::min_word_length() const
////////////////////////////////////////////////////////////////////////////////
{
  unsigned min_length = 0;
  for (unsigned i = 0; i < m_reg_expr.size(); ++i) {
    if (m_reg_expr[i] != '*') {
      ++min_length;
    }
  }
  return min_length;
}

////////////////////////////////////////////////////////////////////////////////
unsigned Constraint::max_word_length() const
////////////////////////////////////////////////////////////////////////////////
{
  unsigned max_length = 0, num_pieces_req = 0, max_len_itr = 0;
  for (unsigned i = 0; i < m_reg_expr.size(); ++i) {
    if (m_reg_expr[i] == '*') {
      unsigned max = m_max_lengths[max_len_itr++]; 
      num_pieces_req += max;
      max_length += max;
    }
    else if (m_reg_expr[i] == '#' || m_reg_expr[i] == '_') {
      num_pieces_req++;
      max_length++;
    }
    else {
      max_length++;
    }
  }
  //no point in taking-into-consideration words that the player could not possibly 
  //form because they require the player to play more than 7 pieces to form.
  if (num_pieces_req > Scrabble_Config::instance().NUM_PLAYER_PIECES()) {
    max_length -= num_pieces_req - Scrabble_Config::instance().NUM_PLAYER_PIECES();
  }
  return max_length;
}

////////////////////////////////////////////////////////////////////////////////
bool Constraint::can_be_eased() const
////////////////////////////////////////////////////////////////////////////////
{
  unsigned num_critical_sections = 0;
  bool within_word_span = false;
  for (unsigned i = 0; i < m_reg_expr.size(); ++i) {
    char c = m_reg_expr[i];
    if (c == '#') {
      ++num_critical_sections;
      within_word_span = false;
    }
    else if (c == '_' || c == '*') {
      within_word_span = false;
    }
    else {
      if (!within_word_span) {
        within_word_span = true;
        ++num_critical_sections;
      }
    }

    if (num_critical_sections > 1) {
      return true;
    }
  }
  my_assert(num_critical_sections == 1,
            "Every valid reg_expr should have at least one critical section");
  return false;
}

////////////////////////////////////////////////////////////////////////////////
Constraint* Constraint::ease(unsigned& mand_sect_offset)
////////////////////////////////////////////////////////////////////////////////
{
  //get index of critical square
  const unsigned crit_idx = m_mandatory_sect_begin + mand_sect_offset;
  
  my_assert(can_be_eased(), "Tried to ease constraint that cannot be eased");
  my_assert(crit_idx < m_mandatory_sect_end,
            "crit-sqr-idx should fall within mandatory section");
#ifndef NDEBUG
  char crit_char = m_reg_expr[crit_idx];
  my_assert(crit_char != '*' && crit_char != '_',
            "crit-sqr component of reg_expr should be highly-constrained");
#endif
  
  //The critical span may involve more than a single character
  //Note that the critical square may never be eased-out
  unsigned crit_span_begin = crit_idx, crit_span_end = crit_idx;
  while (crit_span_begin > 0 && 
         Scrabble_Piece::is_valid_letter(m_reg_expr[crit_span_begin-1])) {
    --crit_span_begin;
  }
  while (crit_span_end < (m_reg_expr.size() - 1) && 
         Scrabble_Piece::is_valid_letter(m_reg_expr[crit_span_end+1])) {
    ++crit_span_end;
  }

#ifdef VERBOSE
  cout << "        Attempting ease with: " 
       << "m_reg_expr=" << m_reg_expr << ", "
       << "crit_span_begin=" << crit_span_begin << ", "
       << "crit_span_end=" << crit_span_end << ", "
       << "mand_sect_offset=" << mand_sect_offset << endl;
#endif

  //MAIN LOOP: loop until we find the next valid easing
  while (true) {
    m_num_easings++; //incr easing iterator
#ifdef VERBOSE
    cout << "          m_num_easings=" << m_num_easings << endl;
#endif

    //create copies of all main state
    std::string new_reg_expr                   = m_reg_expr;
    unsigned    cpy_crit_span_begin            = crit_span_begin;
    unsigned    cpy_crit_span_end              = crit_span_end;
    unsigned    potential_mand_sect_offset_chg = 0;
    std::vector<set<char> > new_req_compats    = m_req_sets;

    
    //figure out visit order in the "easing tree" 
    //(left child = parent eased from left, (right child = parent eased from right)
    stack<unsigned> visit_order;
    unsigned order = m_num_easings;
    while (true) {
      visit_order.push(order);
      if (order == 1 || order == 2) {
        break;
      }
      else {
        order = (order % 2 == 0) ? (order-1) / 2 : order / 2;
      }
    }

    //now perform the visiting
    bool visit_succeeded = true;
    while (!visit_order.empty()) {
      bool ease_from_left   = visit_order.top() % 2 == 1;
#ifdef VERBOSE
      cout << "            Easing from: " << (ease_from_left ? "left" : "right") << endl;
#endif
      visit_order.pop();
      bool was_able_to_ease = false; 
      int delta_i = ease_from_left ? 1 : -1;

      //perform a single easing
      for (unsigned i = ease_from_left ? 0 : new_reg_expr.size() - 1;
           i < cpy_crit_span_begin || i > cpy_crit_span_end; i += delta_i) {
#ifdef VERBOSE
        cout << "              i=" << i << ", examining: " << new_reg_expr[i] << endl;
#endif
        if (new_reg_expr[i] == '#') {
#ifdef VERBOSE
          cout << "              removing: " << new_reg_expr[i] << endl;
#endif
          new_reg_expr.erase(i, 1);
          was_able_to_ease = true;
          my_assert(!new_req_compats.empty(),
                    "Eased away a '#', but nothing in the new-req-compats vector");
          if (ease_from_left) {
            potential_mand_sect_offset_chg++;
#ifdef VERBOSE
            cout << "              reducing offset" << endl;
#endif
            cpy_crit_span_begin--;
            cpy_crit_span_end--;
            new_req_compats.erase(new_req_compats.begin());
          }
          else {
            new_req_compats.pop_back();
          }
          break; //indv easing is done
        }
        else if (new_reg_expr[i] == '*' || new_reg_expr[i] == '_') {
#ifdef VERBOSE
          cout << "              removing: " << new_reg_expr[i] << endl;
#endif
          char temp = new_reg_expr[i];
          new_reg_expr.erase(i, 1);
          if (ease_from_left) {
            if (temp == '_') {
              potential_mand_sect_offset_chg++;
#ifdef VERBOSE
              cout << "              reducing offset" << endl;
#endif
            }
            cpy_crit_span_begin--;
            cpy_crit_span_end--;
            i -= delta_i; //want i to remain the same
          }
        }
        else {
          was_able_to_ease = true;
          //need to erase entire span of chars
          while (Scrabble_Piece::is_valid_letter(new_reg_expr[i])) {
#ifdef VERBOSE
            cout << "              removing: " << new_reg_expr[i] << endl;
#endif
            new_reg_expr.erase(i, 1);
            if (ease_from_left) {
              potential_mand_sect_offset_chg++;
#ifdef VERBOSE
              cout << "              reducing offset" << endl;
#endif
              cpy_crit_span_begin--;
              cpy_crit_span_end--;
            }
            else {
              i += delta_i;
            }
          }

          //remove the touching '_' as well
          if (ease_from_left) {
            if (new_reg_expr[i] == '_') {
#ifdef VERBOSE
              cout << "              removing: " << new_reg_expr[i] << endl;
#endif
              new_reg_expr.erase(i, 1);
#ifdef VERBOSE
              cout << "              reducing offset" << endl;
#endif
              potential_mand_sect_offset_chg++;
              cpy_crit_span_begin--;
              cpy_crit_span_end--;
            }
          }
          else {
            if (new_reg_expr[i] == '_') {
#ifdef VERBOSE
              cout << "              removing: " << new_reg_expr[i] << endl;
#endif
              new_reg_expr.erase(i, 1);
            }
          }
          break; //indv easing is done
        }
      }
      if (!was_able_to_ease) {
#ifdef VERBOSE
        cout << "            Was not able to ease."<< endl;
#endif
        visit_succeeded = false;
        break;
      }
    }
    
    if (visit_succeeded) {
      mand_sect_offset -= potential_mand_sect_offset_chg;
#ifdef VERBOSE
      cout << "          Easing successfully completed, new reg_expr: " << new_reg_expr << endl;
#endif
      //need to recalculate max_lengths
      std::vector<unsigned> new_max_lengths;

      if (new_reg_expr[0] == '*') {
        my_assert(!m_max_lengths.empty(),
                  "Found '*' but max-lengths vector was empty");
        new_max_lengths.push_back(m_max_lengths.front());
      }
      else {
        unsigned num_blanks = 0;
        for (unsigned i = 0; i < new_reg_expr.size() && new_reg_expr[i] == '_'; i++) {
          num_blanks++;
        }
        if (num_blanks != 0) {
          new_reg_expr.erase(0, num_blanks);
          new_reg_expr.insert(0, "*");
          new_max_lengths.push_back(num_blanks);
          mand_sect_offset -= num_blanks;
#ifdef VERBOSE
          cout << "              reducing offset by" << num_blanks << endl;
#endif
        }
      }

      if (new_reg_expr[new_reg_expr.size() - 1] == '*') {
        my_assert(!m_max_lengths.empty(), 
                  "Found '*' but max-lengths vector was empty");
        new_max_lengths.push_back(m_max_lengths.back());
      }
      else {
        unsigned num_blanks = 0;
        for (unsigned i = new_reg_expr.size() - 1; new_reg_expr[i] == '_'; i--) {
          num_blanks++;
        }
        if (num_blanks != 0) {
          new_reg_expr.erase(new_reg_expr.size() - num_blanks, num_blanks);
          new_reg_expr.append(1, '*');
          new_max_lengths.push_back(num_blanks);
        }
      }

#ifdef VERBOSE
      cout << "          About to return new constraint with reg-expr:" << new_reg_expr << endl;
#endif
      return new Constraint(new_reg_expr, new_req_compats, new_max_lengths);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
bool Constraint::is_mandatory_sect_critical_span() const
////////////////////////////////////////////////////////////////////////////////
{
  for (unsigned i = m_mandatory_sect_begin; i < m_mandatory_sect_end; i++) {
    if (!Scrabble_Piece::is_valid_letter(m_reg_expr[i])) {
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////
void Constraint::convert_compat_req_to_set(const set<std::string>& valid_words,
                                           const std::vector<pair<std::string, unsigned> >& req_compatibilities,
                                           std::vector<set<char> >& compatibility_sets)
////////////////////////////////////////////////////////////////////////////////
{
  my_static_assert(compatibility_sets.empty(), 
                   "compatibility_sets should be empty, it's intended to be filled");
  
  compatibility_sets.resize(req_compatibilities.size());

  //loop over each req-compatibility, find all satisfying characters and add them
  //to the corresponding compatibility-set.
  for (unsigned i = 0; i < req_compatibilities.size(); ++i) {
    for (char c = 'A'; c <= 'Z'; ++c) {
      std::string string_with_subst = req_compatibilities[i].first;
      string_with_subst[req_compatibilities[i].second] = c;

      if (valid_words.find(string_with_subst) != valid_words.end()) {
        compatibility_sets[i].insert(c);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
bool Constraint::operator==(const Constraint& rhs) const
////////////////////////////////////////////////////////////////////////////////
{
  return m_reg_expr == rhs.m_reg_expr;
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Constraint& c)
////////////////////////////////////////////////////////////////////////////////
{
  return c.operator<<(out);
}
