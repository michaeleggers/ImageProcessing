#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <glad/glad.h>

#include "batch.h"
#include "shader.h"
#include "fbo.h"

void ShowWindow(const char* title, Framebuffer& fbo, Shader& shader, Batch& batch);

#endif