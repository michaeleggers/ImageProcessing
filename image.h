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
	Image() {
		m_Width = 0;
		m_Height = 0;
		m_Channels = 0;
		m_Data = nullptr;
		m_IsCheckerboard = false;		
	};
	Image(std::string filePath);
	Image(uint32_t width, uint32_t height, uint32_t channels);
	Image(uint32_t width, uint32_t height, uint32_t channels, unsigned char* data);
	Image(const Image& other);
	~Image();

	// Copy assignment
	Image& operator=(const Image& other) {
		if (this != &other) { // Check for self-assignment
			this->m_Width = other.m_Width;
			this->m_Height = other.m_Height;
			this->m_Channels = other.m_Channels;
			this->m_FilePath = other.m_FilePath;
			if (other.m_IsCheckerboard) {
				if (this->m_Data) free(this->m_Data);
				this->m_IsCheckerboard = true;
				this->m_Data = other.m_Data;
			}
			if (other.m_Data) {
				if (this->m_Data) free(this->m_Data);
				size_t numBytes = other.m_Width * other.m_Height * other.m_Channels;
				this->m_Data = (unsigned char*)malloc(numBytes);
				memcpy(this->m_Data, other.m_Data, numBytes);
			}
		}

		return *this;
	}

	// Move assignment
	Image& operator=(Image&& other) noexcept {
		if (this != &other) {
			free(m_Data);
			m_Data = nullptr;			
			this->m_Width = other.m_Width;
			this->m_Height = other.m_Height;
			this->m_Channels = other.m_Channels;
			this->m_FilePath = other.m_FilePath;
			if (other.m_IsCheckerboard) {
				if (this->m_Data) free(this->m_Data);
				this->m_IsCheckerboard = true;			
			}
			std::swap(m_Data, other.m_Data);
		}

		return *this;
	}

	void Destroy();		

	const glm::ivec3 operator()(uint32_t x, uint32_t y) {
		unsigned char* pixel = m_Data + ((y * m_Width + x) * m_Channels);
		return glm::ivec3(pixel[0], pixel[1], pixel[2]);		
	}

	static Image Blend(Image& a, Image& b, float pct);
	static Image ToRGBA(Image& image);

	uint32_t		m_Width, m_Height;
	uint32_t		m_Channels;
	unsigned char * m_Data;
	bool			m_IsCheckerboard;
	std::string		m_FilePath;

private:
	
};

#endif
