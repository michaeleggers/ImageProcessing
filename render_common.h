#ifndef _RENDER_COMMON_H_
#define _RENDER_COMMON_H_

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include "imgui.h"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;
};

struct Rect {
	Vertex v1, v2, v3, v4;
};

// TODO: Move this to somewhere else
struct Line {
	Vertex a, b;
	ImVec2 absA, absB;
	ImVec2 buttonSize;
};

#endif
