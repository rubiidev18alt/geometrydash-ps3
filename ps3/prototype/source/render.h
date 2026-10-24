#ifndef GDPS3_PROTOTYPE_RENDER_H
#define GDPS3_PROTOTYPE_RENDER_H

#include <stdint.h>
#include "game.h"

void render_game(uint32_t *framebuffer, unsigned width, unsigned height,
                 const GameState *game);

#endif
