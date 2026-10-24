#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "render.h"

#define WIDTH 1280
#define HEIGHT 720

static const char *mode_name(GameMode mode)
{
    switch (mode) {
    case GAME_TITLE: return "title";
    case GAME_PLAYING: return "gameplay";
    case GAME_PAUSED: return "paused";
    case GAME_CRASHED: return "crashed";
    case GAME_COMPLETE: return "complete";
    default: return "preview";
    }
}

static int write_preview(GameMode mode, const char *directory)
{
    uint32_t *framebuffer = malloc((size_t)WIDTH * HEIGHT * sizeof(*framebuffer));
    GameState game;
    char path[512];
    FILE *file;

    if (!framebuffer)
        return 0;

    game_init(&game);
    game.mode = mode;
    if (mode == GAME_PLAYING || mode == GAME_PAUSED) {
        game.scroll = 1800.0f;
        game.progress = 42;
        game.player_y = 384.0f;
        game.frame = 180;
    } else if (mode == GAME_CRASHED) {
        game.scroll = 330.0f;
        game.progress = 6;
        game.mode = GAME_CRASHED;
    } else if (mode == GAME_COMPLETE) {
        game.scroll = 5200.0f;
        game.progress = 100;
        game.mode = GAME_COMPLETE;
    }
    render_game(framebuffer, WIDTH, HEIGHT, &game);

    snprintf(path, sizeof(path), "%s/%s.ppm", directory, mode_name(mode));
    file = fopen(path, "wb");
    if (!file) {
        free(framebuffer);
        return 0;
    }
    fprintf(file, "P6\n%u %u\n255\n", WIDTH, HEIGHT);
    for (size_t i = 0, n = (size_t)WIDTH * HEIGHT; i < n; i++) {
        unsigned char rgb[3] = {
            (unsigned char)(framebuffer[i] >> 16),
            (unsigned char)(framebuffer[i] >> 8),
            (unsigned char)framebuffer[i]
        };
        fwrite(rgb, 1, sizeof(rgb), file);
    }
    fclose(file);
    free(framebuffer);
    return 1;
}

int main(int argc, char **argv)
{
    const char *directory = argc > 1 ? argv[1] : ".";
    const GameMode modes[] = { GAME_TITLE, GAME_PLAYING, GAME_PAUSED, GAME_CRASHED, GAME_COMPLETE };
    for (size_t i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
        if (!write_preview(modes[i], directory)) {
            fprintf(stderr, "could not write preview for %s\n", mode_name(modes[i]));
            return 1;
        }
    }
    return 0;
}
