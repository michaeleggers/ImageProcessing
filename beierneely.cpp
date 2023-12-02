#include "beierneely.h"

#include <stdint.h>

#include <vector>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include "image.h"
#include "render_common.h"

static float Distance(float u, float v, glm::vec2 P, glm::vec2 Q, glm::vec2 X) {
    if (0.0f < u && u < 1.0f)   return glm::abs(v);
    else if (u < 0.0f)          return glm::length(X - P);
    else                        return glm::length(X - Q); // u > 1.0f
}

static glm::vec2 Perpendicular(glm::vec2& a, glm::vec2& b) {
    glm::vec2 perpToB = glm::normalize(a - b);
    return glm::length(b) * perpToB;
}

std::vector<Image> BeierNeely(std::vector<Line>& sourceLines, std::vector<Line>& destLines, Image& sourceImage, Image& destImage, uint32_t iterations)
{
    std::vector<Image> result;

    // constants

    float a = 0.01f;
    float b = 1.42f;
    float p = 0.5;

    Image image(sourceImage.m_Width, sourceImage.m_Height, 3); // TODO: Check for channels and handle correctly

    for (int y = 0; y < destImage.m_Height; y++) {
        for (int x = 0; x < destImage.m_Width; x++) { // Go through all pixels in destImages
            glm::vec2 X = glm::vec2(x, y);
            glm::vec2 DSUM = glm::vec2(0.0, 0.0);
            float weightsum = 0;
            for (int i = 0; i < destLines.size(); i++) {
                Line& destLine = destLines[i];
                Line& srcLine = sourceLines[i];
                glm::vec2 P = destLine.a.pos;
                glm::vec2 Q = destLine.b.pos;
                glm::vec2 srcP = srcLine.a.pos;
                glm::vec2 srcQ = srcLine.b.pos;
                glm::vec2 PX = X - P;
                glm::vec2 PQ = Q - P;
                float PQlength = glm::length(PQ);
                float u = glm::dot(PX, PQ) / (PQlength * PQlength);
                float v = glm::dot(PX, Perpendicular(X, u*PQ)) / PQlength;
                glm::vec2 srcPQ = srcQ - srcP;
                glm::vec2 srcX = srcP + u * srcPQ + (v * Perpendicular(X, u * srcPQ) / glm::length(srcPQ));
                glm::vec2 D = srcX - X;
                float dist = Distance(u, v, P, Q, X);
                float weight = glm::pow( glm::pow(PQlength, p) / (a + dist), b );
                DSUM += D * weight;
                weightsum += weight;
            }
            glm::vec2 srcX = X + DSUM / weightsum;
            glm::ivec3 destPixel = destImage(x, y);
            unsigned char * newPixel = &image.m_Data[image.m_Channels*(y * image.m_Width + x)];
            newPixel[0] = destPixel.r;
            newPixel[1] = destPixel.g;
            newPixel[2] = destPixel.b;
            
        } // ! pixel row
    } // ! pixel col

    image.GetTexture().Destroy(); // TODO: Janky!
    image.CreateTexture();
    
    result.push_back(image);
    
    return result;
}

