import random
from constants import PlayerState, DieSide
from typing import List

class PlayerStrategy:
  def yield_tokyo(self, me: PlayerState, other_player: PlayerState):
    return random.random() < 0.5
    
  def keep_dice(self, me: PlayerState, other_player: PlayerState, dice: List[DieSide], reroll_n: int):
    return [random.random() < 0.5 for _ in range(len(dice))]