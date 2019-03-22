#include "scrabble_board.h"
#include "scrabble_exception.h"
#include "wwf_board_builder.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
const Wwf_Board_Builder& Wwf_Board_Builder::instance()
////////////////////////////////////////////////////////////////////////////////
{
  static Wwf_Board_Builder glob_instance;
  return glob_instance;
}

////////////////////////////////////////////////////////////////////////////////
void Wwf_Board_Builder::build_board(Scrabble_Board* board) const
////////////////////////////////////////////////////////////////////////////////
{
  //Default construct all the necessary squares
  board->m_board.resize(BOARD_DIM);
  for (unsigned row = 0; row < board->m_board.size(); row++) {
    board->m_board[row].resize(BOARD_DIM);
  }

  Bonus bonus;
  for (unsigned row = 10, col = 5; col < 10; --row, ++col) {
    std::cout << "Base row, col " << row << ", " << col << std::endl;
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

    int diff_from_mid_row = row - 5;
    int diff_from_mid_col = col - 5;

    for (int i = -1; i < 1; ++i) {
      for (int j = -1; j < 1; ++j) {
        unsigned real_row = row + (i * diff_from_mid_row);
        unsigned real_col = col + (j * diff_from_mid_col);
        cout << "Trying to set " << real_row << ", " << real_col << std::endl;
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
}
