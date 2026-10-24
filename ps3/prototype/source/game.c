#include "game.h"

#define PLAYER_X       172.0f
#define PLAYER_SIZE     48.0f
#define GROUND_Y       432.0f
#define GRAVITY          0.62f
#define JUMP_VELOCITY  -13.3f
#define RUN_SPEED        5.0f
#define LEVEL_LENGTH  5200.0f

/* World coordinates for a short, deliberately hand-tuned prototype course. */
static const float spikes[] = {
    530, 790, 1060, 1098, 1390, 1690, 1730, 1770,
    2110, 2410, 2630, 2670, 2710, 3090, 3430, 3470,
    3810, 4070, 4110, 4450, 4770, 4810
};

void game_init(GameState *game)
{
    game->mode = GAME_TITLE;
    game->scroll = 0.0f;
    game->player_y = GROUND_Y - PLAYER_SIZE;
    game->player_vy = 0.0f;
    game->frame = 0;
    game->progress = 0;
}

static void begin_run(GameState *game)
{
    game_init(game);
    game->mode = GAME_PLAYING;
}

static bool point_in_spike(float x, float y, float left)
{
    const float base_y = GROUND_Y;
    const float apex_y = GROUND_Y - 38.0f;
    const float center_x = left + 19.0f;
    float half_width;

    if (y < apex_y || y > base_y || x < left || x > left + 38.0f)
        return false;

    half_width = (y - apex_y) * 0.5f;
    return x >= center_x - half_width && x <= center_x + half_width;
}

static bool player_hits_spike(const GameState *game, float spike_x)
{
    const float x0 = PLAYER_X + game->scroll;
    const float x1 = x0 + PLAYER_SIZE;
    const float y0 = game->player_y;
    const float y1 = y0 + PLAYER_SIZE;
    float left = spike_x;

    /* Check the cube's corners against the triangular obstacle. */
    if (point_in_spike(x0, y0, left) || point_in_spike(x1, y0, left) ||
        point_in_spike(x0, y1, left) || point_in_spike(x1, y1, left))
        return true;

    /* Also catch a spike wholly contained by the cube's bounding rectangle. */
    return left + 19.0f >= x0 && left + 19.0f <= x1 &&
           GROUND_Y - 38.0f >= y0 && GROUND_Y - 38.0f <= y1;
}

void game_update(GameState *game, bool jump_pressed, bool pause_pressed, bool restart_pressed)
{
    unsigned i;

    if (game->mode == GAME_TITLE) {
        if (jump_pressed)
            begin_run(game);
        return;
    }

    if (game->mode == GAME_CRASHED || game->mode == GAME_COMPLETE) {
        if (jump_pressed || restart_pressed)
            begin_run(game);
        return;
    }

    if (game->mode == GAME_PAUSED) {
        if (pause_pressed)
            game->mode = GAME_PLAYING;
        else if (restart_pressed)
            begin_run(game);
        return;
    }

    if (pause_pressed) {
        game->mode = GAME_PAUSED;
        return;
    }

    game->frame++;
    game->scroll += RUN_SPEED;

    if (jump_pressed && game->player_y >= GROUND_Y - PLAYER_SIZE - 0.01f)
        game->player_vy = JUMP_VELOCITY;

    game->player_vy += GRAVITY;
    game->player_y += game->player_vy;
    if (game->player_y >= GROUND_Y - PLAYER_SIZE) {
        game->player_y = GROUND_Y - PLAYER_SIZE;
        game->player_vy = 0.0f;
    }

    for (i = 0; i < sizeof(spikes) / sizeof(spikes[0]); i++) {
        if (spikes[i] + 38.0f < game->scroll + PLAYER_X ||
            spikes[i] > game->scroll + PLAYER_X + PLAYER_SIZE)
            continue;
        if (player_hits_spike(game, spikes[i])) {
            game->mode = GAME_CRASHED;
            return;
        }
    }

    game->progress = (unsigned)((game->scroll / LEVEL_LENGTH) * 100.0f);
    if (game->progress > 100)
        game->progress = 100;
    if (game->scroll >= LEVEL_LENGTH)
        game->mode = GAME_COMPLETE;
}
