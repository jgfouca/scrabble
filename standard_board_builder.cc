#include "scrabble_board.h"
#include "scrabble_exception.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
const Standard_Board_Builder& Standard_Board_Builder::instance()
////////////////////////////////////////////////////////////////////////////////
{
  static Standard_Board_Builder glob_instance;
  return glob_instance;
}

////////////////////////////////////////////////////////////////////////////////
void Standard_Board_Builder::build_board(Scrabble_Board* board) const
////////////////////////////////////////////////////////////////////////////////
{
  //Default construct all the necessary squares
  board->m_board.resize(BOARD_DIM);
  for (unsigned row = 0; row < board->m_board.size(); row++) {
    board->m_board[row].resize(BOARD_DIM);
  }

  //main diag bonuses
  for (unsigned row = 0, col = 0; row < BOARD_DIM; row++, col++) {
    Bonus bonus;
    if (row == 0 || row == BOARD_DIM - 1) {
      bonus = TRP_WRD;
    }
    else if ( (row >= 1 && row <= 4) || (row >= 10 && row <= 13) ) {
      bonus = DBL_WRD;
    }
    else if ( row == 5 || row == 9 ) {
      bonus = TRP_LET;
    }
    else if ( row == 6 || row == 8 ) {
      bonus = DBL_LET;
    }
    else {
      //no bonus on this square
      my_assert(row == 7, "Only 7,7 should be without any bonus");
      continue;
    }

    board->m_board[row][col].set_bonus(bonus);
    board->m_board[BOARD_DIM - 1 - row][col].set_bonus(bonus);
  }

  //smaller diag bonuses, this is a 5 square sequence that occurs in 4 places on the board
  //loop over the 5 square seq
  for (unsigned itr = 0; itr < 5; itr++) {
    unsigned row = 5, col = 1;
    Bonus bonus = TRP_LET;
    if (itr == 1) {
      row += 1;
      col += 1;
      bonus = DBL_LET;
    }
    else if (itr == 2) {
      row += 2;
      col += 2;
      bonus = DBL_LET;
    }
    else if (itr == 3) {
      row += 3;
      col += 1;
      bonus = DBL_LET;
    }
    else if (itr == 4) {
      row += 4;
    }

    //add to the four places in the board
    board->m_board[row][col].set_bonus(bonus);
    board->m_board[col][row].set_bonus(bonus);
    board->m_board[row][BOARD_DIM - 1 - col].set_bonus(bonus);
    board->m_board[BOARD_DIM - 1 - col][row].set_bonus(bonus);
  }

  //non-diag edge bonuses, this is a 3 square sequence that occurs in 4 places on the board
  for (unsigned itr = 0; itr < 3; itr++) {
    unsigned row = 3, col = 0;
    Bonus bonus = DBL_LET;
    if (itr == 1) {
      row += 4;
      bonus = TRP_WRD;
    }
    else if (itr == 2) {
      row += 8;
    }

    //add to the four places in the board
    board->m_board[row][col].set_bonus(bonus);
    board->m_board[col][row].set_bonus(bonus);
    board->m_board[row][BOARD_DIM - 1 - col].set_bonus(bonus);
    board->m_board[BOARD_DIM - 1 - col][row].set_bonus(bonus);
  }

  //all bonuses should now have been placed
}
