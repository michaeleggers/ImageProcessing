#ifndef _BEIER_NEELY_H_
#define _BEIER_NEELY_H_

#include <stdint.h>

#include <vector>

#include "image.h"
#include "render_common.h"

std::vector<Image> BeierNeely(
	std::vector<Line>& sourceLines, std::vector<Line>& destLines, 
	Image& sourceImage, Image& destImage, 
	uint32_t iterations);

#endif

