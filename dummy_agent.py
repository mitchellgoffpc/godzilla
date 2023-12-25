import random
from constants import PlayerState, DieSide
from typing import List

class PlayerStrategy:
  def yield_tokyo(self, me: PlayerState, other_player: PlayerState):
    # TODO implement smart logic
    return True
    
  def keep_dice(self, me: PlayerState, other_player: PlayerState, dice: List[DieSide], reroll_n: int):
    # TODO implement smart logic
    return [True for _ in range(len(dice))]