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


class Editor {
public:
	Editor(Image* sourceImage, Image* destImage);
	~Editor() {};

	void ShowWindow(const char* title, Image* image, Framebuffer* fbo, std::vector<Line>& lines, EditorWindowType windowType);
	void ShowResultWindow(const char* title);
	void Run();

private:
	// Hold a reference to source and dest images
	Image* m_sourceImage;
	Image* m_destImage;

	// Beier-Neely weight parameters
	float m_A;
	float m_B;
	float m_P;

	int m_NumIterations;
	int m_MaxIterations;
	int m_ImageIndex;
	
	std::vector<Line> m_sourceLines;
	std::vector<Line> m_destLines;

	// Results
	std::vector<Image> m_sourceToDestMorphs;
	std::vector<Image> m_destToSourceMorphs;
	std::vector<Image> m_blendedImages;

	// Framebuffers for ImGUI windows to render our images into
	Framebuffer* m_sourceFBO; 
	Framebuffer* m_destFBO;
	Framebuffer* m_resultFBO;

	// Shader to render the images
	Shader m_imageShader;
};

#endif

