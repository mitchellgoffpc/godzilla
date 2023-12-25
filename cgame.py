import os
import ctypes
os.system('gcc --shared -O2 -o game.so game.c')
lib = ctypes.CDLL('game.so')
lib.seed()

def step(state):
    lib.step_random(ctypes.pointer(state))

def rollout(state):
    return lib.rollout_random(ctypes.pointer(state))
