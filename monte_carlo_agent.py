import random
from constants import PlayerState, GameState, DieSide
from typing import List
from cgame import rollout

class PlayerStrategy:
    def yield_tokyo(self, me: PlayerState, other_player: PlayerState):
        wins = [0, 0]
        N_GAMES = 100

        for action in range(2):
            for i in range(N_GAMES):
                players = [
                    PlayerState(me.health, me.victory_points, action == 0),
                    PlayerState(other_player.health, other_player.victory_points, action == 1)]
                state = GameState((PlayerState * 2)(*players), -1, 0)
                wins[action] += rollout(state) == 0
        
        return wins[1] >= wins[0]

    def keep_dice(self, me: PlayerState, other_player: PlayerState, dice: List[DieSide], reroll_n: int):
        return [random.random() < 0.5 for _ in range(len(dice))]
