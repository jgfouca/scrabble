#include "ai_player.hpp"
#include "scrabble_piece.hpp"
#include "scrabble_board.hpp"
#include "board_loc.hpp"

#include <set>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
AI_Player::~AI_Player()
////////////////////////////////////////////////////////////////////////////////
{
  for (unsigned i = 0, e = m_bitsets.size(); i < e; ++i) {
    delete get<2>(m_bitsets[i]);
  }
}

////////////////////////////////////////////////////////////////////////////////
void AI_Player::initialize()
////////////////////////////////////////////////////////////////////////////////
{
  //allocate memory
  unsigned bitset_itr = 0;
  m_bitsets.resize(m_valid_words.size());
  m_word_reqs.resize(m_valid_words.size());
  m_word_req_indeces.resize(m_valid_words.size());
  m_recent_result.reserve(m_valid_words.size());
  m_result_placements.reserve(m_valid_words.size());

  //loop over all valid words, pre-computing things that will help us later
  for (set<std::string>::const_iterator word_itr = m_valid_words.begin();
       word_itr != m_valid_words.end(); ++word_itr, ++bitset_itr) {
    //we make a copy of the std::string on the heap to avoid having to make additional
    //copies during each individual search
    std::string* new_string = new std::string(*word_itr);
    get<2>(m_bitsets[bitset_itr]) = new_string;
    get<1>(m_bitsets[bitset_itr]) = new_string->size();

    std::bitset<26>& the_set = get<0>(m_bitsets[bitset_itr]);
#ifndef NDEBUG
    for (unsigned i = 0; i < the_set.size(); ++i) {
      my_assert(the_set[i] == 0,
                "bitsets not initializing to all zeroes");
    }
#endif
    m_word_reqs[bitset_itr].resize(26, 0);

    //perform all front-loaded work
    for (unsigned i = 0; i < word_itr->size(); i++) {
      //set the appropriate bits in the bitset for this word
      my_assert(Scrabble_Piece::is_valid_letter( (*word_itr)[i] ),
                std::string("dictionary word ") + *word_itr + " had invalid character");
      unsigned idx = (*word_itr)[i] - 'A';
      my_assert(idx < 26, "idx should always be between 0 and 25");
      the_set.set(idx);

      m_word_reqs[bitset_itr][idx]++;
    }

    //record all instances of word requiring 1 or more letters
    for (unsigned char i = 0; i < 26; ++i) {
      if (m_word_reqs[bitset_itr][i] > 0) {
        m_word_req_indeces[bitset_itr].push_back(i);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void AI_Player::make_play()
////////////////////////////////////////////////////////////////////////////////
{
  //allocate some useful objects
  Indv_Play tmp_play;
  unsigned highest_so_far = 0;
  std::string highest_word = "";
  set<pair<Board_Loc, Board_Loc> > constraints_checked;

  if (m_the_game->is_first_play()) {
    //Need special case for first-play
    //prioritize long words
    //if none, try a smaller word
    const unsigned center_square = m_the_game->get_board()->get_board_dim() / 2;
    for (int l = 7; l > 0; l--) {
      std::vector<unsigned> max_lengths;
      max_lengths.push_back(l);
      Constraint constraint("*", std::vector<set<char> >(), max_lengths);
      find_all_satisfying_strings(constraint, l, l);

      if (!m_recent_result.empty()) {
        //loop over all satisfying words, looking for highest-point placement
        for (unsigned r = 0; r < m_recent_result.size(); r++) {
          const std::string* word = m_recent_result[r];
          my_assert(word->size() == static_cast<unsigned>(l),
                    "word did not have expected length");

          //this word may be placed in one of len(word) places
          for (int p = (-l+1); p <= 0; p++) {
            tmp_play.clear();

            for (unsigned w = 0; w < word->size(); w++) {
              //note that the call below can change wildcard status
              tmp_play.place_piece(center_square, center_square+p+w, get_piece((*word)[w]));
            }
	    remap();

            unsigned score = m_the_game->get_potential_score(tmp_play);
            if (score > highest_so_far) {
              highest_so_far = score;
	      highest_word   = *word;
              m_current_play = tmp_play; //play is stored here
	    }
          }
        }
        if (l == 7) {
          break; //the 50 pt bonus will not be improved upon
        }
      }
    }
  }
  else {
#ifdef VERBOSE
    cout << "***********************************************************************" << endl;
    cout << "Beginning general board search!" << endl;
#endif
    //loop over every square on the entire board.
    const Scrabble_Board* the_board = m_the_game->get_board();
    unsigned b_dim = the_board->get_board_dim();
    for (unsigned row = 0; row < b_dim; row++) {
      for (unsigned col = 0; col < b_dim; col++) {
        //we can play off of any placed piece
        if (!the_board->is_free(row, col)) {
#ifdef VERBOSE
          cout << "  Looking at location: " << Board_Loc(row, col) << endl;
#endif
          //There are 6 ways to play off a piece
          //1) We can play along a vertical line that crosses directly over this piece. In order
          //   to make this play, the formed word must contain all the pre-existing connected
          //   vertical components. Note: if this piece already has an upper and lower neighbor,
          //   we will not search the vertical line. We will wait until we see an end piece to
          //   do the vertical line.
          //2) Playing along the horizontal line is basically the same as playing along the veritcal.
          //3) We can play along vertical or horizontal lines that parallel are adjacent to the
          //   existing piece IF the square immediately adjacent to the existing piece that the lines
          //   cross over is free. (This gives us 4 additional potential lines to look at).

          //Look at all of these potential play lines and build up vectors that have information
          //about the critical square for every viable play-line..
          std::vector<Board_Loc> critical_squares;
          std::vector<Board_Loc> enabling_free_squares; //the free square that enabled the play
          std::vector<bool> is_horiz;
          std::vector<bool> is_across;
          std::vector<bool> is_neg_dir; //free squares in the negative direction

          //we need to ask the board for adjacent free squares since we can only play on free sqrs.
          //each free adjacent square opens-up the possibility of two plays
          std::vector<Board_Loc> free_sqrs = the_board->get_adjacent(row, col, false, true);
          for (unsigned f = 0; f < free_sqrs.size(); f++) {
            for (unsigned p = 0; p < 2; p++) {
              critical_squares.push_back(Board_Loc(p == 0 ? row : free_sqrs[f].row(),
                                                   p == 0 ? col : free_sqrs[f].col()));
              enabling_free_squares.push_back(free_sqrs[f]);
              is_horiz.push_back(p == 0 ? row == free_sqrs[f].row() : row != free_sqrs[f].row());
              is_across.push_back(p == 0);
              is_neg_dir.push_back(row != free_sqrs[f].row() ? free_sqrs[f].row() < row :
                                   free_sqrs[f].col() < col);
#ifdef VERBOSE
              std::string dir_str    = is_neg_dir.back() ? "negative"   : "positive";
              std::string vert_str   = is_horiz.back()   ? "horizontal" : "vertical";
              std::string across_str = is_across.back()  ? "across"     : "adjacent";
              cout << "    " << is_horiz.size() << ") Potential " << dir_str << " " << vert_str
                   << "-line play " << across_str << " " << critical_squares.back() << endl;
#endif
            }
          }

          //Now that we have found all possible play-lines, we need to formulate constraints for
          //the possible plays along that line
          my_assert(is_horiz.size()              == critical_squares.size(),
                    "vectors did not have matching size");
          my_assert(is_across.size()             == is_horiz.size(),
                    "vectors did not have matching size");
          my_assert(is_neg_dir.size()            == is_horiz.size(),
                    "vectors did not have matching size");
          my_assert(enabling_free_squares.size() == is_horiz.size(),
                    "vectors did not have matching size");
          //loop over all possible play-lines
          for (unsigned line = 0; line < critical_squares.size(); line++) {
#ifdef VERBOSE
            cout << "      Examining line (" << line+1 << ")" << endl;
#endif
            std::string mandatory_sect; //reg_expr for the mandatory section of this line
            const unsigned critical_row = critical_squares[line].row();
            const unsigned critical_col = critical_squares[line].col();
            const bool horizontal       = is_horiz[line];
            const bool across           = is_across[line];

            //we need to keep track of how much the mand grows out from the critical square
            //in the negative direction. This will be used as a point of reference later
            //when we are trying to figure out where to put a word.
            unsigned primary_mand_section_exp = 0;

            //orig_req is used to store the original compatibility requirement that is
            //and innate property of all "adjacent-line plays"
            pair<std::string, unsigned> orig_req;

            //Get the mandatory section of the line
            if (across) {
              //"across" plays potentially have large mandatory sections
              Scrabble_Word connected_seq;
              the_board->add_all_pieces_in_direction(enabling_free_squares[line].row(),
                                                     enabling_free_squares[line].col(),
                                                     critical_row, critical_col, connected_seq);
              mandatory_sect = connected_seq.get_word_str();
              if (!is_neg_dir[line]) {
                primary_mand_section_exp = mandatory_sect.size() - 1;
              }
            }
            else {
              mandatory_sect = "#";
              //this type of play immediately involves requirement compatibilities
              std::vector<Board_Loc> adj_pieces = the_board->get_adjacent(critical_row, critical_col);
              if (adj_pieces.size() > 1) {
                continue; // this line is over-constrained
              }
              my_assert(adj_pieces.size() == 1, "Expected a single adjacent piece");
              my_assert(enabling_free_squares[line].row() == critical_row &&
                        enabling_free_squares[line].col() == critical_col,
                        "critical square should have been the enabling free-square");

              //get the neighboring word.
              Scrabble_Word neighbor_sw;
              the_board->add_all_pieces_in_direction(critical_row, critical_col,
                                                     adj_pieces[0].row(), adj_pieces[0].col(),
                                                     neighbor_sw);
              std::string neighbor_word = neighbor_sw.get_word_str();

              unsigned idx;
              if (is_neg_dir[line]) {
                idx = 0;
                neighbor_word.insert(0, "#");
              }
              else {
                idx = neighbor_word.size();
                neighbor_word.append(1, '#');
              }
              orig_req.first  = neighbor_word;
              orig_req.second = idx;
#ifdef VERBOSE
              cout << "        Found orig-req: (" << orig_req.first << ", " << orig_req.second << ")" << endl;
#endif
            }
            my_assert(mandatory_sect.size() >= 1, "Invalid: Null mandatory section");
#ifdef VERBOSE
            cout << "        Found mandatory section: " << mandatory_sect
                 << ", primary_mand_sect_exp=" << primary_mand_section_exp << endl;
#endif

            //Search the line, collecting squares until:
            //1) We have processed 7 squares
            //2) We have hit our CONSTRAINED_SQR_LIMIT-th highly constrained square
            //3) We have reached the end of the board
            //Searching will be done as follows:
            // Search in one direction until the conditions above have been met. If we have
            // constrained-sqrs to spare, begin searching the opposite direction. After each
            // iteration of the search, reduce the cap on contrained squares going in the initial
            // search direction. This allows us to expand the search in the second direction.

            //Each search needs a row/col starting point and a search direction
            //calculate search directions
            int initial_row_search_dir = 0, initial_col_search_dir = 0;
            if (horizontal) {
              initial_col_search_dir = -1;
            }
            else {
              initial_row_search_dir = -1;
            }
            int second_row_search_dir = -initial_row_search_dir;
            int second_col_search_dir = -initial_col_search_dir;
            //calculate starting points
            unsigned initial_row_search_start = critical_row;
            unsigned initial_col_search_start = critical_col;
            unsigned second_row_search_start = critical_row;
            unsigned second_col_search_start = critical_col;
            if (is_neg_dir[line]) {
              initial_row_search_start += initial_row_search_dir;
              initial_col_search_start += initial_col_search_dir;
              if (horizontal) {
                second_col_search_start += mandatory_sect.size();
              }
              else {
                second_row_search_start += mandatory_sect.size();
              }
            }
            else {
              if (horizontal) {
                initial_col_search_start -= mandatory_sect.size();
              }
              else {
                initial_row_search_start -= mandatory_sect.size();
              }
              second_row_search_start += second_row_search_dir;
              second_col_search_start += second_col_search_dir;
            }
#ifdef VERBOSE
            cout << "        Initial search will be at "
                 << Board_Loc(initial_row_search_start, initial_col_search_start)
                 << " in row-direction: " << initial_row_search_dir
                 << " in col-direction: " << initial_col_search_dir << endl;
            cout << "        Second search will be at "
                 << Board_Loc(second_row_search_start, second_col_search_start)
                 << " in row-direction: " << second_row_search_dir
                 << " in col-direction: " << second_col_search_dir << endl;
#endif

            //we are already constrained by whatever is in the mandatory sect
            //JGF: might want to change this since it would make any fancy play that involved a
            //large mand-section impossible
            unsigned num_constrained_sqrs = mandatory_sect.size();

	    //need a std::vector to hold the constraint objects created by each search
	    std::vector<Constraint*> constraints;
            //need a std::vector to hold expansions in the mandatory section in the "upstream" direction.
            //need this information so we know where to start placing the word later
            std::vector<unsigned> mandatory_sect_expansions;

            const unsigned NUM_CONSTRAINED_LIMIT = Scrabble_Config::instance().CONSTRAINED_SQUARE_LIMIT();
            for (int max_constrained_sqrs_in_negative_dir = NUM_CONSTRAINED_LIMIT;
                 max_constrained_sqrs_in_negative_dir >= 0; --max_constrained_sqrs_in_negative_dir) {
#ifdef VERBOSE
              cout << "          Beginning search with max-constrained-sqrs-in-negative-direction: "
                   << max_constrained_sqrs_in_negative_dir << endl;
#endif
              std::string reg_expr = mandatory_sect;
              unsigned num_constrained_sqrs_found = num_constrained_sqrs;
              unsigned num_req_placements = (!across) ? 1 : 0;
              std::vector<pair<std::string, unsigned> > req_compatibilities;
              std::vector<unsigned> max_lengths;
              if (!across) {
                my_assert(orig_req.first != "", "orig_req should have been populated");
                req_compatibilities.push_back(orig_req);
#ifdef VERBOSE
                cout << "          Starting off with req-compat: " << req_compatibilities[0].first << endl;
#endif
              }
              unsigned mand_sect_exp = primary_mand_section_exp;

              for (unsigned s = 0; s < 2; s++) {
                bool search_in_negative_dir = (s == 0);
                unsigned init_row = search_in_negative_dir ? initial_row_search_start : second_row_search_start;
                unsigned init_col = search_in_negative_dir ? initial_col_search_start : second_col_search_start;
                int row_dir = search_in_negative_dir ? initial_row_search_dir : second_row_search_dir;
                int col_dir = search_in_negative_dir ? initial_col_search_dir : second_col_search_dir;
                unsigned max_constrained = search_in_negative_dir ? max_constrained_sqrs_in_negative_dir : NUM_CONSTRAINED_LIMIT;
#ifdef VERBOSE
                std::string is_neg_str = (s == 0) ? "negative" : "postive";
                cout << "            Performing "  << is_neg_str << " phase of search." << endl;
                cout << "              Params: "
                     << "init_row: "        << init_row << ", "
                     << "init_col: "        << init_col << ", "
                     << "row_dir: "         << row_dir  << ", "
                     << "col_dir: "         << col_dir  << ", "
                     << "max_constrained: " << max_constrained << endl;
#endif


                //perform the initial search
                for (unsigned r = init_row, c = init_col;
                     r < b_dim && c < b_dim &&
                       num_req_placements < 7;
                     r += row_dir, c += col_dir) {
#ifdef VERBOSE
                  cout << "              Searching square: " << Board_Loc(r, c)
                       << ". So far num-constrained-sqrs-found=" << num_constrained_sqrs_found
                       << " and num-req-placements=" << num_req_placements << endl;
#endif
                  my_assert(the_board->is_free(r, c), "should only be looking at free squares");

                  //if square is totally open, handling it is simple
                  if (the_board->is_unconstrained(r, c)) {
                    add_to_reg_expr(reg_expr, search_in_negative_dir, "_", mand_sect_exp);
                    num_req_placements++;
                  }
                  else {
                    //For each adjacent piece there are 3 cases:
                    //1) We are adjacent to a piece that's already be processed by by the search
                    //2) We are adjacent to pieces perpendicular to the search line
                    //3) We are adjacent to pieces further down the search-line
                    //4) Both cases 2 and 3 are true

                    std::vector<Board_Loc> adj_pieces = the_board->get_adjacent(r, c);
                    //Remove any instances of case 1
                    for (std::vector<Board_Loc>::iterator itr = adj_pieces.begin();
                         itr != adj_pieces.end(); itr++) {
                      if ( (horizontal && itr->row() == r &&
                            itr->col() == (c - col_dir)) ||
                           (!horizontal && itr->col() == c &&
                            itr->row() == (r - row_dir)) ) {
                        adj_pieces.erase(itr);
                        break;
                      }
                    }
                    //if we still have 2 or more neighbors, this sqr is overly constrained
                    if (adj_pieces.size() > 1) {
#ifdef VERBOSE
                      cout << "                Ran into an overly constrained square... aborting "
                           << "the search in this direction." << endl;
#endif
                      break;
                    }
                    else if (adj_pieces.size() == 1) {
                      if (num_constrained_sqrs_found >= max_constrained) {
#ifdef VERBOSE
                        cout << "                Ran into too many constrained squares... aborting "
                             << "the search in this direction." << endl;
#endif
                        break; //we have hit our max number of constrained sqrs
                      }

                      if ( (horizontal && adj_pieces[0].row() == r &&
                            adj_pieces[0].col() == (c + col_dir)) ||
                           (!horizontal && adj_pieces[0].col() == c &&
                            adj_pieces[0].row() == (r + row_dir)) ) {
                        //we are dealing with case 3, grab the upcoming sequence
                        Scrabble_Word upcoming_sw;
                        the_board->add_all_pieces_in_direction(r, c,
                                                               adj_pieces[0].row(),
                                                               adj_pieces[0].col(),
                                                               upcoming_sw);
                        std::string upcoming = upcoming_sw.get_word_str();
                        if (upcoming.size() + num_constrained_sqrs_found <=
                            max_constrained) {
                          add_to_reg_expr(reg_expr, search_in_negative_dir, "_", mand_sect_exp);
                          add_to_reg_expr(reg_expr, search_in_negative_dir, upcoming, mand_sect_exp);

                          num_req_placements++;
                          num_constrained_sqrs_found += upcoming.size();
                          r += row_dir * upcoming.size();
                          c += col_dir * upcoming.size();
                        }
                        else {
#ifdef VERBOSE
                          cout << "                Ran into a long word '" << upcoming
                               << "' ... aborting the search in this direction." << endl;
#endif
                          break; //absorbing that entire word would over-constrain us
                        }
                      }
                      else {
                        my_assert( (horizontal && adj_pieces[0].row() != r) ||
                                   (!horizontal && adj_pieces[0].col() != c),
                                   "adj-sqr should be different than the examined sqr");
                        //we are dealing with case 2, get the perpendicular word and compute the
                        //necessary compatibility-requirement.

                        //figure out if perpendicular word is going in positive or negative direction
                        //from the p.o.v. of the sqr we are at
                        bool is_negative = horizontal ? adj_pieces[0].row() < r :
                                                        adj_pieces[0].col() < c;

                        //compute the perpendicular word
                        Scrabble_Word perp_word_sw;
                        the_board->add_all_pieces_in_direction(r, c,
                                                               adj_pieces[0].row(),
                                                               adj_pieces[0].col(),
                                                               perp_word_sw);
                        std::string perp_word = perp_word_sw.get_word_str();
                        //figure out where a letter played on the sqr we are at would lie within
                        //the perpendicular word
                        unsigned idx;
                        if (!is_negative) {
                          idx = 0;
                          perp_word.insert(0, "#");
                        }
                        else {
                          idx = perp_word.size();
                          perp_word.append("#");
                        }

                        //manipulate state accordingly
                        if (search_in_negative_dir) {
                          req_compatibilities.insert(req_compatibilities.begin(),
						     pair<std::string,unsigned>(perp_word, idx));
                        }
                        else {
                          req_compatibilities.push_back(pair<std::string,unsigned>(perp_word, idx));
                        }
                        add_to_reg_expr(reg_expr, search_in_negative_dir, "#", mand_sect_exp);
                        num_req_placements++;
                        num_constrained_sqrs_found++;
                      }
                    }
                    else {
                      my_assert(adj_pieces.empty(),
                                "case 1 should have had only adj piece removed from adj_pieces");
                      //if case 1, treat as normal unconstrained sqr
                      add_to_reg_expr(reg_expr, search_in_negative_dir, "_", mand_sect_exp);
                      num_req_placements++;
                    }
                  }
                }
                //put * at beginning and end if possible, adjust num_req_placements accordingly
		if (search_in_negative_dir) {
		  unsigned num_blanks = 0;
		  for (unsigned i = 0; i < reg_expr.size() && reg_expr[i] == '_'; i++) {
		    num_blanks++;
		  }
		  if (num_blanks != 0) {
		    reg_expr.erase(0, num_blanks);
		    reg_expr.insert(0, "*");
		    max_lengths.push_back(num_blanks);
		    num_req_placements -= num_blanks;
                    mand_sect_exp -= num_blanks;
		  }
		}
		else {
		  unsigned num_blanks = 0;
		  for (unsigned i = reg_expr.size() - 1; reg_expr[i] == '_'; i--) {
		    num_blanks++;
		  }
		  if (num_blanks != 0) {
		    reg_expr.erase(reg_expr.size() - num_blanks, num_blanks);
		    reg_expr.append(1, '*');
		    max_lengths.push_back(num_blanks);
		    num_req_placements -= num_blanks;
		  }
		}
              }

              //Both neg, pos search done. Record the constraint the resulted from this search
              //note that we neet to convert our req-compatibilities into req-sets first
              std::vector<set<char> > req_sets;
              Constraint::convert_compat_req_to_set(m_valid_words, req_compatibilities, req_sets);
	      constraints.push_back(new Constraint(reg_expr, req_sets, max_lengths));
              mandatory_sect_expansions.push_back(mand_sect_exp);
#ifdef VERBOSE
              cout << "          Created constraint: " << *(constraints.back()) << endl;
              cout << "          Total expansion of mandatory section was " << mand_sect_exp << endl;
#endif
            }
#ifdef VERBOSE
            cout << "        Finished searching potential line." << endl;
#endif
            // All searches done for this line
	    //remove all duplicate constraint objects. it's OK to be inefficient here
            std::vector<unsigned>::iterator ms_i_itr = mandatory_sect_expansions.begin();
            for (std::vector<Constraint*>::iterator i_itr = constraints.begin();
                 i_itr != constraints.end(); i_itr++, ms_i_itr++) {
              std::vector<unsigned>::iterator    ms_j_itr = ms_i_itr + 1;
              std::vector<Constraint*>::iterator j_itr    = i_itr    + 1;
              for ( ; j_itr != constraints.end(); j_itr++, ms_j_itr++) {
                if (*(*i_itr) == *(*j_itr)) {
                  constraints.erase(j_itr--);
                  mandatory_sect_expansions.erase(ms_j_itr--);
                }
              }
            }
#ifdef VERBOSE
            cout << "        After removing duplicates we have: " << endl;
            for (unsigned x = 0; x < constraints.size(); x++) {
              cout << "          " << *constraints[x] << endl;
              cout << "          with mandatory section expansion: " << mandatory_sect_expansions[x] << endl;
            }
#endif

            //loop over all unique constraints
            my_assert(constraints.size() == mandatory_sect_expansions.size(),
                      "these two vectors should have had same number of items removed");
            for (unsigned c = 0; c < constraints.size(); c++) {
              Constraint* original_constraint = constraints[c];
              unsigned original_mand_exp      = mandatory_sect_expansions[c];

              Constraint* constraint = original_constraint;
              unsigned mand_exp      = original_mand_exp;
              while (true) {
#ifdef VERBOSE
                cout << "        Searching by constraint: "  << *constraint << endl;
                cout << "        Mandatory sect expan is : " << mand_exp << endl;
#endif
                //check that we have not already checked a similar constraint
                unsigned mand_sect_row_begin = !horizontal ? critical_row - mand_exp : critical_row;
                unsigned mand_sect_col_begin = horizontal  ? critical_col - mand_exp : critical_col;
                unsigned mand_sect_row_end   = !horizontal ? mand_sect_row_begin + constraint->mand_sect_size() : critical_row;
                unsigned mand_sect_col_end   = horizontal  ? mand_sect_col_begin + constraint->mand_sect_size() : critical_col;
                pair<Board_Loc, Board_Loc> bounds(Board_Loc(mand_sect_row_begin, mand_sect_col_begin),
                                                  Board_Loc(mand_sect_row_end,   mand_sect_col_end));
                if (constraints_checked.find(bounds) == constraints_checked.end()) {
                  constraints_checked.insert(bounds);

                  unsigned min_len = constraint->min_word_length();
                  my_assert(min_len >= mandatory_sect.size(),
                            "contraint should be reporting a min_length that at least covers the mandatort section");
                  if (across && min_len == mandatory_sect.size() &&
                      constraint->is_mandatory_sect_critical_span()) {
                    min_len++; //doesn't do any good to find words that don't add on to what's already there
                  }
                  if (min_len > constraint->max_word_length()) {
                    //no possible match
                    break;
                  }

                  //get all satisfying words! Store them in m_recent_result
                  find_all_satisfying_strings(*constraint,
                                              min_len,
                                              constraint->max_word_length());

                  if (!m_recent_result.empty()) {
                    //loop over all satisfying words, looking for highest-point placement
                    for (unsigned r = 0; r < m_recent_result.size(); r++) {
                      const std::string* word = m_recent_result[r];
                      my_assert(word->size() >= constraint->min_word_length() &&
                                word->size() <= constraint->max_word_length(),
                                "word did not meet length constraint?");

                      //this word may be placed in one of many places
                      my_assert(m_result_placements[r].size() > 0,
                                "word should have come with at least one valid placement");
                      for (unsigned p = 0; p < m_result_placements[r].size(); p++) {
                        unsigned additional_offset = m_result_placements[r][p];
                        tmp_play.clear();

                        //Calculate where exactly on the board this play will begin
                        unsigned play_row_start =
                          !horizontal ? critical_row - mand_exp - additional_offset : critical_row;
                        unsigned play_col_start =
                          horizontal ? critical_col - mand_exp - additional_offset : critical_col;
                        int play_row_dir = horizontal ? 0 : 1;
                        int play_col_dir = horizontal ? 1 : 0;
                        std::string played_letters;

                        my_assert(play_row_start < b_dim && play_col_start < b_dim,
                                  "start of play is outside of board!");

                        for (unsigned w = 0; w < word->size(); w++) {
                          unsigned play_row = play_row_start + w*play_row_dir;
                          unsigned play_col = play_col_start + w*play_col_dir;

                          if (the_board->is_free(play_row, play_col)) {
                            tmp_play.place_piece(play_row, play_col, get_piece((*word)[w]));
                            played_letters += (*word)[w];
                          }
#ifndef NDEBUG
                          else {
                            my_assert(the_board->get_piece(play_row, play_col)->get_letter() == (*word)[w],
                                      "The pieces already on the board should be consistent with word");
                          }
#endif
                        }
                        remap();

#ifdef VERBOSE
                        cout << "         Attempting play: " << tmp_play << endl;
#endif
                        unsigned score = m_the_game->get_potential_score(tmp_play);
                        if (score > highest_so_far) {
                          highest_so_far = score;
                          highest_word   = played_letters;
                          m_current_play = tmp_play; //play is stored here
                        }
                      }
                    }
                    break;
                  }
                }

                //need to ease the constraint, if cannot be eased any more, break.
                //Changing the reg_expr may require the modification of mand_sect_expan
#ifdef VERBOSE
                cout << "        Easing the constraint: " << *constraint << endl;
#endif
                if (constraint->can_be_eased()) {
                  //note: we must always "ease" from the point of view of the original constraint.
                  //this is because certain iterations of the easing process may actually restore
                  //pieces of state removed by previous easings.
                  //note: it is possible that an "easier" play may result in a higher score.
                  mand_exp = original_mand_exp;
                  if (constraint != original_constraint) {
                    delete constraint;
                  }
                  constraint = original_constraint->ease(mand_exp);
#ifdef VERBOSE
                  cout << "        New, eased, constraint: " << *constraint << endl;
#endif
                }
                else {
                  if (constraint != original_constraint) {
                    delete constraint;
                  }
                  break;
                }
              }
              delete original_constraint;
            }
#ifdef VERBOSE
            cout << "      Finished examining line (" << line+1 << ")" << endl;
#endif
          }
          //all lines done
#ifdef VERBOSE
          cout << "  Finished examining location: " << Board_Loc(row,col) << endl;
#endif
        }
      }
    }
#ifdef VERBOSE
    cout << "Finished general board search!" << endl;
    cout << "***********************************************************************" << endl;
#endif
    //all played pieces have been examined
  }

  for (unsigned w = 0; w < highest_word.size(); w++) {
    //note: the call below should change back any wildcards to
    //what they should be to make the highest play
    get_piece(highest_word[w]);
  }
}

////////////////////////////////////////////////////////////////////////////////
void AI_Player::find_all_satisfying_strings(const Constraint& constraint,
                                            unsigned min_length,
                                            unsigned max_length) const
////////////////////////////////////////////////////////////////////////////////
{
  //clear previous results
  m_recent_result.clear();
  m_result_placements.clear();

  // Put available letters into a std::vector of counts and mark them in a bitset
  unsigned tolerance = 0;
  std::vector<unsigned> available_letters(26, 0);
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    if (m_pieces[i]) {
      if (m_pieces[i]->is_wildcard()) {
      tolerance++;
      }
      else {
        available_letters[m_pieces[i]->get_letter() - 'A']++;
      }
    }
  }

  //get mandatory letters as a bitset
  const bitset<26>& mandatory_bitset = constraint.get_mandatory_bitset();

  //find indeces
  std::vector<unsigned char> mandatory_bits;
  for (unsigned i = 0; i < 26; i++) {
    if (mandatory_bitset.test(i)) {
      mandatory_bits.push_back(i);
    }
  }

  //get mandatory letters as a std::vector, add them to available_letters
  const std::vector<char>& mandatory_letters = constraint.get_mandatory_letters();
  for (unsigned i = 0; i < mandatory_letters.size(); i++) {
    available_letters[mandatory_letters[i] - 'A']++;
  }

  // Loop over all words, any with a matching bitset and appropriate size will pass the
  // first if-statement which is designed to be cheap. Then check if it is possible to
  // form the word given our pieces and what's on the board.
  // Watch this loop closely, it should be the critical path.
  for (unsigned i = 0, e = m_bitsets.size(); i < e; ++i) {
    //Do initial cheap filters
    size_t word_size = get<1>(m_bitsets[i]);
    if (word_size >= min_length && word_size <= max_length) {

      const std::string* word = get<2>(m_bitsets[i]);

      if (!mandatory_letters.empty() &&
          !is_subset(mandatory_bits, get<0>(m_bitsets[i]))) {
        continue;
      }

      //Do possible-formulation filter
      bool break_out = false;
      unsigned char missing = 0; //number of letters we lack to complete the word
      //only loop over indeces where there is a possible mismatch in char reqs
      for (unsigned char j = 0; j < m_word_req_indeces[i].size(); ++j) {
        const unsigned char num_req   = m_word_reqs[i][m_word_req_indeces[i][j]];
        const unsigned char num_avail = available_letters[m_word_req_indeces[i][j]];
        if (num_req > num_avail) {
          missing += num_req - num_avail;
          if (missing > tolerance) {
            //if the word requires more of a certain letter than is available, we cannot
            //possibly make this word
            break_out = true;
            break;
          }
        }
      }
      if (break_out) {
	continue;
      }
      //at this point, we have passed the possible-formulation filter
      //only thing left is to see if this word satisfies the constraint object
      std::vector<unsigned> potential_placements;
      if (constraint.satisfies(word, potential_placements)) {
        //this word has satisfied all constraints
        m_recent_result.push_back(word);
        m_result_placements.push_back(potential_placements);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void AI_Player::add_to_reg_expr(std::string& reg_expr, bool negative_dir,
                                const std::string& new_component,
                                unsigned& mandatory_sect_expansion)
////////////////////////////////////////////////////////////////////////////////
{
  if (negative_dir) {
    reg_expr.insert(0, new_component);
    mandatory_sect_expansion += new_component.size();
  }
  else {
    reg_expr.append(new_component);
  }
#ifdef VERBOSE
  cout << "                Regular expr changed, new value: " << reg_expr << endl;
#endif
}

////////////////////////////////////////////////////////////////////////////////
bool AI_Player::is_subset(const std::vector<unsigned char>& mandatory_bits,
                          const bitset<26>& superset)
////////////////////////////////////////////////////////////////////////////////
{
  for (unsigned char i = 0; i < mandatory_bits.size(); ++i) {
    if (!superset.test(mandatory_bits[i])) {
      return false;
    }
  }
  return true;
}
