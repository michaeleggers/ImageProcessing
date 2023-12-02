#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>

#include <string>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include "texture.h"

class Image {
public:
	Image(std::string filePath);
	Image(uint32_t width, uint32_t height, uint32_t channels);
	~Image();

	Texture& GetTexture();
	void CreateTexture();

	const glm::ivec3& operator()(size_t x, size_t y) {
		unsigned char* pixel = m_Data + ((y * m_Width + x) * m_Channels);
		return glm::ivec3(pixel[0], pixel[1], pixel[2]);		
	}

	uint32_t		m_Width, m_Height;
	uint32_t		m_Channels;
	unsigned char * m_Data;
private:
	Texture			m_Texture;
};

#endif
