#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int MAX_HEALTH = 10;
int VICTORY_PTS_WIN = 20;
int DIE_COUNT = 6;

typedef enum DieSide {
    ATTACK,
    HEAL,
    ONE,
    TWO,
    THREE
} DieSide;

typedef struct PlayerState {
    int health;
    int victory_points;
    int in_tokyo;
} PlayerState;

typedef struct GameState {
    PlayerState players[2];
    int winner;
    int current_player_idx;
} GameState;

typedef struct PlayerStrategy {
    int (*yield_tokyo)(PlayerState* me, PlayerState* other);
    void (*keep_dice)(PlayerState* me, PlayerState* other, DieSide* dice, int reroll_n, int* keep_mask);
} PlayerStrategy;


// Random agent

int random_yield_tokyo(PlayerState* me, PlayerState* other) {
    return rand() % 2;
}
void random_keep_dice(PlayerState* me, PlayerState* other, DieSide* dice, int reroll_n, int* keep_mask) {
    for (int i = 0; i < DIE_COUNT; i++) {
        keep_mask[i] = rand() % 2;
    }
}

PlayerStrategy RANDOM_AGENT = (PlayerStrategy){
    .yield_tokyo = random_yield_tokyo,
    .keep_dice = random_keep_dice
};


// Angry agent

int angry_yield_tokyo(PlayerState* me, PlayerState* other) {
    return me->health <= 5;
}
void angry_keep_dice(PlayerState* me, PlayerState* other, DieSide* dice, int reroll_n, int* keep_mask) {
    int heals = 0;
    int to_heal = MAX_HEALTH - me->health;
    for (int i = 0; i < DIE_COUNT; i++) {
        if (dice[i] == HEAL) {
            if (heals < to_heal && !me->in_tokyo) {
                keep_mask[i] = 1;
                heals++;
            } else {
                keep_mask[i] = 0;
            }
        } else if (dice[i] == ATTACK) {
            keep_mask[i] = 1;
        } else {
            keep_mask[i] = 0;
        }
    }
}

PlayerStrategy ANGRY_AGENT = (PlayerStrategy){
    .yield_tokyo = angry_yield_tokyo,
    .keep_dice = angry_keep_dice
};


// Monte carlo strategy

void step(GameState* game, PlayerStrategy* player_strategies[2]);
int mc_yield_tokyo(PlayerState *me, PlayerState* other) {
    PlayerStrategy* strategies[2] = { &RANDOM_AGENT, &RANDOM_AGENT };
    int wins[2] = { 0, 0 };
    int n_games = 100;

    for (int action = 0; action < 2; action++) {
        for (int i = 0; i < n_games; i++) {
            GameState game = (GameState){ .winner = -1, .current_player_idx = 0 };
            game.players[0] = (PlayerState){ .health = me->health, .victory_points = me->victory_points, .in_tokyo = action == 0 };
            game.players[1] = (PlayerState){ .health = other->health, .victory_points = other->victory_points, .in_tokyo = action == 1 };

            while (game.winner == -1) {
                step(&game, strategies);
            }
            wins[action] += game.winner == 0;
        }
    }

    return wins[1] >= wins[0];
}

PlayerStrategy MONTE_CARLO_AGENT = (PlayerStrategy){
    .yield_tokyo = mc_yield_tokyo,
    .keep_dice = random_keep_dice,
};


// Game logic

int min(int a, int b) {
    return a < b ? a : b;
}

void start_turn(GameState* game) {
    if (game->players[game->current_player_idx].in_tokyo) {
        game->players[game->current_player_idx].victory_points += 2;
    }
}

DieSide roll_die(void) {
    return (DieSide) rand() % 5;
}

void roll_dice(GameState* game, DieSide dice[DIE_COUNT], PlayerStrategy* strategy) {
    int keep_mask[DIE_COUNT];
    for (int i = 0; i < DIE_COUNT; i++) {
        dice[i] = roll_die();
    }
    for (int roll = 0; roll < 2; roll++) {
        PlayerState* me = &game->players[game->current_player_idx];
        PlayerState* other = &game->players[(game->current_player_idx + 1) % 2];
        strategy->keep_dice(me, other, dice, roll, keep_mask);
        for (int i = 0; i < DIE_COUNT; i++) {
            if (!keep_mask[i]) {
                dice[i] = roll_die();
            }
        }
    }
}

void resolve_victory_point_dice(GameState* game, DieSide dice[DIE_COUNT]) {
    for (DieSide dieside = ONE; dieside <= THREE; dieside = (DieSide)(dieside + 1)) {
        int cnt = 0;
        for (int i = 0; i < DIE_COUNT; i++) {
            if (dice[i] == dieside) { cnt++; }
        }
        if (cnt >= 3) {
            game->players[game->current_player_idx].victory_points += (dieside - ONE + 1) + (cnt - 3);
        }
    }
}

void resolve_health_dice(GameState* game, DieSide dice[DIE_COUNT]) {
    int heals = 0;
    for (int i = 0; i < DIE_COUNT; i++) {
        if (dice[i] == HEAL) { heals++; }
    }
    if (!game->players[game->current_player_idx].in_tokyo) {
        game->players[game->current_player_idx].health = min(MAX_HEALTH, game->players[game->current_player_idx].health + heals);
    }
}

void resolve_attack_dice(GameState* game, DieSide dice[DIE_COUNT], PlayerStrategy* strategy) {
    PlayerState* current_player = &game->players[game->current_player_idx];
    PlayerState* other_player = &game->players[(game->current_player_idx + 1) % 2];
    int attack = 0;
    for (int i = 0; i < DIE_COUNT; i++) {
        if (dice[i] == ATTACK) { attack++; }
    }
    if (current_player->in_tokyo) {
        other_player->health -= attack;
    } else if (other_player->in_tokyo) {
        other_player->health -= attack;
        if (attack > 0 && strategy->yield_tokyo(other_player, current_player)) {
            current_player->in_tokyo = 1;
            other_player->in_tokyo = 0;
        }
    } else {
        current_player->in_tokyo = 1;
        current_player->victory_points++;
    }
}

void resolve_dice(GameState* game, DieSide dice[DIE_COUNT], PlayerStrategy* strategy) {
    resolve_victory_point_dice(game, dice);
    resolve_health_dice(game, dice);
    resolve_attack_dice(game, dice, strategy);
}

void check_winner(GameState* game) {
    for (int i = 0; i < 2; i++) {
        if (game->players[i].health <= 0) {
            game->winner = (i + 1) % 2;
        }
        if (game->players[i].victory_points >= VICTORY_PTS_WIN) {
            game->winner = i;
        }
    }
}

void step(GameState* game, PlayerStrategy* player_strategies[2]) {
    DieSide dice[DIE_COUNT];
    start_turn(game);
    roll_dice(game, dice, player_strategies[game->current_player_idx]);
    resolve_dice(game, dice, player_strategies[(game->current_player_idx + 1) % 2]);
    check_winner(game);
    game->current_player_idx = (game->current_player_idx + 1) % 2;
}

int rollout(GameState* game, PlayerStrategy* player_strategies[2]) {
    while (game->winner == -1) {
        step(game, player_strategies);
    }
    return game->winner;
}


// Placeholder functions for the random agent
void step_random(GameState* game) {
    PlayerStrategy* strategies[2] = { &RANDOM_AGENT, &RANDOM_AGENT };
    step(game, strategies);
}

int rollout_random(GameState* game) {
    PlayerStrategy* strategies[2] = { &RANDOM_AGENT, &RANDOM_AGENT };
    return rollout(game, strategies);
}

void seed() {
    srand(time(NULL));
}


int main() {
    seed();
    int N_GAMES = 1000;
    int player_one_wins = 0;
    int total_steps = 0;
    float start_time = (float)clock()/CLOCKS_PER_SEC;
    // PlayerStrategy* player_strategies[2] = { &RANDOM_AGENT, &RANDOM_AGENT };
    PlayerStrategy* player_strategies[2] = { &MONTE_CARLO_AGENT, &RANDOM_AGENT };
    // PlayerStrategy* player_strategies[2] = { &RANDOM_AGENT, &ANGRY_AGENT };

    for (int i = 0; i < N_GAMES; i++) {
        GameState game = (GameState){ .winner = -1, .current_player_idx = i % 2 };
        for (int i = 0; i < 2; i++) {
            game.players[i] = (PlayerState){ .health = MAX_HEALTH, .victory_points = 0, .in_tokyo = 0 };
        }

        while (game.winner == -1) {
            step(&game, player_strategies);
            total_steps += 1;
        }
        if (game.winner == 0) {
            player_one_wins += 1;
        }
    }

    float end_time = (float)clock()/CLOCKS_PER_SEC;
    printf("player one won %d/%d games against player two\n", player_one_wins, N_GAMES);
    printf("total time: %fs | steps per game: %f\n", end_time - start_time, (float)total_steps / N_GAMES);
    return 0;
}
