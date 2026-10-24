#include "render.h"

#include <stddef.h>
#include <string.h>

#define VIRTUAL_W 960
#define VIRTUAL_H 540
#define GROUND_Y  432
#define PLAYER_X  172
#define PLAYER_SZ 48
#define LEVEL_END 5200.0f

#define COLOR_BG       0x00101a36u
#define COLOR_GRID     0x001c2b4eu
#define COLOR_GROUND   0x00212e4bu
#define COLOR_GROUND2  0x0038496cu
#define COLOR_CYAN     0x0048f0e5u
#define COLOR_WHITE    0x00f5fbffu
#define COLOR_PINK     0x00ff527du
#define COLOR_YELLOW   0x00ffe86bu
#define COLOR_PANEL    0x0015213bu

/* Five-bit rows, readable at small sizes and independent of external assets. */
static const unsigned char font[36][7] = {
    {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30}, {14,17,16,16,16,17,14},
    {30,17,17,17,17,17,30}, {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
    {14,17,16,23,17,17,15}, {17,17,17,31,17,17,17}, {14,4,4,4,4,4,14},
    {7,2,2,2,18,18,12}, {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17}, {14,17,17,17,17,17,14},
    {30,17,17,30,16,16,16}, {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4}, {17,17,17,17,17,17,14},
    {17,17,17,17,17,10,4}, {17,17,17,21,21,21,10}, {17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31},
    {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14}, {14,17,1,2,4,8,31},
    {30,1,1,14,1,1,30}, {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
    {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8}, {14,17,17,14,17,17,14},
    {14,17,17,15,1,1,14}
};

static unsigned sx(unsigned x, unsigned width)
{
    return (x * width) / VIRTUAL_W;
}

static unsigned sy(unsigned y, unsigned height)
{
    return (y * height) / VIRTUAL_H;
}

static void fill_rect(uint32_t *fb, unsigned width, unsigned height,
                      int x, int y, int w, int h, uint32_t color)
{
    int x0, x1, y0, y1, row;
    int vx0, vx1, vy0, vy1;
    uint32_t *dst;

    if (w <= 0 || h <= 0 || !fb || !width || !height)
        return;

    vx0 = x < 0 ? 0 : x;
    vy0 = y < 0 ? 0 : y;
    vx1 = x + w > VIRTUAL_W ? VIRTUAL_W : x + w;
    vy1 = y + h > VIRTUAL_H ? VIRTUAL_H : y + h;
    if (vx1 <= vx0 || vy1 <= vy0)
        return;

    x0 = (int)sx((unsigned)vx0, width);
    y0 = (int)sy((unsigned)vy0, height);
    x1 = (int)sx((unsigned)vx1, width);
    y1 = (int)sy((unsigned)vy1, height);
    if (x1 > (int)width) x1 = (int)width;
    if (y1 > (int)height) y1 = (int)height;
    if (x1 <= x0) x1 = x0 + 1;
    if (y1 <= y0) y1 = y0 + 1;

    for (row = y0; row < y1; row++) {
        dst = fb + (size_t)row * width + x0;
        for (int col = x0; col < x1; col++)
            *dst++ = color;
    }
}

static int glyph_index(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return 26 + c - '0';
    return -1;
}

static void draw_text(uint32_t *fb, unsigned width, unsigned height,
                      int x, int y, const char *text, unsigned scale, uint32_t color)
{
    if (scale == 0) scale = 1;

    while (*text) {
        int idx = glyph_index(*text++);
        if (*text == '\0' && idx < 0) break;
        if (idx < 0) {
            if (text[-1] == ':') {
                fill_rect(fb, width, height, x + 2 * (int)scale, y + 1 * (int)scale,
                          (int)scale, (int)scale, color);
                fill_rect(fb, width, height, x + 2 * (int)scale, y + 5 * (int)scale,
                          (int)scale, (int)scale, color);
            } else if (text[-1] == '%') {
                fill_rect(fb, width, height, x, y, scale, scale, color);
                fill_rect(fb, width, height, x + 4 * (int)scale, y + 6 * (int)scale,
                          scale, scale, color);
                fill_rect(fb, width, height, x + 1 * (int)scale, y + 5 * (int)scale,
                          scale, scale, color);
                fill_rect(fb, width, height, x + 3 * (int)scale, y + 1 * (int)scale,
                          scale, scale, color);
            }
            x += 6 * (int)scale;
            continue;
        }
        for (int row = 0; row < 7; row++) {
            unsigned char bits = font[idx][row];
            for (int col = 0; col < 5; col++) {
                if (bits & (1u << (4 - col)))
                    fill_rect(fb, width, height, x + col * (int)scale,
                              y + row * (int)scale, scale, scale, color);
            }
        }
        x += 6 * (int)scale;
    }
}

static void centered_text(uint32_t *fb, unsigned width, unsigned height,
                          int y, const char *text, unsigned scale, uint32_t color)
{
    unsigned len = (unsigned)strlen(text);
    int x = (VIRTUAL_W - (int)(len * 6 * scale)) / 2;
    draw_text(fb, width, height, x, y, text, scale, color);
}

static void draw_spike(uint32_t *fb, unsigned width, unsigned height, int x)
{
    const int top = GROUND_Y - 38;
    for (int row = 0; row < 38; row++) {
        int half = row / 2;
        fill_rect(fb, width, height, x + 19 - half, top + row, half * 2 + 1, 1, COLOR_PINK);
    }
}

static void draw_player(uint32_t *fb, unsigned width, unsigned height, int y, unsigned frame)
{
    /* A simple bright cube, with a one-pixel color split to suggest rotation. */
    fill_rect(fb, width, height, PLAYER_X, y, PLAYER_SZ, PLAYER_SZ, COLOR_CYAN);
    fill_rect(fb, width, height, PLAYER_X + 5, y + 5, PLAYER_SZ - 10, PLAYER_SZ - 10, COLOR_WHITE);
    fill_rect(fb, width, height, PLAYER_X + 10, y + 10, PLAYER_SZ - 20, PLAYER_SZ - 20,
              (frame / 8) % 2 ? COLOR_CYAN : COLOR_YELLOW);
    fill_rect(fb, width, height, PLAYER_X + 14, y + 14, 6, 6, COLOR_PANEL);
    fill_rect(fb, width, height, PLAYER_X + 28, y + 14, 6, 6, COLOR_PANEL);
}

static void draw_scene(uint32_t *fb, unsigned width, unsigned height, const GameState *game)
{
    int offset = (int)game->scroll;
    const int spikes[] = {
        530,790,1060,1098,1390,1690,1730,1770,2110,2410,2630,2670,2710,
        3090,3430,3470,3810,4070,4110,4450,4770,4810
    };

    /* Solid, fast-to-draw backdrop plus parallax bars and a lightweight grid. */
    for (size_t i = 0, n = (size_t)width * height; i < n; i++)
        fb[i] = COLOR_BG;
    for (int y = 54; y < GROUND_Y; y += 54)
        fill_rect(fb, width, height, 0, y, VIRTUAL_W, 1, COLOR_GRID);
    for (int i = 0; i < 12; i++) {
        int x = (i * 113 - (offset / 3) % 113 + VIRTUAL_W) % VIRTUAL_W;
        fill_rect(fb, width, height, x, 75 + (i % 4) * 54, 5, 5, COLOR_GRID);
    }

    fill_rect(fb, width, height, 0, GROUND_Y, VIRTUAL_W, VIRTUAL_H - GROUND_Y, COLOR_GROUND);
    fill_rect(fb, width, height, 0, GROUND_Y, VIRTUAL_W, 5, COLOR_CYAN);
    for (int x = -(offset % 64); x < VIRTUAL_W; x += 64)
        fill_rect(fb, width, height, x, GROUND_Y + 30, 2, VIRTUAL_H - GROUND_Y - 30, COLOR_GROUND2);

    for (unsigned i = 0; i < sizeof(spikes) / sizeof(spikes[0]); i++) {
        int x = spikes[i] - offset;
        if (x > -40 && x < VIRTUAL_W + 40)
            draw_spike(fb, width, height, x);
    }
    draw_player(fb, width, height, (int)game->player_y, game->frame);
}

static void draw_hud(uint32_t *fb, unsigned width, unsigned height, const GameState *game)
{
    fill_rect(fb, width, height, 22, 22, 560, 54, COLOR_PANEL);
    draw_text(fb, width, height, 40, 39, "PS3 PROTOTYPE", 3, COLOR_WHITE);
    fill_rect(fb, width, height, 760, 28, 174, 20, COLOR_PANEL);
    fill_rect(fb, width, height, 765, 33, 164, 10, COLOR_GROUND2);
    fill_rect(fb, width, height, 765, 33, (int)(164 * game->progress / 100), 10, COLOR_CYAN);
    draw_text(fb, width, height, 800, 58, "PROGRESS", 2, COLOR_WHITE);
}

void render_game(uint32_t *framebuffer, unsigned width, unsigned height,
                 const GameState *game)
{
    if (!framebuffer || !game || width == 0 || height == 0)
        return;

    draw_scene(framebuffer, width, height, game);
    draw_hud(framebuffer, width, height, game);

    switch (game->mode) {
    case GAME_TITLE:
        fill_rect(framebuffer, width, height, 160, 150, 640, 220, COLOR_PANEL);
        centered_text(framebuffer, width, height, 182, "GEOMETRY DASH", 5, COLOR_WHITE);
        centered_text(framebuffer, width, height, 270, "PRESS CROSS TO START", 3, COLOR_CYAN);
        centered_text(framebuffer, width, height, 320, "CROSS JUMP  START PAUSE", 2, COLOR_WHITE);
        break;
    case GAME_PAUSED:
        fill_rect(framebuffer, width, height, 270, 195, 420, 140, COLOR_PANEL);
        centered_text(framebuffer, width, height, 220, "PAUSED", 5, COLOR_YELLOW);
        centered_text(framebuffer, width, height, 295, "START RESUME  CIRCLE RETRY", 2, COLOR_WHITE);
        break;
    case GAME_CRASHED:
        fill_rect(framebuffer, width, height, 245, 185, 470, 155, COLOR_PANEL);
        centered_text(framebuffer, width, height, 210, "CRASHED", 5, COLOR_PINK);
        centered_text(framebuffer, width, height, 286, "CROSS OR CIRCLE TO RETRY", 2, COLOR_WHITE);
        break;
    case GAME_COMPLETE:
        fill_rect(framebuffer, width, height, 245, 185, 470, 155, COLOR_PANEL);
        centered_text(framebuffer, width, height, 210, "LEVEL COMPLETE", 4, COLOR_YELLOW);
        centered_text(framebuffer, width, height, 286, "CROSS TO PLAY AGAIN", 2, COLOR_WHITE);
        break;
    case GAME_PLAYING:
    default:
        break;
    }
}
