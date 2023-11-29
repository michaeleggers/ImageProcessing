#ifndef _FBO_H_
#define _FBO_H_

#include <glad/glad.h>

class Framebuffer {

public:
	Framebuffer(int width, int height);
	~Framebuffer();

	void Bind();
	void Unbind();
	void Resize(int width, int height);
	GLuint Texture() const;

private:
	GLuint m_FBO;
	GLuint m_Texture;
	int m_Width;
	int m_Height;
};

#endif