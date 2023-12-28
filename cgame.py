import os
import ctypes
os.system('gcc --shared -O2 -o libgame.so game.c')
lib = ctypes.CDLL('libgame.so')
lib.seed()

def step(state):
    lib.step_random(ctypes.pointer(state))

def rollout(state):
    return lib.rollout_random(ctypes.pointer(state))

def resolve_dice(state, dice):
    from constants import DieSide
    die_indices = {x.value: i for i, x in enumerate(DieSide)}
    cdice = (ctypes.c_int32 * 6)(*[die_indices[x.value] for x in dice])
    lib.resolve_dice_random(ctypes.pointer(state), cdice)

def check_winner(state):
    return lib.check_winner(ctypes.pointer(state))