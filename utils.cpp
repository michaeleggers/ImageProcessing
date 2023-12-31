#include "utils.h"


// NOTE: Images will allways be 24bits/pixel!
std::vector<Image> CreateImagesFromTexture(std::vector<Texture>& textures)
{
    std::vector<Image> results;

    for (auto& texture : textures) {
        Image image = Image(texture.m_Width, texture.m_Height, 3, texture.m_Data);
        results.push_back(image);
    }

    return results;
}
