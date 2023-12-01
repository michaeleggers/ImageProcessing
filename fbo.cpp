#include "fbo.h"

#include <assert.h>

#include <SDL2/SDL.h>


#include "texture.h"

Framebuffer::Framebuffer(int width, int height)
{
    assert(width > 0 && height > 0);

    m_Width = width;
    m_Height = height;

    // Create a FBO (yay!)    
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    
    // Framebuffer texture    

    m_Texture = Texture(nullptr, width, height);

    // Attach texture to fbo 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture.GetHandle(), 0);
    // Check if fbo is OK and unbind
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SDL_Log("Failed to create OpenGL Framebuffer object!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &m_FBO);
}

void Framebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

void Framebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(int width, int height)
{
    assert(width > 0 && height > 0);

    m_Width = width;
    m_Height = height;

    m_Texture.Destroy();
    Bind();
    m_Texture = Texture(nullptr, width, height);    
    // Attach texture to fbo 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture.GetHandle(), 0);
    Unbind();
}

Texture& Framebuffer::GetTexture()
{
    return m_Texture;
}
