#include "scrabble_board.hpp"
#include "scrabble_exception.hpp"
#include "wwf_board_builder.hpp"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
void Wwf_Board_Builder::build_board(Scrabble_Board* board) const
////////////////////////////////////////////////////////////////////////////////
{
  //Default construct all the necessary squares
  board->m_board.resize(BOARD_DIM);
  for (unsigned row = 0; row < board->m_board.size(); row++) {
    board->m_board[row].resize(BOARD_DIM);
  }

  // Inner diamond
  Bonus bonus;
  const int mid_idx = BOARD_DIM / 2;
  int diamond_radius = 4;
  for (int row = mid_idx + diamond_radius, col = mid_idx; col <= mid_idx + diamond_radius; --row, ++col) {
    if (row == mid_idx || col == mid_idx) {
      bonus = DBL_WRD;
    }
    else if (row % 2 == 0) {
      bonus = DBL_LET;
    }
    else if (row == mid_idx + 2) {
      bonus = TRP_LET;
    }
    else {
      my_static_assert(false, "All tiles in this routine have a bonus");
    }

    set_4way_symm(board, row, col, bonus);
  }

  diamond_radius = 8;
  for (int row = mid_idx + diamond_radius, col = mid_idx; col <= mid_idx + diamond_radius; --row, ++col) {
    if (row == BOARD_DIM-1 || col == BOARD_DIM-1 || row==col) {
      bonus = TRP_LET;
    }
    else if (row % 2 == 0) {
      bonus = DBL_LET;
    }
    else if (row < 0 || row >= BOARD_DIM || col < 0 || col >= BOARD_DIM) {
      continue;
    }
    else if (row == 9 || row == 13) {
      bonus = DBL_WRD;
    }
    else {
      my_static_assert(false, "All tiles in this routine have a bonus");
    }

    set_4way_symm(board, row, col, bonus);
  }

  diamond_radius = 11;
  for (int row = mid_idx + diamond_radius, col = mid_idx; col <= mid_idx + diamond_radius; --row, ++col) {
    if (row == BOARD_DIM-1 || col == BOARD_DIM-1) {
      bonus = TRP_WRD;
    }
    else if (row < 0 || row >= BOARD_DIM || col < 0 || col >= BOARD_DIM) {
      continue;
    }
    else {
      bonus = DBL_LET;
    }

    set_4way_symm(board, row, col, bonus);
  }

  Board_Builder::finish(*board);
}

///////////////////////////////////////////////////////////////////////////////
void Wwf_Board_Builder::set_4way_symm(Scrabble_Board* board, unsigned hi_row, unsigned hi_col, int arg_bonus) const
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
