import ctypes

MAX_HEALTH = 10
VICTORY_PTS_WIN = 20
DIE_COUNT = 6

class DieSide:
    ATTACK = 'Attack'
    HEAL = 'Heal'
    ONE = '1'
    TWO = '2'
    THREE = '3'

class PlayerState(ctypes.Structure):
    _fields_ = [
        ("health", ctypes.c_int32),
        ("victory_points", ctypes.c_int32),
        ("in_tokyo", ctypes.c_int32),
    ]

class GameState(ctypes.Structure):
    _fields_ = [
        ('players', PlayerState * 2),
        ('winner', ctypes.c_int32),
        ('current_player_idx', ctypes.c_int32),
    ]
