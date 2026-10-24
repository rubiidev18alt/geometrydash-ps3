#ifndef GDPS3_PROTOTYPE_RSXUTIL_H
#define GDPS3_PROTOTYPE_RSXUTIL_H

#include <ppu-types.h>
#include <rsx/rsx.h>

#define RSX_CMD_BUFFER_SIZE 0x100000
#define RSX_HOST_SIZE (32 * 1024 * 1024)

extern gcmContextData *rsx_context;
extern u32 rsx_width;
extern u32 rsx_height;
extern u32 rsx_current_fb;
extern u32 *rsx_framebuffer[2];

int rsx_video_init(void *host_memory, u32 host_size);
void rsx_video_flip(void);
void rsx_video_set_target(u32 index);

#endif
