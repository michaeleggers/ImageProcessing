#include "image.h"

#include <stdlib.h>

#include <SDL2/SDL.h>

#include "texture.h"
#include "common.h" 
#include "static_image_data.h"

#include "stb_image.h"

Image::Image(std::string filePath)
{	
	int x, y, n;
	unsigned char* data = stbi_load(filePath.c_str(), &x, &y, &n, 3);
	if (!data) {
		SDL_Log("Failed to load %s from disk! Init Image with placeholder data...\n", filePath.c_str());		
		Checkerboard cb = GetCheckerboard();
		m_Width = cb.width;
		m_Height = cb.height;
		m_Channels = cb.channels;
		m_Data = cb.data;		
		m_FilePath = "file not found!";
		m_IsCheckerboard = true;
	}
	else {
		size_t numBytes = x * y * n * sizeof(unsigned char);
		m_Data = (unsigned char*)malloc(numBytes);
		memcpy(m_Data, data, numBytes);
		m_Width = x;
		m_Height = y;
		m_Channels = n;
		m_IsCheckerboard = false;
		m_FilePath = filePath;
		stbi_image_free(data);
	}	
}

Image::Image(uint32_t width, uint32_t height, uint32_t channels)
{
	m_Width = width;
	m_Height = height;
	m_Channels = channels;
	m_Data = (unsigned char*)malloc(m_Width * m_Height * m_Channels * sizeof(unsigned char));
	m_IsCheckerboard = false;
	m_FilePath = "";
}

Image::Image(const Image& other)
{	
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_Channels = other.m_Channels;
	m_FilePath = other.m_FilePath;
	if (other.m_IsCheckerboard) {
		m_IsCheckerboard = true;
		m_Data = other.m_Data;
	}
	else if (other.m_Data) {		
		size_t numBytes = other.m_Width * other.m_Height * other.m_Channels;
		m_Data = (unsigned char*)malloc(numBytes);
		m_IsCheckerboard = false;
		memcpy(m_Data, other.m_Data, numBytes);		
	}
	else {
		m_IsCheckerboard = false;
		this->m_Data = nullptr;
	}
}

Image::~Image()
{
	if (m_Data != nullptr && !m_IsCheckerboard) {
		free(m_Data);
	}
}

void Image::Destroy()
{
	if (m_Data && !m_IsCheckerboard) {
		free(m_Data);
	}	
	m_Data = nullptr;
}

Image Image::Blend(Image& a, Image& b, float pct)
{
	Image result(a.m_Width, a.m_Height, a.m_Channels);
	for (uint32_t y = 0; y < a.m_Height; y++) {
		for (uint32_t x = 0; x < a.m_Width; x++) {
			glm::ivec3 pixelA = a(x, y);
			glm::ivec3 pixelB = b(x, y);
			glm::ivec3 blendedPixel;
			blendedPixel.r = int((1.0f - pct) * pixelA.r + pct * pixelB.r);
			blendedPixel.g = int((1.0f - pct) * pixelA.g + pct * pixelB.g);
			blendedPixel.b = int((1.0f - pct) * pixelA.b + pct * pixelB.b);
			unsigned char* resultPixel = result.m_Data + (result.m_Channels * (y * result.m_Width + x));
			resultPixel[0] = blendedPixel.r;
			resultPixel[1] = blendedPixel.g;
			resultPixel[2] = blendedPixel.b;
		}
	}	

	return result;
}

// Takes an image and converts it to RGBA. A is set to 255. 
Image Image::ToRGBA(Image& image)
{
	Image result(image.m_Width, image.m_Height, 4);
	for (uint32_t y = 0; y < image.m_Height; y++) {
		for (uint32_t x = 0; x < image.m_Width; x++) {
			glm::ivec3 pixel = image(x, y);
			unsigned char* resultPixel = result.m_Data + (result.m_Channels * (y * result.m_Width + x));
			resultPixel[0] = pixel.r;
			resultPixel[1] = pixel.g;
			resultPixel[2] = pixel.b;
			resultPixel[3] = 255;
		}
	}
	
	return result;
}


