#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>

#include <string>

class Image {
public:
	Image(std::string filePath);
	~Image();

private:
	unsigned char * m_Data;
	uint32_t		m_Width, m_Height;
	uint32_t		m_Channels;
};

#endif
