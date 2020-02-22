#include "scrabble_board.hpp"
#include "scrabble_square.hpp"

#include <iomanip>
#include <sstream>
#include <set>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
Scrabble_Board::Scrabble_Board(const Board_Builder& board_builder_instance)
////////////////////////////////////////////////////////////////////////////////
{
  board_builder_instance.build_board(this);
}

////////////////////////////////////////////////////////////////////////////////
std::vector<Scrabble_Word> Scrabble_Board::get_created_words(const Indv_Play& the_play) const
////////////////////////////////////////////////////////////////////////////////
{
  const unsigned num_played_letters = the_play.get_size();
  const unsigned b_dim              = m_board.size();

  //determine the line of the primary word
  bool const_row = false, const_col = false;
  unsigned primary_line_id = static_cast<unsigned>(-1);
  if (num_played_letters > 1) {
    const_row       = the_play.get_ith_row(0) == the_play.get_ith_row(1);
    const_col       = the_play.get_ith_col(0) == the_play.get_ith_col(1);
    primary_line_id = (const_row) ? the_play.get_ith_row(0) : the_play.get_ith_col(0);

    my_assert(const_row || const_col, std::string("Play: ") + obj_to_str(the_play) +
              ", was not vertical or horizontal??");
    my_assert(!(const_row && const_col), std::string("Play: ") + obj_to_str(the_play) +
              ", was both vertical or horizontal??");
  }

  std::vector<Scrabble_Word> rv; //will hold all of the created words
  Scrabble_Word primary_word;    //the word formed along the primary line

  //find the entire word formed along the primary line AND any secondary words
  //that may be formed also (perpendicular to the primary line)
  for (unsigned i = 0; i < num_played_letters; i++) {
    unsigned row = the_play.get_ith_row(i), col = the_play.get_ith_col(i);

    //if we have a single-letter play, there is no primary word
    if (num_played_letters > 1) {
      //not a single-letter play, look along the primary-play-line.
      //every played piece is part of the primary word
      primary_word.add_component(the_play.get_ith_piece(i),
                                 m_board[row][col].get_bonus(),
                                 const_row ? col : row);

      //check for already-placed pieces along the primary line, get all adjacent pieces
      std::vector<Board_Loc> adjacent_pieces = get_adjacent(row, col);
      for (unsigned a = 0; a < adjacent_pieces.size(); a++) {
        //if any adjacent piece has the same 'id'
        if ( (const_row && adjacent_pieces[a].row() == primary_line_id) ||
             (const_col && adjacent_pieces[a].col() == primary_line_id) ) {
          //we have found already-placed pieces that are a part of the primary word
          add_all_pieces_in_direction(row, col, adjacent_pieces[a].row(), adjacent_pieces[a].col(),
                                      primary_word);
        }
      }
    }

    //check for secondary words
    std::vector<Board_Loc> adjacent_pieces = get_adjacent(row, col, true);
    for (unsigned a = 0; a < adjacent_pieces.size(); a++) {
      //check if neighbor is NOT along the primary line
      if (!((const_row && adjacent_pieces[a].row() == primary_line_id) ||
            (const_col && adjacent_pieces[a].col() == primary_line_id)) ) {
        //figure out which direction the secondary word is on
        bool vertical   = adjacent_pieces[a].row() != row;
        bool horizontal = adjacent_pieces[a].col() != col;
        my_assert(vertical || horizontal, std::string("Neighboring piece: ") + obj_to_str(adjacent_pieces[a]) +
                  " was neither a veritical or horizonal neighbor??");
        my_assert(!(vertical && horizontal), std::string("Neighboring piece: ") + obj_to_str(adjacent_pieces[a]) +
                  " was both a veritical andd horizonal neighbor??");

        //we have found a new secondary word
        Scrabble_Word secondary_word;

        //add the placed letter
        secondary_word.add_component(the_play.get_ith_piece(i),
                                     m_board[row][col].get_bonus(),
                                     horizontal ? col : row);

        //add all pieces in the neighbor's direction
        add_all_pieces_in_direction(row, col, adjacent_pieces[a].row(), adjacent_pieces[a].col(),
                                    secondary_word);

        //now try to add all pieces opposite of the neighbor's direction
        if ( (vertical && adjacent_pieces[a].row() < row) ||
             (horizontal && adjacent_pieces[a].col() < col) ) {
          //we had additional neighbors along the same secondary line in the
          //opposite direction of the original neighbor
          unsigned opp_row = vertical  ? adjacent_pieces[a].row() + 2 : adjacent_pieces[a].row();
          unsigned opp_col = !vertical ? adjacent_pieces[a].col() + 2 : adjacent_pieces[a].col();
          if (opp_row < b_dim && opp_col < b_dim && !is_free(opp_row, opp_col)) {
            add_all_pieces_in_direction(row, col, opp_row, opp_col, secondary_word);
          }
        }

        rv.push_back(secondary_word); //add all secondary words to total
      }
    }
  }

  //add primary word to total
  if (primary_word.get_word_str() != "") {
    rv.push_back(primary_word);
  }

  return rv;
}

////////////////////////////////////////////////////////////////////////////////
ostream& Scrabble_Board::operator<<(ostream& out) const
////////////////////////////////////////////////////////////////////////////////
{
  const unsigned b_dim = m_board.size();

  //add col headers
  out << "   ";
  for (unsigned i = 0; i < b_dim; ++i) {
    out << ( (i < 10) ? "   " : "  ") << i << "  ";
  }
  out << "\n";

  for (unsigned row = 0; row < b_dim; ++row) {
    //need separators between rows
    out << "   ";
    for (unsigned tmp = 0; tmp < Scrabble_Square::OUTPUT_LEN * b_dim; ++tmp) {
      out << "-";
    }
    out << "\n";

    out << setiosflags(ios::left) << setw(3) << row;
    for (unsigned col = 0; col < b_dim; ++col) {
      out << m_board[row][col];
    }
    out << "|\n";
  }

  out << "   ";
  for (unsigned tmp = 0; tmp < Scrabble_Square::OUTPUT_LEN * b_dim; ++tmp) {
    out << "-";
  }

  return out;
}

////////////////////////////////////////////////////////////////////////////////
istream& Scrabble_Board::operator>>(istream& in)
////////////////////////////////////////////////////////////////////////////////
{
  std::string line;
  const unsigned dim = get_board_dim();

  getline(in, line); // skip headers
  getline(in, line); // skip hashes

  for (unsigned row = 0; row < dim; ++row) {
    std::string curr_cell = "";
    getline(in, line); // read row
    unsigned col = 0;
    for (unsigned i = 4; i < line.size(); ++i) {
      if (line[i] == '|') {
        unsigned blanks = std::count(curr_cell.begin(), curr_cell.end(), ' ');
        if (blanks != curr_cell.size()) {
          std::istringstream iss(curr_cell);
          iss >> m_board[row][col];
        }
        curr_cell = "";
        ++col;
      }
      else {
        curr_cell += line[i];
      }
    }

    getline(in, line); // skip hashes
  }

  return in;
}

////////////////////////////////////////////////////////////////////////////////
bool Scrabble_Board::is_free(unsigned row, unsigned col) const
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(row < m_board.size() && col < m_board.size(), std::string("Received loc: ") +
            obj_to_str(Board_Loc(row,col)) + " which does not fall within the board.");

  return m_board[row][col].is_free();
}

////////////////////////////////////////////////////////////////////////////////
std::vector<Board_Loc> Scrabble_Board::get_adjacent(unsigned row,
                                                    unsigned col,
                                                    bool one_per_dim,
                                                    bool looking_for_free) const
////////////////////////////////////////////////////////////////////////////////
{
  std::vector<Board_Loc> adjacent_squares;
  const unsigned b_dim = m_board.size();
  my_assert(row < b_dim && col < b_dim, std::string("Received loc: ") +
            obj_to_str(Board_Loc(row,col)) + " which does not fall within the board.");
  bool adj_row_found = false, adj_col_found = false;

  //sometimes only want to report one adjacent square per dimension! get_created_words depends on this
  //to avoid slicing bridged words
  if ( (row > 0) && (m_board[row-1][col].is_free() == looking_for_free) ) {
    adjacent_squares.push_back(Board_Loc(row-1, col));
    adj_row_found = true;
  }
  if ( (row < b_dim-1) && (m_board[row+1][col].is_free() == looking_for_free) ) {
    if (!(one_per_dim && adj_row_found)) {
      adjacent_squares.push_back(Board_Loc(row+1, col));
    }
  }


  if ( (col > 0) && (m_board[row][col-1].is_free() == looking_for_free) ) {
    adjacent_squares.push_back(Board_Loc(row, col-1));
    adj_col_found = true;
  }
  if ( (col < b_dim-1) && (m_board[row][col+1].is_free() == looking_for_free) ) {
    if (!(one_per_dim && adj_col_found)) {
      adjacent_squares.push_back(Board_Loc(row, col+1));
    }
  }

  return adjacent_squares;
}

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Board::add_all_pieces_in_direction(unsigned my_row,  unsigned my_col,
                                                 unsigned adj_row, unsigned adj_col,
                                                 Scrabble_Word& word) const
////////////////////////////////////////////////////////////////////////////////
{
  const unsigned b_dim = m_board.size();

  my_assert(my_row < b_dim && my_col < b_dim, std::string("Received my-loc: ") +
            obj_to_str(Board_Loc(my_row,my_col)) + " which does not fall within the board.");

  my_assert(adj_row < b_dim && adj_col < b_dim, std::string("Received adj-loc: ") +
            obj_to_str(Board_Loc(adj_row,adj_col)) + " which does not fall within the board.");
  my_assert(!is_free(adj_row, adj_col), std::string("Asking to add pieces starting with adj-loc: ") +
            obj_to_str(Board_Loc(adj_row,adj_col)) + " which did not have a piece on it.");

  unsigned starting_row = adj_row;
  unsigned starting_col = adj_col;
  int      row_diff     = adj_row - my_row;
  int      col_diff     = adj_col - my_col;

  my_assert(row_diff == 0 || col_diff == 0, std::string("Bad values: row_diff=") + obj_to_str(row_diff) +
            ", col_diff=" + obj_to_str(col_diff) + ". Expected one of them to be zero.");
  my_assert(!(row_diff == 0 && col_diff == 0), std::string("Bad values: row_diff=") + obj_to_str(row_diff) +
            ", col_diff=" + obj_to_str(col_diff) + ". Expected one of them to be non-zero.");
  bool is_horiz = row_diff == 0;

  //follow this direction until it ends, everything we hit is part of the word
  for (unsigned r = starting_row, c = starting_col;
       r < b_dim && c < b_dim && !is_free(r, c);
       r += row_diff, c += col_diff) {
    unsigned piece_id = is_horiz ? c : r;
    if (!word.has_piece(piece_id)) {
      word.add_component(m_board[r][c].get_piece(),
                         NONE, //Note: This piece is already played, bonus is gone
                         piece_id);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Board::place_piece(unsigned row, unsigned col, const Scrabble_Piece* the_piece)
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(row < m_board.size() && col < m_board.size(), std::string("Received loc: ") +
            obj_to_str(Board_Loc(row,col)) + " which does not fall within the board.");

  m_board[row][col].add_piece(the_piece);
}

////////////////////////////////////////////////////////////////////////////////
const Scrabble_Piece* Scrabble_Board::get_piece(unsigned row, unsigned col) const
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(row < m_board.size() && col < m_board.size(), std::string("Received loc: ") +
            obj_to_str(Board_Loc(row,col)) + " which does not fall within the board.");
  my_assert(!is_free(row, col), std::string("Received request to get_piece with loc: ") +
            obj_to_str(Board_Loc(row,col)) + " which does not have a piece on it.");

  return m_board[row][col].get_piece();
}

////////////////////////////////////////////////////////////////////////////////
Scrabble_Square Scrabble_Board::get_square(unsigned row, unsigned col) const
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(row < m_board.size() && col < m_board.size(), std::string("Received loc: ") +
            obj_to_str(Board_Loc(row,col)) + " which does not fall within the board.");

  return m_board[row][col];
}

////////////////////////////////////////////////////////////////////////////////
bool Scrabble_Board::is_unconstrained(unsigned row, unsigned col) const
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(row < m_board.size() && col < m_board.size(), std::string("Received loc: ") +
            obj_to_str(Board_Loc(row,col)) + " which does not fall within the board.");

  //a square is "unconstrained" if it is empty and is not adjacent to any square with a piece on it
  return (is_free(row, col) && !is_adjacent(row, col));
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Scrabble_Board& sb)
////////////////////////////////////////////////////////////////////////////////
{
  return sb.operator<<(out);
}

////////////////////////////////////////////////////////////////////////////////
std::istream& operator>>(std::istream& in, Scrabble_Board& sb)
////////////////////////////////////////////////////////////////////////////////
{
  return sb.operator>>(in);
}
