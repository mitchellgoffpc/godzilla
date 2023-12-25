from constants import PlayerState, DieSide, MAX_HEALTH
from typing import List

class PlayerStrategy:
    def yield_tokyo(self, me, other_player):
        return me.health <= 5
        
    def keep_dice(self, me: PlayerState, other_player: PlayerState, dice: List[DieSide], reroll_n: int):
        heals = 0
        to_heal = MAX_HEALTH - me.health
        keep_mask = []
        for die in dice:
            if die == DieSide.HEAL:
                if heals < to_heal and not me.in_tokyo:
                    keep_mask.append(True)
                    heals += 1
                else:
                    keep_mask.append(False)
            elif die == DieSide.ATTACK:
                keep_mask.append(True)
            else:
                keep_mask.append(False)
        return keep_mask