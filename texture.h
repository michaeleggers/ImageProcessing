#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>

class Texture {
public:
	Texture();
	Texture(unsigned char* data, uint32_t width, uint32_t height);
	
	Texture(const Texture& other) {
		m_Width = other.m_Width;
		m_Height = other.m_Height;

		glGenTextures(1, &m_GLTextureHandle);
		glBindTexture(GL_TEXTURE_2D, m_GLTextureHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLuint)m_Width, (GLuint)m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, other.m_Data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		if (m_Data) {
			free(m_Data);
		}
		if (other.m_Data) {
			m_Data = (unsigned char*)malloc(m_Width * m_Height * 3);
			memcpy(m_Data, other.m_Data, m_Width * m_Height * 3);
		}
		else {
			m_Data = nullptr;
		}
	}
	
	Texture& operator=(const Texture& other) noexcept {

		m_Width = other.m_Width;
		m_Height = other.m_Height;

		glGenTextures(1, &m_GLTextureHandle);
		glBindTexture(GL_TEXTURE_2D, m_GLTextureHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLuint)m_Width, (GLuint)m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, other.m_Data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		if (m_Data) {
			free(m_Data);
		}
		if (other.m_Data) {
			m_Data = (unsigned char*)malloc(m_Width * m_Height * 3);
			memcpy(m_Data, other.m_Data, m_Width * m_Height * 3);
		}
		else {
			m_Data = nullptr;
		}

		return *this;
	}

	~Texture();

	void Bind();
	void Unbind();

	void Destroy();

	GLuint GetHandle();

	uint32_t m_Width, m_Height;
	unsigned char* m_Data = nullptr;

private:
	GLuint m_GLTextureHandle;
};

#endif

