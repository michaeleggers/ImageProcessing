#include "texture.h"

#include <stdio.h>

#include <glad/glad.h>

Texture::Texture()
{
    m_Width = 0;
    m_Height = 0;
    m_GLTextureHandle = 0;
    m_isOK = false;
}

Texture::Texture(unsigned char* data, uint32_t width, uint32_t height)
{
    m_Width = width;
    m_Height = height;

    glGenTextures(1, &m_GLTextureHandle);
    glBindTexture(GL_TEXTURE_2D, m_GLTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLuint)width, (GLuint)height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_isOK = true;
}

Texture::~Texture()
{ 
    printf("Want to destroy texture\n");
    Destroy(); // TODO: Copy ctor in fbo ctor won't work
}

void Texture::Bind()
{
    glBindTexture(GL_TEXTURE_2D, m_GLTextureHandle);
}

void Texture::Unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Destroy()
{
    if (m_isOK) {
        glDeleteTextures(1, &m_GLTextureHandle);
    }
}

GLuint Texture::GetHandle()
{
    return m_GLTextureHandle;
}
