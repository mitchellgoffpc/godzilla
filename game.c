#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int MAX_HEALTH = 10;
int VICTORY_PTS_WIN = 20;
int DIE_COUNT = 6;

typedef enum {
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



int min(int a, int b) {
    return a < b ? a : b;
}

DieSide roll_die(void) {
    return (DieSide) rand() % 5;
}

void roll_n_dice(DieSide* dice, int n) {
    for (int i = 0; i < n; i++) {
        dice[i] = roll_die();
    }
}

void start_turn(GameState* game) {
    if (game->players[game->current_player_idx].in_tokyo) {
        game->players[game->current_player_idx].victory_points += 2;
    }
}

int other_player_yields_tokyo(GameState* game) {
    // Implement the yield_tokyo strategy here
    return rand() % 2;
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

void resolve_attack_dice(GameState* game, DieSide dice[DIE_COUNT]) {
    int attack = 0;
    for (int i = 0; i < DIE_COUNT; i++) {
        if (dice[i] == ATTACK) { attack++; }
    }
    if (game->players[game->current_player_idx].in_tokyo) {
        game->players[(game->current_player_idx + 1) % 2].health -= attack;
    } else if (game->players[(game->current_player_idx + 1) % 2].in_tokyo) {
        game->players[(game->current_player_idx + 1) % 2].health -= attack;
        if (other_player_yields_tokyo(game)) {
            game->players[game->current_player_idx].in_tokyo = 1;
            game->players[(game->current_player_idx + 1) % 2].in_tokyo = 0;
        }
    } else {
        game->players[game->current_player_idx].in_tokyo = 1;
        game->players[game->current_player_idx].victory_points++;
    }
}

void resolve_dice(GameState* game, DieSide dice[DIE_COUNT]) {
    resolve_victory_point_dice(game, dice);
    resolve_health_dice(game, dice);
    resolve_attack_dice(game, dice);
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

void step(GameState* game) {
    start_turn(game);
    DieSide dice[DIE_COUNT];
    roll_n_dice(dice, DIE_COUNT);
    resolve_dice(game, dice);
    check_winner(game);
    game->current_player_idx = (game->current_player_idx + 1) % 2;
}


int main() {
    srand(time(NULL));
    int N_GAMES = 1000000;
    int player_one_wins = 0;
    float start_time = (float)clock()/CLOCKS_PER_SEC;
    int total_steps = 0;

    for (int i = 0; i < N_GAMES; i++) {
        GameState game = (GameState){ .winner = -1, .current_player_idx = i % 2 };
        for (int i = 0; i < 2; i++) {
            game.players[i] = (PlayerState){ .health = MAX_HEALTH, .victory_points = 0, .in_tokyo = 0 };
        }
        while (game.winner == -1) {
            step(&game);
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
