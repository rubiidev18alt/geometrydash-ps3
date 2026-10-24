#include <ppu-types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include <io/pad.h>
#include <rsx/rsx.h>
#include <sys/process.h>
#include <sysutil/sysutil.h>

#include "game.h"
#include "render.h"
#include "rsxutil.h"

SYS_PROCESS_PARAM(1001, 0x100000);

static volatile int running = 1;

static void sysutil_callback(u64 status, u64 param, void *userdata)
{
    (void)param;
    (void)userdata;
    if (status == SYSUTIL_EXIT_GAME)
        running = 0;
}

static void shutdown_graphics(void)
{
    sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);
    if (rsx_context) {
        gcmSetWaitFlip(rsx_context);
        rsxFinish(rsx_context, 1);
    }
}

int main(int argc, const char *argv[])
{
    void *host_memory;
    GameState game;
    padInfo pad_info;
    padData pad_data;
    int old_cross = 0;
    int old_start = 0;
    int old_circle = 0;
    int held_cross = 0;
    int held_start = 0;
    int held_circle = 0;

    (void)argc;
    (void)argv;
    printf("Geometry Dash PS3 gameplay prototype\n");

    host_memory = memalign(1024 * 1024, RSX_HOST_SIZE);
    if (!host_memory) {
        printf("PS3 prototype: RSX host memory allocation failed\n");
        return 1;
    }

    if (!rsx_video_init(host_memory, RSX_HOST_SIZE) ||
        !rsx_context || !rsx_framebuffer[0] || !rsx_framebuffer[1])
        return 1;

    ioPadInit(7);
    sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sysutil_callback, NULL);
    atexit(shutdown_graphics);
    game_init(&game);
    memset(&pad_info, 0, sizeof(pad_info));
    memset(&pad_data, 0, sizeof(pad_data));

    while (running) {
        int cross = 0;
        int start = 0;
        int circle = 0;
        bool connected = false;

        sysUtilCheckCallback();
        ioPadGetInfo(&pad_info);
        for (int i = 0; i < MAX_PADS; i++) {
            if (!pad_info.status[i])
                continue;
            if (ioPadGetData(i, &pad_data) == 0) {
                /* PSL1GHT reports button data only when it changes (len > 0). */
                if (pad_data.len > 0) {
                    held_cross = pad_data.BTN_CROSS != 0;
                    held_start = pad_data.BTN_START != 0;
                    held_circle = pad_data.BTN_CIRCLE != 0;
                }
                cross = held_cross;
                start = held_start;
                circle = held_circle;
                connected = true;
                break;
            }
        }

        if (!connected) {
            held_cross = held_start = held_circle = 0;
            cross = start = circle = 0;
        }
        game_update(&game, cross && !old_cross, start && !old_start,
                    circle && !old_circle);
        old_cross = connected ? cross : 0;
        old_start = connected ? start : 0;
        old_circle = connected ? circle : 0;

        render_game(rsx_framebuffer[rsx_current_fb], rsx_width, rsx_height, &game);
        rsx_video_flip();
    }

    /* host_memory backs the RSX command buffer until process shutdown. */
    return 0;
}
