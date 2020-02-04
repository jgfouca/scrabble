"""
Holds a simple image of the game state, receives events updating
the state (from C++) and presents the GUI.
"""

import ctypes, sys, threading, string
import tkinter as tk
from functools import partial

from utils import cstr_to_letters, cstr_to_ints, expect, to_cstr, tyn

# Globals

GAME = None

# Events, must match GUI_Event enum from C++
TILES, PLAY, BOARD_INIT, CHECK_PLAY, CONFIRM_PLAY = range(5)

# Bonusses, must match Bonus enum from C++
BNONE, DBL_LET, TRP_LET, DBL_WRD, TRP_WRD = range(5)

NUM_PLAYER_TILES = 7

BUTTON_SIZE = 80

TILE_COLOR = "bisque"
SQUARE_COLOR = "gray"

###############################################################################
class ScrabbleButton(tk.Button):
###############################################################################

    def __init__(self, root, action, xloc, yloc):
        tk.Button.__init__(self, root, command=action)
        self._xloc = xloc
        self._yloc = yloc
        self._tile = None
        self._prev_text = None
        self._prev_color = None
        self["font"] = ("TkDefaultFont", 20)

    def place(self):
        tk.Button.place(self, x=(BUTTON_SIZE*self._xloc), y=(BUTTON_SIZE*self._yloc),
                        height=BUTTON_SIZE, width=BUTTON_SIZE)

    def set_text(self, text):
        self._prev_text = self["text"]
        self["text"] = text

    def set_color(self, color):
        self._prev_color = self["bg"]
        self["bg"] = color

    def has_tile(self):
        return self._tile is not None

    def revert(self):
        self.set_text(self._prev_text)
        self.set_color(self._prev_color)

    def getx(self): return self._xloc

    def gety(self): return self._yloc

    def __lt__(self, rhs):
        if (self._xloc != rhs._xloc):
            return self._xloc < rhs._xloc
        else:
            return self._yloc < rhs._yloc

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

    def __init__(self, root, action, xloc, yloc):
        ScrabbleButton.__init__(self, root, action, xloc, yloc)
        self.set_color(SQUARE_COLOR)
        self.place()
        self._fixed = False

    def play_tile(self, tile):
        self.set_text(tile["text"])
        tile.set_text("")
        self.set_color(tile["bg"])
        tile.set_color(self.master["bg"])

    def finalize(self):
        if self._tile:
            self._tile  = None
            self._fixed = True

    def is_fixed(self): return self._fixed

###############################################################################
class PlayerTile(ScrabbleButton):
###############################################################################

    def __init__(self, root, action, xloc, yloc):
        ScrabbleButton.__init__(self, root, action, xloc, yloc)
        self.set_color(TILE_COLOR)
        self.place()

###############################################################################
class ActionButton(ScrabbleButton):
###############################################################################

    def __init__(self, root, action, name, xloc, yloc):
        ScrabbleButton.__init__(self, root, action, xloc, yloc)
        self["text"] = name
        self.place()

###############################################################################
class PyScrabbleGame(tk.Frame):
###############################################################################

    def __init__(self, dim):
        self._root = tk.Tk()
        self._root.geometry("{}x{}".format(BUTTON_SIZE * (dim+3), BUTTON_SIZE * (dim+2)))

        tk.Frame.__init__(self, self._root)

        # changing the title of our root widget
        self._root.title("SCRABBLE/WWF")

        # allowing the widget to take the full space of the root window
        self.pack(fill=tk.BOTH, expand=1)

        # creating board_buttons
        self._board = [[None for i in range(dim)] for j in range(dim)]
        for i in range(dim):
            for j in range(dim):
                self._board[i][j] = BoardTile(self, partial(self.board_click_event, i, j), i, j)

        # hold tiles
        self._tiles = [None] * NUM_PLAYER_TILES
        offset = (dim - NUM_PLAYER_TILES) / 2
        for i in range(NUM_PLAYER_TILES):
            self._tiles[i] = PlayerTile(self, partial(self.tile_click_event, i), offset+i, dim)

        self._active_tile = None

        self._tile_label = ScrabbleLabel(self, "Tiles:")
        self._tile_label.place(offset-1, dim)

        self._playbut = ActionButton(self, partial(self.make_play), "PLAY", int(dim / 2), dim + 1)
        self._playbut["bg"] = "gold"

        self._godbut  = ActionButton(self, partial(self.toggle_god), "GOD\nMODE", int(dim / 2) + 3, dim + 1)
        self._godbut["bg"] = "black"
        self._godbut["fg"] = "red"
        self._god_mode = False

        self._play = []
        self._play_cmd = None

        self._lock = threading.Lock()

        num_letters = len(string.ascii_uppercase)
        self._god_letters = [None] * num_letters
        for idx, letter in enumerate(string.ascii_uppercase):
            self._god_letters[idx] = PlayerTile(self, partial(self.god_letter_click_event, letter),
                                                (dim+1) + int(idx / (num_letters / 2)), idx % (num_letters / 2))
            self._god_letters[idx].set_text(letter)


    def error_popup(self, msg):
        #TODO
        print("ERROR: {}".format(msg))

    #
    # C++ events
    #

    def play_event(self, rows, cols, letters):
        for row, col, letter in zip(rows, cols, letters):
            self._board[col][row].set_text(letter)
            self._board[col][row].set_color(TILE_COLOR)

    def tiles_event(self, num, letters):
        for i in range(NUM_PLAYER_TILES):
            if i < num:
                self._tiles[i].set_text(letters[i])
                self._tiles[i].set_color(TILE_COLOR)
            else:
                self._tiles[i].set_text("")
                self._tiles[i].set_color(self["bg"])

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
                self._board[col][row].set_text(change_text)
                self._board[col][row].set_color(change_color)

    def check_play_event(self, size_buffer, play_buffer):
        with self._lock:
            if self._play_cmd:
                size_buffer[0] = len(self._play_cmd)
                play_bytes = to_cstr(self._play_cmd)
                for i in range(len(self._play_cmd)):
                    play_buffer[i] = play_bytes[i]
                return True
            else:
                return False

    def confirm_play_event(self, success, err_msg):
        with self._lock:
            if success == 0:
                # if play accepted, finalize
                for board in self._play:
                    board.finalize()

                self._play = []
                self._play_cmd = None

            else:
                self.error_popup(err_msg)
    #
    # Button click events
    #

    def board_click_event(self, i, j):
        print("board [{}][{}] was clicked".format(i, j))
        board = self._board[i][j]
        if self._active_tile:
            if (not board.has_tile()):
                board.play_tile(self._active_tile)
                self._active_tile = None
                self._play.append(board)
        elif board.has_tile():
            board.revert()
            self._board.tile().revert()

    def tile_click_event(self, i):
        print("tile [{}] was clicked".format(i))
        if self._active_tile:
            self._active_tile.revert()

        self._active_tile = self._tiles[i]

    def toggle_god(self):
        if self._active_tile:
            self._active_tile.revert()
            self._active_tile = None

        self._god_mode = not self._god_mode
        print("god button clicked, god mode now {}".format(self._god_mode))
        godbg, godfg = self._godbut["bg"], self._godbut["fg"]
        self._godbut["bg"] = godfg
        self._godbut["fg"] = godbg

    def god_letter_click_event(self, letter):
        if self._god_mode and self._active_tile:
            self._active_tile["text"] = letter

        self._active_tile = None

    def make_play(self):
        if self._active_tile:
            self._active_tile.revert()
            self._active_tile = None

        with self._lock:
            self._make_play_impl()

    def _make_play_impl(self):
        print("clicked make play")
        xs, ys, letters = [], [], []
        if self._play:
            self._play.sort()

            for board in self._play:
                xs.append(board.getx())
                ys.append(board.gety())
                letters.append(board["text"])

            is_horiz = "True"
            origx, origy, playword = xs[0], ys[0], letters[0]
            lastx, lasty = origx, origy
            if len(self._play) > 1:
                for x, y, letter in zip(xs[1:], ys[1:], letters[1:]):
                    is_horiz = x == origx

                    # Check straight line
                    if is_horiz and y != origy:
                        self.error_popup("Must play along straight line")
                        return
                    elif not is_horiz and x != origx:
                        self.error_popup("Must play along straight line")
                        return

                    lastp = lastx if is_horiz else lasty
                    curp  = x     if is_horiz else y
                    playword += "_"*(curp-lastp-1)

                    playword += letter
                    lastx, lasty = x, y

            # Send play to cxx, turn it into string
            self._play_cmd = "play {} {} {} {} {}".format(origy, origx, playword, tyn(is_horiz), tyn(self._god_mode))

            print("PLAY IS '{}'".format(self._play_cmd))

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
def check_play_callback(size_buffer, play_buffer):
###############################################################################
    return GAME.check_play_event(size_buffer, play_buffer)

###############################################################################
def confirm_play_callback(success, error_msg_bytes):
###############################################################################
    error_msg_bytes = error_msg_bytes[0:success]
    GAME.confirm_play_event(success, error_msg_bytes.decode("utf-8"))

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
        elif event == CHECK_PLAY:
            success = check_play_callback(play_rows, play_letters)
        elif event == CONFIRM_PLAY:
            confirm_play_callback(play_size, play_letters)
        else:
            expect(False, "Unknown event {}".format(event))
    except BaseException as e:
        success = False
        print(e)

    return ctypes.c_bool(success)
