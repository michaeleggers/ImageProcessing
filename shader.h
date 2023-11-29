#ifndef _RENDER_H_
#define _RENDER_H_

#include <string>

#include <glad/glad.h>

#include <glm/glm.hpp>

#include "render_common.h"

static const Rect rectGeoVerts[] = {
	{ glm::vec3(-0.5, 0.5, 0.0) },
	{ glm::vec3(0.5, 0.5, 0.0) },
	{ glm::vec3(0.5, -0.5, 0.0) },
	{ glm::vec3(-0.5, -0.5, 0.0) }
};

static const uint32_t rectGeoIndices[] = {
	0, 1, 2, // upper right tri
	2, 3, 0  // bottome left tri
};


class Shader {
public:
	Shader() {}
	~Shader() {
		Unload();
	}

	bool Load(const std::string& vertName, const std::string& fragName);
	void Unload();
	void Activate();

private:
	bool CompileShader(const std::string& fileName, GLenum shaderType, GLuint& outShader);
	bool IsCompiled(GLuint shader);
	bool IsValidProgram();

	GLuint m_VertexShader;
	GLuint m_FragmentShader;
	GLuint m_ShaderProgram;
};

#endif

