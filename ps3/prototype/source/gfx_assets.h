#ifndef GDPS3_PROTOTYPE_GFX_ASSETS_H
#define GDPS3_PROTOTYPE_GFX_ASSETS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *rgba;
    uint16_t width;
    uint16_t height;
} GfxTexture;

typedef struct {
    GfxTexture cube_primary;
    GfxTexture cube_secondary;
    GfxTexture block;
    GfxTexture spike;
    GfxTexture background;
    GfxTexture ground;
    uint8_t *owned_blob;
} PrototypeAssets;

bool prototype_assets_from_blob(PrototypeAssets *assets, const uint8_t *blob, size_t size);
bool prototype_assets_load_file(PrototypeAssets *assets, const char *path);
bool prototype_assets_load_embedded(PrototypeAssets *assets);
void prototype_assets_free(PrototypeAssets *assets);

#endif
