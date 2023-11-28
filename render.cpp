#include "render.h"
#include "common.h"

#include <string>

#include <SDL2/SDL.h>

// Interface

bool Shader::Load(const std::string& vertName, const std::string& fragName) {
	if (!CompileShader(vertName, GL_VERTEX_SHADER, m_VertexShader)
		|| !CompileShader(fragName, GL_FRAGMENT_SHADER, m_FragmentShader)) {

		return false;
	}

	m_ShaderProgram = glCreateProgram();
	glAttachShader(m_ShaderProgram, m_VertexShader);
	glAttachShader(m_ShaderProgram, m_FragmentShader);
	glLinkProgram(m_ShaderProgram);

	if (!IsValidProgram()) return false;

	return true;
}

void Shader::Unload() {
	glDeleteProgram(m_ShaderProgram);
	glDeleteShader(m_VertexShader);
	glDeleteShader(m_FragmentShader);
}

void Shader::Activate() {
	glUseProgram(m_ShaderProgram);
}

// Internal

bool Shader::CompileShader(const std::string& fileName, GLenum shaderType, GLuint& outShader) {
	ATP_File shaderCode;
	if (atp_read_file(fileName.c_str(), &shaderCode) != ATP_SUCCESS) {
		SDL_Log("Could not read file: %s!\n", fileName.c_str());
		return false;
	}

	outShader = glCreateShader(shaderType);
	glShaderSource(outShader, 1, (GLchar**)(&shaderCode.data), nullptr);
	glCompileShader(outShader);

	if (!IsCompiled(outShader)) {
		SDL_Log("Failed to compile shader: %s\n", fileName.c_str());
		return false;
	}

	return true;
}

bool Shader::IsCompiled(GLuint shader) {
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		memset(buffer, 0, 512);
		glGetShaderInfoLog(shader, 511, nullptr, buffer);
		SDL_Log("GLSL compile error:\n%s\n", buffer);

		return false;
	}

	return true;
}

bool Shader::IsValidProgram() {
	GLint status;
	glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		memset(buffer, 0, 512);
		glGetProgramInfoLog(m_ShaderProgram, 511, nullptr, buffer);
		SDL_Log("GLSL compile error:\n%s\n", buffer);

		return false;
	}

	return true;
}




