#include "image.h"

#include <SDL2/SDL.h>

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
}

Image::~Image()
{
	stbi_image_free(m_Data);
}


