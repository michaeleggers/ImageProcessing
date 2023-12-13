#include "static_image_data.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include <SDL2/SDL.h>

static Checkerboard checkerboard;

void InitCheckerboardTexture(int width, int height, int channels)
{
    if (checkerboard.data) {
        SDL_Log("Checkerboard texture already initialized. Do nothing...\n");
        return;
    }

    assert(channels >= 3 && channels <= 4 && "Checkerboard texture must have 3 or 4 channels!");    

    checkerboard.width = width;
    checkerboard.height = height;    
    checkerboard.data = (uint8_t*)malloc(width * height * channels);
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t index = (y * width + x) * channels;
            uint8_t blackOrWhite = ((y & 0x8) == 0 ^ (x & 0x8) == 0) * 255;
            checkerboard.data[index + 0] = blackOrWhite;
            checkerboard.data[index + 1] = blackOrWhite;
            checkerboard.data[index + 2] = blackOrWhite;
            if (channels == 4) {
                checkerboard.data[index + 3] = SDL_ALPHA_OPAQUE;
            }
        }
    }
}

Checkerboard GetCheckerboard()
{
    return checkerboard;
}

// TODO: Kill checkerboard texture

