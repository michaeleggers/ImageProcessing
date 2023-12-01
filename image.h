#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>

#include <string>

#include "texture.h"

class Image {
public:
	Image(std::string filePath);
	~Image();

	Texture& GetTexture();

	uint32_t		m_Width, m_Height;
private:
	unsigned char * m_Data;
	uint32_t		m_Channels;
	Texture			m_Texture;
};

#endif
