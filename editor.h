#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <vector>

#include <glad/glad.h>

#include "imgui.h"

#include "batch.h"
#include "shader.h"
#include "fbo.h"
#include "image.h"
#include "render_common.h"

enum EditorWindowType {
	ED_WINDOW_TYPE_SOURCE,
	ED_WINDOW_TYPE_DEST
};

enum EditorState {
	ED_IDLE,	
	ED_PLACE_SOURCE_LINE,
	ED_PLACE_DEST_LINE_1,
	ED_PLACE_DEST_LINE_2
};

enum EditorMouseState {
	ED_MOUSE_IDLE,
	ED_MOUSE_CLICKED_ONCE,
	ED_MOUSE_CLICKED_TWICE
};

struct EditorMouseInfo {
	ImVec2 pos1;
	ImVec2 pos2;
};

void ShowWindow(const char* title, Framebuffer& fbo, Shader& shader, Image& image, Batch& batch, std::vector<Line>& lines, EditorWindowType windowType);

#endif

