#ifndef GDPS3_PROTOTYPE_GAME_H
#define GDPS3_PROTOTYPE_GAME_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    GAME_TITLE,
    GAME_PLAYING,
    GAME_PAUSED,
    GAME_CRASHED,
    GAME_COMPLETE
} GameMode;

typedef struct {
    GameMode mode;
    float scroll;
    float player_y;
    float player_vy;
    unsigned frame;
    unsigned progress;
} GameState;

void game_init(GameState *game);
void game_update(GameState *game, bool jump_pressed, bool pause_pressed, bool restart_pressed);

#endif
