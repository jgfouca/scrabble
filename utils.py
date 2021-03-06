"""
Utilities
"""

import ctypes, sys

###############################################################################
def expect(condition, error_msg, exc_type=SystemExit, error_prefix="ERROR:"):
###############################################################################
    """
    Similar to assert except doesn't generate an ugly stacktrace. Useful for
    checking user error, not programming error.

    >>> expect(True, "error1")
    >>> expect(False, "error2")
    Traceback (most recent call last):
        ...
    SystemExit: ERROR: error2
    """
    if not condition:
        msg = error_prefix + " " + error_msg
        raise exc_type(msg)

###############################################################################
def to_cstr(item):
###############################################################################
    return bytes(item, "utf-8")

###############################################################################
def to_cstr_list(items):
###############################################################################
    arr = (ctypes.c_char_p * len(items))()
    arr[:] = [to_cstr(item) for item in items]

    return arr

###############################################################################
def cstr_to_str(cstr, size):
###############################################################################
    return cstr[0:size].decode("utf-8")

###############################################################################
def cstr_to_ints(cstr, size):
###############################################################################
    ints = []
    for i in range(size):
        ints.append(int.from_bytes(cstr[i], byteorder=sys.byteorder))

    return ints

###############################################################################
def tyn(arg_bool):
###############################################################################
    return "y" if arg_bool else "n"
