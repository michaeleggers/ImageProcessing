#include "image.h"

#include <SDL2/SDL.h>

#include "texture.h"

#include "stb_image.h"

Image::Image(std::string filePath)
{
	int x, y, n;
	m_Data = stbi_load(filePath.c_str(), &x, &y, &n, 3);
	if (!m_Data) {
		SDL_Log("Failed to load %s from disk! Abort.\n", filePath.c_str());
		exit(-1);
	}
	m_Width = x;
	m_Height = y;
	m_Channels = n;
	m_Texture = Texture(m_Data, m_Width, m_Height);
}

Image::Image(uint32_t width, uint32_t height, uint32_t channels)
{
	m_Width = width;
	m_Height = height;
	m_Channels = channels;
	m_Data = (unsigned char*)malloc(m_Width * m_Height * m_Channels * sizeof(unsigned char));
	m_Texture = Texture(nullptr, m_Width, m_Height);
}

Image::~Image()
{
	//stbi_image_free(m_Data);  // TODO: We have to check if mem was allocated by stb_image or c-runtime
	//m_Texture.Destroy(); // Assignment operator missing. This kills the texture on Construction and Assignment! TODO: FIX!!!
}

Texture& Image::GetTexture()
{
	return m_Texture;
}

void Image::CreateTexture()
{
	m_Texture = Texture(m_Data, m_Width, m_Height);
}

Image Image::Blend(Image& a, Image& b, float pct)
{
	Image result(a.m_Width, a.m_Height, a.m_Channels);
	for (int y = 0; y < a.m_Height; y++) {
		for (int x = 0; x < a.m_Width; x++) {
			glm::ivec3 pixelA = a(x, y);
			glm::ivec3 pixelB = b(x, y);
			glm::ivec3 blendedPixel;
			blendedPixel.r = (1.0f - pct) * pixelA.r + pct * pixelB.r;
			blendedPixel.g = (1.0f - pct) * pixelA.g + pct * pixelB.g;
			blendedPixel.b = (1.0f - pct) * pixelA.b + pct * pixelB.b;
			unsigned char* resultPixel = result.m_Data + (result.m_Channels * (y * result.m_Width + x));
			resultPixel[0] = blendedPixel.r;
			resultPixel[1] = blendedPixel.g;
			resultPixel[2] = blendedPixel.b;
		}
	}
	result.CreateTexture();

	return result;
}


