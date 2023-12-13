#ifndef _STATIC_IMAGE_DATA_H_
#define _STATIC_IMAGE_DATA_H_

#include <stdint.h>

struct Checkerboard {
	int width, height, channels;
	uint8_t* data;
}; 

void InitCheckerboardTexture(int width, int height, int channels);
Checkerboard GetCheckerboard();

#endif