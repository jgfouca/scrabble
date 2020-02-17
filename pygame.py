"""
Holds a simple image of the game state, receives events updating
the state (from C++) and presents the GUI.
"""

import ctypes, sys, threading, string, time
import tkinter as tk
from functools import partial
import platform

if platform.system() == "Darwin":
    from tkmacosx import Button
else:
    from tk import Button

from utils import cstr_to_letters, cstr_to_ints, expect, to_cstr, tyn

# Globals

GAME = None

# Events, must match GUI_Event enum from C++
TILES, PLAY, BOARD_INIT, CHECK_PLAY, CONFIRM_PLAY, CHECK_HINT, GIVE_HINT, CHECK_SAVE = range(8)

# Bonusses, must match Bonus enum from C++
BNONE, DBL_LET, TRP_LET, DBL_WRD, TRP_WRD = range(5)

NUM_PLAYER_TILES = 7

BUTTON_SIZE = 80

TILE_COLOR = "bisque"
SQUARE_COLOR = "gray"
WILD_COLOR = "pink"
HINT_COLOR = "spring green"
TRAYBUT_COLOR = "SteelBlue3"

###############################################################################
class ScrabbleButton(Button):
###############################################################################

    def __init__(self, root, action, xloc, yloc):
        super().__init__(root, command=action, text="ignore") # Ignore text working around bug in tkmacosx
        self["text"] = ""
        self._xloc = xloc
        self._yloc = yloc
        self._prev_text = None
        self._prev_color = None
        self["font"] = ("TkDefaultFont", 20)
        self._fixed = False

    def place(self):
        super().place(x=(BUTTON_SIZE*self._xloc), y=(BUTTON_SIZE*self._yloc),
                      height=BUTTON_SIZE, width=BUTTON_SIZE)

    def set_text(self, text):
        expect(not self._fixed, "Tile is fixed")

        self._prev_text = self["text"]
        self["text"] = text

    def set_color(self, color):
        expect(not self._fixed, "Tile is fixed")

        self._prev_color = self["bg"]
        self["bg"] = color

    def revert(self):
        expect(not self._fixed, "Tile is fixed")

        self.set_text(self._prev_text)
        self.set_color(self._prev_color)

    def swap(self, other):
        expect(not self._fixed, "Tile is fixed")

        tmpt = self["text"]
        tmpc = self["bg"]
        self["text"] = other["text"]
        self["bg"]   = other["bg"]
        other["text"] = tmpt
        other["bg"]   = tmpc

    def invert(self):
        expect(not self._fixed, "Tile is fixed")

        bg, fg = self["bg"], self["fg"]
        self["bg"] = fg
        self["fg"] = bg

    def getx(self): return self._xloc

    def gety(self): return self._yloc

    def is_fixed(self): return self._fixed

    def __lt__(self, rhs):
        if (self._xloc != rhs._xloc):
            return self._xloc < rhs._xloc
        else:
            return self._yloc < rhs._yloc

###############################################################################
class ScrabbleLabel(tk.Label):
###############################################################################

    def __init__(self, root, text):
        super().__init__(root, text=text, font=("TkDefaultFont", 20))

    def place(self, xslot, yslot):
        super().place(x=(BUTTON_SIZE*xslot),
                      y=(BUTTON_SIZE*yslot + BUTTON_SIZE/3)) # want text to appear in middle of tile

###############################################################################
class BoardTile(ScrabbleButton):
###############################################################################

    def __init__(self, root, action, xloc, yloc):
        super().__init__(root, action, xloc, yloc)
        self.set_color(SQUARE_COLOR)
        self.place()
        self._tile = None

    def play_tile(self, tile):
        self.set_text(tile["text"])
        self.set_color(tile["bg"])
        tile.set_text("")
        self._tile = tile

    def pop_tile(self):
        rv = self._tile
        self._tile = None
        return rv

    def has_tile(self):
        return self._tile is not None

    def tile(self): return self._tile

    def finalize(self):
        if self._tile:
            self._tile  = None
            ScrabbleButton._fixed = True

###############################################################################
class PlayerTile(ScrabbleButton):
###############################################################################

    def __init__(self, root, action, xloc, yloc):
        super().__init__(root, action, xloc, yloc)
        self.set_color(TILE_COLOR)
        self.place()

    def set_text(self, text):
        super().set_text(text)
        if text == "-":
            self.set_color(WILD_COLOR)
        elif text == "":
            self.set_color(self.master["bg"])
        else:
            self.set_color(TILE_COLOR)

    def revert(self):
        expect(not self._fixed, "Tile is fixed")

        self.set_text(self._prev_text)

###############################################################################
class ActionButton(ScrabbleButton):
###############################################################################

    def __init__(self, root, action, name, xloc, yloc):
        super().__init__(root, action, xloc, yloc)
        self["text"] = name
        self.place()

###############################################################################
class PyScrabbleGame(tk.Frame):
###############################################################################

    def __init__(self, dim):
        self._root = tk.Tk()
        self._root.geometry("{}x{}".format(BUTTON_SIZE * dim, BUTTON_SIZE * (dim+2)))

        super().__init__(self._root)

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

        self._traybut = ActionButton(self, partial(self.tray_click_event), "EDIT\nTRAY", int(dim / 2) + 1, dim + 1)
        self._traybut["bg"] = "black"
        self._traybut["fg"] = TRAYBUT_COLOR
        self._traybut_idx = -1  # -1 -> not active

        self._hintbut = ActionButton(self, partial(self.request_hint), "HINT", int(dim / 2) + 2, dim + 1)
        self._hintbut["bg"] = HINT_COLOR
        self._wants_hint = False

        self._savebut = ActionButton(self, partial(self.save_event), "SAVE", int(dim / 2) - 1, dim + 1)
        self._savebut["bg"] = "purple"
        self._save_request = None

        self._root.bind("<KeyPress>", self.key_press_event)

    def error_popup(self, msg):
        print("ERROR: {}".format(msg))
        popup = tk.Tk()
        popup.wm_title("Error")
        label = tk.Label(popup, text=msg)
        label.pack(side="top", fill="x", pady=10)
        B1 = tk.Button(popup, text="Okay", command=popup.destroy)
        B1.pack()
        popup.mainloop()

    def get_raw_tile_info(self, inc_board=False):
        result = ""
        play_idx = 0
        for tile in self._tiles:
            if inc_board and tile["text"] == "" and play_idx < len(self._play):
                result += "-" if self._play[play_idx]["bg"] == WILD_COLOR else self._play[play_idx]["text"]
                play_idx += 1
            if tile["text"] != "":
                result += tile["text"]

        return result

    #
    # C++ events
    #

    def play_event(self, rows, cols, letters):
        print("play_event")
        with self._lock:
            for row, col, letter in zip(rows, cols, letters):
                self._board[col][row].set_text(letter)
                self._board[col][row].set_color(TILE_COLOR)

    def tiles_event(self, num, letters):
        with self._lock:
            for i in range(NUM_PLAYER_TILES):
                self._tiles[i].set_text(letters[i] if i < num else "")

    def board_init_event(self, rows, cols, bonusses):
        with self._lock:
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
                tiles = self.get_raw_tile_info(inc_board=True)
                size_buffer[0] = len(self._play_cmd)
                size_buffer[1] = len(tiles)
                play_bytes = to_cstr(self._play_cmd + tiles)
                for idx, b in enumerate(play_bytes):
                    play_buffer[idx] = b

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
                self._play_cmd = None

    def check_hint_event(self, size_buffer, tray_buffer):
        with self._lock:
            if self._wants_hint:
                tiles = self.get_raw_tile_info()
                size_buffer[0] = len(tiles)
                tile_bytes = to_cstr(tiles)
                for idx, b in enumerate(tile_bytes):
                    tray_buffer[idx] = b

            return self._wants_hint

    def check_save_event(self, size_buffer, filename_buffer):
        with self._lock:
            if self._save_request:
                size_buffer[0] = len(self._save_request)
                filename_bytes = to_cstr(self._save_request)
                for idx, b in enumerate(filename_bytes):
                    filename_buffer[idx] = b

                self._save_request = None
                return True

            return False

    def hint_event(self, rows, cols, letters):
        with self._lock:
            for row, col, letter in zip(rows, cols, letters):
                self._board[col][row].set_text(letter)
                self._board[col][row].set_color(HINT_COLOR)

            time.sleep(3)

            for row, col, letter in zip(rows, cols, letters):
                self._board[col][row].revert()

            self._wants_hint = False

    #
    # Button click events
    #

    def board_click_event(self, i, j):
        with self._lock:
            print("board [{}][{}] was clicked".format(i, j))
            board = self._board[i][j]
            if self._active_tile:
                if (not board.has_tile() and not board.is_fixed()):
                    board.play_tile(self._active_tile)
                    self._play.append(board)
                    self._active_tile = None

            elif board.has_tile():
                tile = board.pop_tile()
                board.revert()
                tile.revert()
                self._play.remove(board)

    def tile_click_event(self, i):
        if not self._lock.locked():
            with self._lock:
                print("tile [{}] was clicked".format(i))
                if self._active_tile:
                    self._active_tile.swap(self._tiles[i])
                    self._active_tile = None
                else:
                    self._active_tile = self._tiles[i]

    def toggle_god(self):
        with self._lock:
            if self._active_tile:
                self._active_tile = None

            self._god_mode = not self._god_mode
            print("god button clicked, god mode now {}".format(self._god_mode))
            self._godbut.invert()

    def request_hint(self):
        with self._lock:
            if self._active_tile:
                self._active_tile = None

            if self._god_mode:
                if self._play:
                    self.error_popup("Cannot request hint in middle of play")
                else:
                    self._wants_hint = True

    def tray_click_event(self):
        if self._active_tile:
            self._active_tile = None

        if self._god_mode:
            if self._traybut_idx == -1:
                self._traybut_idx = 0
            else:
                self._traybut_idx = -1

            self._traybut.invert()

    def make_play(self):
        with self._lock:
            if self._active_tile:
                self._active_tile = None

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
                print(xs[-1], ys[-1], letters[-1])

            is_horiz = "True"
            origx, origy, playword = xs[0], ys[0], letters[0]
            lastx, lasty = origx, origy
            if len(self._play) > 1:
                for x, y, letter in zip(xs[1:], ys[1:], letters[1:]):
                    is_horiz = y == origy

                    # Check straight line
                    if not is_horiz and x != origx:
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

    def save_event(self):
        popup = tk.Tk()
        popup.wm_title("Save")
        label = tk.Label(popup, text="Enter filename:")
        label.grid(row=0, column=0)
        e1 = tk.Entry(popup)
        e1.grid(row=0, column=1)
        B1 = tk.Button(popup, text="Okay", command=lambda:self.finish_save_event(popup, e1))
        B1.grid(row=1, column=1)
        popup.mainloop()

    def finish_save_event(self, popup, entry):
        self._save_request = entry.get()
        print("SAVE request for file: {}".format(self._save_request))
        popup.destroy()

    #
    # KeyPress events
    #

    def key_press_event(self, key):
        keyval = key.char.upper()
        print("Pressed {}".format(keyval))

        if self._traybut_idx != -1:
            if keyval in "{}-".format(string.ascii_uppercase):
                tile = self._tiles[self._traybut_idx]
                tile.set_text(keyval)
                self._traybut_idx += 1
                if self._traybut_idx == len(self._tiles):
                    self.tray_click_event()

        else:
            for board in self._play:
                if board["text"] == "-":
                    board["text"] = keyval
                    break

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
def check_hint_callback(size_buffer, tray_buffer):
###############################################################################
    return GAME.check_hint_event(size_buffer, tray_buffer)

###############################################################################
def check_save_callback(size_buffer, filename_buffer):
###############################################################################
    return GAME.check_save_event(size_buffer, filename_buffer)

###############################################################################
def confirm_play_callback(success, error_msg_bytes):
###############################################################################
    error_msg_bytes = error_msg_bytes[0:success]
    GAME.confirm_play_event(success, error_msg_bytes.decode("utf-8"))

###############################################################################
def give_hint_callback(play_size, play_rows, play_cols, play_letters):
###############################################################################
    GAME.hint_event(play_rows, play_cols, cstr_to_letters(play_letters, play_size))

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
        elif event == CHECK_HINT:
            success = check_hint_callback(play_rows, play_letters)
        elif event == CHECK_SAVE:
            success = check_save_callback(play_rows, play_letters)
        elif event == GIVE_HINT:
            give_hint_callback(play_size, play_rows, play_cols, play_letters)
        else:
            expect(False, "Unknown event {}".format(event))
    except BaseException as e:
        success = False
        print(e)

    return ctypes.c_bool(success)
