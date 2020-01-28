"""
Holds a simple image of the game state, receives events updating
the state (from C++) and presents the GUI.
"""

import ctypes
import tkinter as tk

class Piece(object):
    def __init__(self, letter, points):
        self._letter = letter
        self._points = points

class PyScrabbleGame(tk.Frame):

    def __init__(self, dim):
        self._board = [[None] * dim] * dim
        self._root = tk.Tk()

        tk.Frame.__init__(self, self._root)

    def play_event(self, rows, cols, letters):
        for row, col, letter in zip(rows, cols, letters):
            self._board[row][col] = Piece(letter, 0)

GAME = None

def test_func(play_size, play_rows, play_cols, play_letters):
    global GAME
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
