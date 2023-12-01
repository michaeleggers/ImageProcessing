#include "texture.h"

#include <stdio.h>

#include <glad/glad.h>

Texture::Texture()
{
    m_Width = 0;
    m_Height = 0;
    m_Texture = 0;
}

Texture::Texture(unsigned char* data, uint32_t width, uint32_t height)
{
    m_Width = width;
    m_Height = height;

    glGenTextures(1, &m_Texture);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{ 
    // Destroy(); // TODO: Copy ctor in fbo ctor won't work
}

void Texture::Destroy()
{
    glDeleteTextures(1, &m_Texture);
}

GLuint Texture::GetHandle()
{
    return m_Texture;
}
