#ifndef _RENDER_H_
#define _RENDER_H_

#include <string>

#include <glad/glad.h>

#include <glm/glm.hpp>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;
};

struct Rect {
	glm::vec2 v1, v2, v3, v4;
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

