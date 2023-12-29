#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <stdint.h>

#include <glad/glad.h>

class Texture {
public:
	Texture();
	Texture(unsigned char* data, uint32_t width, uint32_t height);
	Texture& operator=(const Texture& other) {
		this->m_GLTextureHandle = other.m_GLTextureHandle;
		this->m_Width = other.m_Width;
		this->m_Height = other.m_Height;
		this->m_isOK = other.m_isOK;

		return *this;
	}

	~Texture();

	void Bind();
	void Unbind();

	void Destroy();

	GLuint GetHandle();

	uint32_t m_Width, m_Height;
	bool m_isOK;

private:
	GLuint m_GLTextureHandle;
};

#endif

