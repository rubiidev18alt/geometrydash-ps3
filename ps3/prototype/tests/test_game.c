#include <assert.h>
#include <stdio.h>

#include "game.h"
#include "render.h"

static uint32_t framebuffer[960 * 540];

int main(void)
{
    GameState game;
    game_init(&game);
    assert(game.mode == GAME_TITLE);
    assert(game.progress == 0);
    render_game(framebuffer, 960, 540, &game);
    assert(framebuffer[0] == 0x00101a36u);
    assert(framebuffer[384 * 960 + 173] == 0x0048f0e5u); /* cube */
    assert(framebuffer[394 * 960 + 549] == 0x00ff527du); /* first spike tip */

    game_update(&game, true, false, false);
    assert(game.mode == GAME_PLAYING);

    game_update(&game, true, false, false);
    assert(game.player_y < 384.0f); /* Cross makes the cube jump. */
    assert(game.scroll > 0.0f);

    game_update(&game, false, true, false);
    assert(game.mode == GAME_PAUSED);
    {
        float paused_scroll = game.scroll;
        game_update(&game, false, false, false);
        assert(game.scroll == paused_scroll);
    }
    game_update(&game, false, true, false);
    assert(game.mode == GAME_PLAYING);

    /* A no-input run reaches the opening spike and fails instead of passing through. */
    game_init(&game);
    game.mode = GAME_PLAYING;
    for (int i = 0; i < 100 && game.mode == GAME_PLAYING; i++)
        game_update(&game, false, false, false);
    assert(game.mode == GAME_CRASHED);

    /* Circle restarts from a crash; the ending is reachable as well. */
    game_update(&game, false, false, true);
    assert(game.mode == GAME_PLAYING);
    assert(game.scroll == 0.0f);
    game.scroll = 5196.0f;
    game_update(&game, false, false, false);
    assert(game.mode == GAME_COMPLETE);
    assert(game.progress == 100);

    puts("prototype gameplay tests passed");
    return 0;
}
