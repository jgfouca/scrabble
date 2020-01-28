"""
Holds a simple image of the game state, receives events updating
the state (from C++) and presents the GUI.
"""

class PyScrabbleGame(object):

    def __init__(self):
        pass

    def play_event(arg):
        print("RECEIVED EVENT", arg)

GAME = PyScrabbleGame()

def test_func(arg):
    print("HERE")
    print("RECEIVED ARG", arg)
