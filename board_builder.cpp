#include "board_builder.hpp"
#include "scrabble_board.hpp"

////////////////////////////////////////////////////////////////////////////////
void Board_Builder::finish(Scrabble_Board& board) const
////////////////////////////////////////////////////////////////////////////////
{
  const unsigned b_dim = board.get_board_dim();

  for (unsigned row = 0; row < b_dim; ++row) {
    for (unsigned col = 0; col < b_dim; ++col) {
      board.m_board[row][col].set_parent(m_parent);
    }
  }

}
