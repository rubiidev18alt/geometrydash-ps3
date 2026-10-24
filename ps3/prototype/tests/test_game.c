#include <assert.h>
#include <stdio.h>

#include "game.h"
#include "render.h"

static uint32_t framebuffer[960 * 540];

int main(void)
{
    GameState game;
    PrototypeAssets assets;
    assert(prototype_assets_load_embedded(&assets));
    assert(assets.cube_primary.width == 30 && assets.cube_primary.height == 30);
    assert(assets.background.width == 512 && assets.background.height == 512);
    game_init(&game);
    assert(game.mode == GAME_TITLE);
    assert(game.progress == 0);
    render_game(framebuffer, 960, 540, &game, &assets);
    assert(framebuffer[432 * 960 + 10] == 0x0048f0e5u); /* ground edge */
    {
        size_t green_pixels = 0;
        size_t cyan_cube_pixels = 0;
        for (size_t i = 0; i < 960u * 540u; i++)
            if (((framebuffer[i] >> 16) & 255) < 20 &&
                ((framebuffer[i] >> 8) & 255) > 180 && (framebuffer[i] & 255) < 20)
                green_pixels++;
        for (unsigned y = 384; y < 432; y++)
            for (unsigned x = 160; x < 208; x++) {
                uint32_t pixel = framebuffer[y * 960 + x];
                if (((pixel >> 16) & 255) < 20 &&
                    ((pixel >> 8) & 255) > 180 && (pixel & 255) > 180)
                    cyan_cube_pixels++;
            }
        assert(green_pixels > 20); /* default green P1 tint */
        assert(cyan_cube_pixels > 0); /* default cyan P2 detail tint */
    }

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

    /* Blocks are solid hazards too, using the same added obstacle placement. */
    game_init(&game);
    game.mode = GAME_PLAYING;
    game.scroll = 2190.0f;
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

    prototype_assets_free(&assets);
    puts("prototype gameplay and asset tests passed");
    return 0;
}
