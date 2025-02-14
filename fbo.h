#ifndef _FBO_H_
#define _FBO_H_

#include <glad/glad.h>

#include "texture.h"

class Framebuffer {

public:
	Framebuffer() {
		m_Width = 0;
		m_Height = 0;
		m_FBO = 0;
	};
	Framebuffer(int width, int height);
	~Framebuffer();

	void Bind();
	void Unbind();
	void Resize(int width, int height);	

	Texture& GetTexture();

	int		m_Width;
	int		m_Height;

private:
	GLuint	m_FBO;
	Texture m_Texture;
};

#endif
