"""
Holds a simple image of the game state, receives events updating
the state (from C++) and presents the GUI.
"""

import ctypes
import tkinter as tk
from functools import partial

from utils import cstr_to_letters, cstr_to_ints

# Globals

GAME = None

# Events, must match GUI_Event enum from C++
TILES, PLAY, BOARD_INIT = range(3)

# Bonusses, must match Bonus enum from C++
BNONE, DBL_LET, TRP_LET, DBL_WRD, TRP_WRD = range(5)

NUM_PLAYER_TILES = 7

BUTTON_SIZE = 80

###############################################################################
class ScrabbleButton(tk.Button):
###############################################################################

    def __init__(self, root, action):
        tk.Button.__init__(self, root, command=action)

    def place(self, xslot, yslot):
        tk.Button.place(self, x=(BUTTON_SIZE*xslot), y=(BUTTON_SIZE*yslot),
                        height=BUTTON_SIZE, width=BUTTON_SIZE)

###############################################################################
class ScrabbleLabel(tk.Label):
###############################################################################

    def __init__(self, root, text):
        tk.Label.__init__(self, root, text=text, font=("TkDefaultFont", 20))

    def place(self, xslot, yslot):
        tk.Label.place(self, x=(BUTTON_SIZE*xslot),
                       y=(BUTTON_SIZE*yslot + BUTTON_SIZE/3)) # want text to appear in middle of tile

###############################################################################
class BoardTile(ScrabbleButton):
###############################################################################

    def __init__(self, root, action):
        ScrabbleButton.__init__(self, root, action)

###############################################################################
class PlayerTile(ScrabbleButton):
###############################################################################

    def __init__(self, root, action):
        ScrabbleButton.__init__(self, root, action)

###############################################################################
class PyScrabbleGame(tk.Frame):
###############################################################################

    def __init__(self, dim):
        self._root = tk.Tk()
        self._root.geometry("{}x{}".format(BUTTON_SIZE * dim, BUTTON_SIZE * (dim+1)))

        tk.Frame.__init__(self, self._root)

        # changing the title of our root widget
        self._root.title("SCRABBLE/WWF")

        # allowing the widget to take the full space of the root window
        self.pack(fill=tk.BOTH, expand=1)

        # creating board_buttons
        self._board = [[None for i in range(dim)] for j in range(dim)]
        for i in range(dim):
            for j in range(dim):
                self._board[i][j] = BoardTile(self, partial(self.board_click_event, i, j))
                # placing the button on my window
                self._board[i][j].place(j, i)

        # hold tiles
        self._tiles = [None] * NUM_PLAYER_TILES
        offset = (dim - NUM_PLAYER_TILES) / 2
        for i in range(NUM_PLAYER_TILES):
            self._tiles[i] = PlayerTile(self, partial(self.tile_click_event, i))
            # placing the button on my window
            self._tiles[i].place(offset+i, dim)

        self._active_tile = None

        self._tile_label = ScrabbleLabel(self, "Tiles:")
        self._tile_label.place(offset-1, dim)

    #
    # C++ events
    #

    def play_event(self, rows, cols, letters):
        for row, col, letter in zip(rows, cols, letters):
            self._board[row][col]["text"] = letter

    def tiles_event(self, num, letters):
        for i in range(NUM_PLAYER_TILES):
            if i < num:
                self._tiles[i]["text"] = letters[i]
            else:
                self._tiles[i]["text"] = ""

    def board_init_event(self, rows, cols, bonusses):
        for row, col, bonus in zip(rows, cols, bonusses):
            change_color, change_text = None, None
            if bonus == BNONE:
                pass
            elif bonus == DBL_LET:
                change_color, change_text = "blue", "DL"
            elif bonus == TRP_LET:
                change_color, change_text = "green", "TL"
            elif bonus == DBL_WRD:
                change_color, change_text = "red", "DW"
            elif bonus == TRP_WRD:
                change_color, change_text = "orange", "TW"
            else:
                expect(False, "Unknown bonus {}".format(bonus))

            if change_color is not None:
                self._board[row][col]["bg"] = change_color
                self._board[row][col]["text"] = change_text

    #
    # Button click events
    #

    def board_click_event(self, i, j):
        print("board [{}][{}] was clicked".format(i, j))
        if self._active_tile:
            self._board[i][j]["text"] = self._active_tile["text"]
            self._active_tile = None

    def tile_click_event(self, i):
        print("tile [{}] was clicked".format(i))
        self._active_tile = self._tiles[i]

###############################################################################
def play_callback(play_size, play_rows, play_cols, play_letters):
###############################################################################
    GAME.play_event(play_rows, play_cols, cstr_to_letters(play_letters, play_size))

###############################################################################
def tiles_callback(num_tiles, tile_letters):
###############################################################################
    GAME.tiles_event(num_tiles, cstr_to_letters(tile_letters, num_tiles))

###############################################################################
def board_init_callback(num_bonus, bonus_rows, bonus_cols, bonus_types):
###############################################################################
    GAME.board_init_event(bonus_rows, bonus_cols, cstr_to_ints(bonus_types, num_bonus))

###############################################################################
def callback_func(event, play_size, play_rows, play_cols, play_letters):
###############################################################################
    success = True
    try:
        if event == PLAY:
            play_callback(play_size, play_rows, play_cols, play_letters)
        elif event == TILES:
            tiles_callback(play_size, play_letters)
        elif event == BOARD_INIT:
            board_init_callback(play_size, play_rows, play_cols, play_letters)
        else:
            expect(False, "Unknown event {}".format(event))
    except BaseException as e:
        success = False
        print(e)

    return ctypes.c_bool(success)
