#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <vector>

#include <glad/glad.h>

#include "batch.h"
#include "shader.h"
#include "fbo.h"
#include "image.h"
#include "render_common.h"

void ShowWindow(const char* title, Framebuffer& fbo, Shader& shader, Image& image, Batch& batch, std::vector<Line> lines);

#endif

