#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <stdint.h>

#include <glad/glad.h>

class Texture {
public:
	Texture();
	Texture(unsigned char* data, uint32_t width, uint32_t height);

	// Delete copy and move Ctors to prevent calling the Destructor.
	Texture(const Texture& other) = delete;
	Texture& operator=(const Texture& other) = delete;

	// Implement move constructor and move assignment
	Texture(Texture&& other) noexcept {
		m_GLTextureHandle = other.m_GLTextureHandle;
		m_Width = other.m_Width;
		m_Height = other.m_Height;
		m_isOK = other.m_isOK;
		other.m_GLTextureHandle = 0; // Set the moved-from object's handle to 0 to avoid deleting it		
	}

	Texture& operator=(Texture&& other) noexcept {
		if (this != &other) {
			// Release current resources
			glDeleteTextures(1, &m_GLTextureHandle);

			// Move resources from 'other' to this object
			m_GLTextureHandle = other.m_GLTextureHandle;
			m_Width = other.m_Width;
			m_Height = other.m_Height;
			m_isOK = other.m_isOK;
			other.m_GLTextureHandle = 0; // Set the moved-from object's handle to 0 to avoid deleting it		
		}

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

