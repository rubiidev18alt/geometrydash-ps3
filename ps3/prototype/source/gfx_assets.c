#include "gfx_assets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const uint8_t gdps3_asset_blob_start[];
extern const uint8_t gdps3_asset_blob_end[];

#define ASSET_COUNT 6
#define BUNDLE_HEADER_SIZE 8
#define IMAGE_HEADER_SIZE 8

static uint16_t read_u16le(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t read_u32le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

bool prototype_assets_from_blob(PrototypeAssets *assets, const uint8_t *blob, size_t size)
{
    GfxTexture *textures[ASSET_COUNT];
    size_t offset = BUNDLE_HEADER_SIZE;

    if (!assets || !blob || size < BUNDLE_HEADER_SIZE || memcmp(blob, "P3TX", 4) != 0 ||
        read_u16le(blob + 4) != 1 || read_u16le(blob + 6) != ASSET_COUNT)
        return false;

    memset(assets, 0, sizeof(*assets));
    textures[0] = &assets->cube_primary;
    textures[1] = &assets->cube_secondary;
    textures[2] = &assets->block;
    textures[3] = &assets->spike;
    textures[4] = &assets->background;
    textures[5] = &assets->ground;

    for (unsigned i = 0; i < ASSET_COUNT; i++) {
        uint16_t width, height;
        uint32_t bytes;
        size_t expected;

        if (size - offset < IMAGE_HEADER_SIZE)
            return false;
        width = read_u16le(blob + offset);
        height = read_u16le(blob + offset + 2);
        bytes = read_u32le(blob + offset + 4);
        offset += IMAGE_HEADER_SIZE;
        if (!width || !height || (size_t)width > SIZE_MAX / 4 / height)
            return false;
        expected = (size_t)width * height * 4;
        if (expected != bytes || size - offset < bytes)
            return false;

        textures[i]->width = width;
        textures[i]->height = height;
        textures[i]->rgba = blob + offset;
        offset += bytes;
    }

    return offset == size;
}

bool prototype_assets_load_file(PrototypeAssets *assets, const char *path)
{
    FILE *file;
    long length;
    uint8_t *blob;
    bool result;

    if (!assets || !path || !(file = fopen(path, "rb")))
        return false;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }
    blob = (uint8_t *)malloc((size_t)length);
    if (!blob) {
        fclose(file);
        return false;
    }
    if (fread(blob, 1, (size_t)length, file) != (size_t)length) {
        free(blob);
        fclose(file);
        return false;
    }
    fclose(file);

    result = prototype_assets_from_blob(assets, blob, (size_t)length);
    if (!result) {
        free(blob);
        return false;
    }
    assets->owned_blob = blob;
    return true;
}

bool prototype_assets_load_embedded(PrototypeAssets *assets)
{
    uintptr_t start = (uintptr_t)gdps3_asset_blob_start;
    uintptr_t end = (uintptr_t)gdps3_asset_blob_end;
    if (!assets || end <= start)
        return false;
    return prototype_assets_from_blob(assets, gdps3_asset_blob_start, (size_t)(end - start));
}

void prototype_assets_free(PrototypeAssets *assets)
{
    if (!assets)
        return;
    free(assets->owned_blob);
    memset(assets, 0, sizeof(*assets));
}
