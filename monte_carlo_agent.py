import random
from typing import List
from constants import PlayerState, GameState, DieSide, DIE_COUNT
from cgame import rollout, resolve_dice, check_winner

def get_keep_mask(idx):
    assert idx < 64
    return [bool((idx >> i) & 1) for i in range(DIE_COUNT)]

class PlayerStrategy:
    def yield_tokyo(self, me: PlayerState, other_player: PlayerState):
        N_ACTIONS = 2
        N_GAMES = 100
        wins = [0] * 2

        for action in range(N_ACTIONS):
            for i in range(N_GAMES):
                players = [
                    PlayerState(me.health, me.victory_points, action == 0),
                    PlayerState(other_player.health, other_player.victory_points, action == 1)]
                state = GameState((PlayerState * 2)(*players), -1, 0)
                wins[action] += rollout(state) == 0

        return wins[1] >= wins[0]

    def keep_dice(self, me: PlayerState, other_player: PlayerState, dice: List[DieSide], reroll_n: int):
        N_ACTIONS = 64
        N_GAMES = 5
        wins = [0] * N_ACTIONS
        
        for action in range(N_ACTIONS):
            keep_mask = get_keep_mask(action)
            for i in range(N_GAMES):
                players = [
                    PlayerState(me.health, me.victory_points, me.in_tokyo),
                    PlayerState(other_player.health, other_player.victory_points, other_player.in_tokyo)]
                state = GameState((PlayerState * 2)(*players), -1, 0)
                new_dice = [dice[i] for i in range(DIE_COUNT) if keep_mask[i]] + random.choices(list(DieSide), k=DIE_COUNT - sum(keep_mask))
                resolve_dice(state, new_dice)
                check_winner(state)
                state.current_player_idx = (state.current_player_idx + 1) % 2
                wins[action] += rollout(state) == 0
        
        max_wins = max(wins)
        best_actions = [i for i, value in enumerate(wins) if value == max_wins]
        action_idx = random.choice(best_actions)
        return get_keep_mask(action_idx)
