#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "render.h"

#define WIDTH 1280
#define HEIGHT 720

typedef struct {
    const char *name;
    GameMode mode;
    float scroll;
    float player_y;
    unsigned progress;
    unsigned frame;
} Preview;

static const Preview previews[] = {
    { "title", GAME_TITLE, 0.0f, 384.0f, 0, 0 },
    { "gameplay_approach", GAME_PLAYING, 250.0f, 384.0f, 4, 50 },
    { "gameplay_jump", GAME_PLAYING, 300.0f, 340.0f, 5, 60 },
    { "gameplay_run", GAME_PLAYING, 1800.0f, 384.0f, 34, 180 },
    { "paused", GAME_PAUSED, 1800.0f, 384.0f, 34, 180 },
    { "crashed", GAME_CRASHED, 330.0f, 384.0f, 6, 72 },
    { "complete", GAME_COMPLETE, 5200.0f, 384.0f, 100, 1000 }
};

static int write_preview(const Preview *preview, const char *directory)
{
    uint32_t *framebuffer = malloc((size_t)WIDTH * HEIGHT * sizeof(*framebuffer));
    GameState game;
    char path[512];
    FILE *file;

    if (!framebuffer)
        return 0;

    game_init(&game);
    game.mode = preview->mode;
    game.scroll = preview->scroll;
    game.player_y = preview->player_y;
    game.progress = preview->progress;
    game.frame = preview->frame;
    render_game(framebuffer, WIDTH, HEIGHT, &game);

    snprintf(path, sizeof(path), "%s/%s.ppm", directory, preview->name);
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
    for (size_t i = 0; i < sizeof(previews) / sizeof(previews[0]); i++) {
        if (!write_preview(&previews[i], directory)) {
            fprintf(stderr, "could not write preview: %s\n", previews[i].name);
            return 1;
        }
    }
    return 0;
}
