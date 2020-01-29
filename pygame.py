"""
Holds a simple image of the game state, receives events updating
the state (from C++) and presents the GUI.
"""

import ctypes
import tkinter as tk
from functools import partial

###############################################################################
class Piece(object):
###############################################################################
    def __init__(self, letter, points):
        self._letter = letter
        self._points = points

###############################################################################
class PyScrabbleGame(tk.Frame):
###############################################################################

    BUTTON_SIZE = 80

    def __init__(self, dim):
        self._board = [[None for i in range(dim)] for j in range(dim)]
        self._root = tk.Tk()
        self._root.geometry("{}x{}".format(self.BUTTON_SIZE * dim, self.BUTTON_SIZE * dim))

        tk.Frame.__init__(self, self._root)

        # changing the title of our root widget
        self._root.title("SCRABBLE/WWF")

        # allowing the widget to take the full space of the root window
        self.pack(fill=tk.BOTH, expand=1)

        # creating buttons
        self._buttons = [[None for i in range(dim)] for j in range(dim)]
        for i in range(dim):
            for j in range(dim):
                self._buttons[i][j] = tk.Button(self, command=partial(self.tile_click_event, i, j))
                # placing the button on my window
                self._buttons[i][j].place(x=(self.BUTTON_SIZE*j), y=(self.BUTTON_SIZE*i), height=self.BUTTON_SIZE, width=self.BUTTON_SIZE)

    def play_event(self, rows, cols, letters):
        for row, col, letter in zip(rows, cols, letters):
            self._board[row][col] = Piece(letter, 0)
            print("CHANGING {} {} ({}) to {}".format(row, col, self._buttons[row][col]["text"], letter))
            self._buttons[row][col]["text"] = letter

    def tile_click_event(self, i, j):
        print("[{}][{}] was clicked".format(i, j))

GAME = None

###############################################################################
def test_func(play_size, play_rows, play_cols, play_letters):
###############################################################################
    letters = []
    for i in range(play_size):
        print(play_rows[i], play_cols[i], play_letters[i])
        letters.append(play_letters[i].decode("utf-8"))

    success = True
    try:
        GAME.play_event(play_rows, play_cols, letters)
    except BaseException as e:
        success = False
        print(e)

    return ctypes.c_bool(success)
