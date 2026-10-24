#include "rsxutil.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sysutil/video.h>

#define RSX_LABEL_INDEX 255

videoResolution rsx_resolution;
gcmContextData *rsx_context;
u32 rsx_width;
u32 rsx_height;
u32 rsx_current_fb;
u32 *rsx_framebuffer[2];
static u32 rsx_color_pitch;
static u32 rsx_color_offset[2];
static u32 rsx_depth_pitch;
static u32 rsx_depth_offset;
static u32 rsx_next_label = 1;
static int rsx_first_flip = 1;

static void wait_rsx_idle(void)
{
    rsxSetWriteBackendLabel(rsx_context, RSX_LABEL_INDEX, rsx_next_label);
    rsxSetWaitLabel(rsx_context, RSX_LABEL_INDEX, rsx_next_label);
    rsx_next_label++;
    rsxSetWriteBackendLabel(rsx_context, RSX_LABEL_INDEX, rsx_next_label);
    rsxFlushBuffer(rsx_context);
    while (*(volatile u32 *)gcmGetLabelAddress(RSX_LABEL_INDEX) != rsx_next_label)
        usleep(1000);
    rsx_next_label++;
}

void rsx_video_set_target(u32 index)
{
    gcmSurface surface;
    memset(&surface, 0, sizeof(surface));
    surface.colorFormat = GCM_SURFACE_X8R8G8B8;
    surface.colorTarget = GCM_SURFACE_TARGET_0;
    surface.colorLocation[0] = GCM_LOCATION_RSX;
    surface.colorOffset[0] = rsx_color_offset[index];
    surface.colorPitch[0] = rsx_color_pitch;
    surface.colorLocation[1] = GCM_LOCATION_RSX;
    surface.colorLocation[2] = GCM_LOCATION_RSX;
    surface.colorLocation[3] = GCM_LOCATION_RSX;
    surface.colorPitch[1] = surface.colorPitch[2] = surface.colorPitch[3] = 64;
    surface.depthFormat = GCM_SURFACE_ZETA_Z16;
    surface.depthLocation = GCM_LOCATION_RSX;
    surface.depthOffset = rsx_depth_offset;
    surface.depthPitch = rsx_depth_pitch;
    surface.type = GCM_SURFACE_TYPE_LINEAR;
    surface.antiAlias = GCM_SURFACE_CENTER_1;
    surface.width = rsx_width;
    surface.height = rsx_height;
    surface.x = surface.y = 0;
    rsxSetSurface(rsx_context, &surface);
}

int rsx_video_init(void *host_memory, u32 host_size)
{
    videoState state;
    videoConfiguration config;

    if (rsxInit(&rsx_context, RSX_CMD_BUFFER_SIZE, host_size, host_memory) != 0) {
        printf("PS3 prototype: rsxInit failed\n");
        return 0;
    }
    videoGetState(VIDEO_PRIMARY, 0, &state);
    videoGetResolution(state.displayMode.resolution, &rsx_resolution);

    memset(&config, 0, sizeof(config));
    config.resolution = state.displayMode.resolution;
    config.format = VIDEO_BUFFER_FORMAT_XRGB;
    config.pitch = rsx_resolution.width * sizeof(u32);
    wait_rsx_idle();
    videoConfigure(VIDEO_PRIMARY, &config, NULL, 0);
    gcmSetFlipMode(GCM_FLIP_VSYNC);

    rsx_width = rsx_resolution.width;
    rsx_height = rsx_resolution.height;
    rsx_color_pitch = rsx_width * sizeof(u32);
    rsx_framebuffer[0] = (u32 *)rsxMemalign(64, rsx_height * rsx_color_pitch);
    rsx_framebuffer[1] = (u32 *)rsxMemalign(64, rsx_height * rsx_color_pitch);
    if (!rsx_framebuffer[0] || !rsx_framebuffer[1]) {
        printf("PS3 prototype: framebuffer allocation failed\n");
        return 0;
    }
    rsxAddressToOffset(rsx_framebuffer[0], &rsx_color_offset[0]);
    rsxAddressToOffset(rsx_framebuffer[1], &rsx_color_offset[1]);
    gcmSetDisplayBuffer(0, rsx_color_offset[0], rsx_color_pitch, rsx_width, rsx_height);
    gcmSetDisplayBuffer(1, rsx_color_offset[1], rsx_color_pitch, rsx_width, rsx_height);

    rsx_depth_pitch = rsx_width * sizeof(u32);
    void *depth = rsxMemalign(64, rsx_height * rsx_depth_pitch * 2);
    if (!depth) {
        printf("PS3 prototype: depth buffer allocation failed\n");
        return 0;
    }
    rsxAddressToOffset(depth, &rsx_depth_offset);
    rsx_current_fb = 0;
    rsx_video_set_target(rsx_current_fb);
    return 1;
}

void rsx_video_flip(void)
{
    if (!rsx_context || !rsx_framebuffer[0] || !rsx_framebuffer[1])
        return;

    if (!rsx_first_flip) {
        while (gcmGetFlipStatus() != 0)
            usleep(1000);
        gcmResetFlipStatus();
    } else {
        gcmResetFlipStatus();
    }

    gcmSetFlip(rsx_context, rsx_current_fb);
    rsxFlushBuffer(rsx_context);
    gcmSetWaitFlip(rsx_context);

    rsx_current_fb ^= 1;
    rsx_video_set_target(rsx_current_fb);
    rsx_first_flip = 0;
}
