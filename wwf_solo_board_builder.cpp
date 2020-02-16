#include "scrabble_board.hpp"
#include "scrabble_exception.hpp"
#include "wwf_solo_board_builder.hpp"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
void Wwf_Solo_Board_Builder::build_board(Scrabble_Board* board) const
////////////////////////////////////////////////////////////////////////////////
{
  //Default construct all the necessary squares
  board->m_board.resize(BOARD_DIM);
  for (unsigned row = 0; row < board->m_board.size(); row++) {
    board->m_board[row].resize(BOARD_DIM);
  }

  // Inner diamond
  Bonus bonus;
  for (unsigned row = 9, col = 5; col < 10; --row, ++col) {
    if (row == 5 || col == 5) {
      bonus = DBL_WRD;
    }
    else if (row % 2 == 0) {
      bonus = DBL_LET;
    }
    else if (row == 7) {
      bonus = TRP_LET;
    }
    else {
      my_assert(false, "All tiles in this routine have a bonus");
    }

    set_4way_symm(board, row, col, bonus);
  }

  for (unsigned row = 8, col = 8; col < 11; ++row, ++col) {
    if (row == 8) {
      bonus = DBL_LET;
    }
    else if (row == 9) {
      bonus = DBL_WRD;
    }
    else if (row == 10) {
      bonus = TRP_LET;
    }
    else {
      my_assert(false, "All tiles in this routine have a bonus");
    }

    set_4way_symm(board, row, col, bonus);
  }

  for (unsigned row = 10, col = 8; col < 11; --row, ++col) {
    if (row == 8) {
      bonus = TRP_WRD;
    }
    else if (row == 9) {
      bonus = DBL_WRD;
    }
    else if (row == 10) {
      bonus = TRP_WRD;
    }
    else {
      my_assert(false, "All tiles in this routine have a bonus");
    }

    set_4way_symm(board, row, col, bonus);
  }

  Board_Builder::finish(*board);
}

///////////////////////////////////////////////////////////////////////////////
void Wwf_Solo_Board_Builder::set_4way_symm(Scrabble_Board* board, unsigned hi_row, unsigned hi_col, int arg_bonus) const
///////////////////////////////////////////////////////////////////////////////
{
  Bonus bonus = static_cast<Bonus>(arg_bonus);
  int mid = board->get_board_dim() / 2;

  int diff_from_mid_row = hi_row - mid;
  int diff_from_mid_col = hi_col - mid;

  for (int i = -1; i < 1; ++i) {
    for (int j = -1; j < 1; ++j) {
      unsigned real_row = hi_row + (2 * i * diff_from_mid_row);
      unsigned real_col = hi_col + (2 * j * diff_from_mid_col);
      Scrabble_Square& square = board->m_board[real_row][real_col];
      if (square.get_bonus() == NONE) {
        square.set_bonus(bonus);
      }
      else {
        my_assert(square.get_bonus() == bonus, "Bonus mismatch");
      }
    }
  }
}
