#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <stdint.h>

#include <glad/glad.h>

class Texture {
public:
	Texture();
	Texture(unsigned char* data, uint32_t width, uint32_t height);
	~Texture();

	void Bind();
	void Unbind();

	void Destroy();

	GLuint GetHandle();

private:
	GLuint m_Texture;
	uint32_t m_Width, m_Height;
};

#endif

