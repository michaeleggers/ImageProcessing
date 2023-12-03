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

static glm::vec2 Perpendicular(glm::vec2& a) {
    glm::vec2 perp = glm::vec2(a.y, a.x);
    perp.x *= -1.0;
    perp = glm::normalize(perp);

    return glm::length(a) * perp;
}

std::vector<Image> BeierNeely(std::vector<Line>& sourceLines, std::vector<Line>& destLines, Image& sourceImage, Image& destImage, uint32_t iterations)
{
    std::vector<Image> result;

    // constants

    float a = 0.01;
    float b = 1.5;
    float p = 0.01;

    for (int iter = 0; iter < iterations; iter++) {
        float pct = (float)iter / (float)iterations;
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
                    float v = glm::dot(PX, Perpendicular(PQ)) / PQlength;
                    glm::vec2 srcPQ = srcQ - srcP;
                    glm::vec2 srcX = srcP + u * srcPQ + (v * Perpendicular(srcPQ) / glm::length(srcPQ));
                    glm::vec2 D = pct * (srcX - X); // Interpolate here over time!
                    float dist = Distance(u, v, P, Q, X);
                    float weight = glm::pow(glm::pow(PQlength, p) / (a + dist), b);
                    DSUM += D * weight;
                    weightsum += weight;
                }
                glm::vec2 srcX = X + DSUM / weightsum;
                if (srcX.x > destImage.m_Height - 1) srcX.x = (destImage.m_Height-1);
                if (srcX.y > destImage.m_Height - 1) srcX.y = (destImage.m_Height-1);
                if (srcX.x < 0) srcX.x = 0;
                if (srcX.y < 0) srcX.y = 0;

                // if (srcX.x < 0 || srcX.y < 0) printf("srcX negative!\n");

                glm::ivec3 sourcePixel = sourceImage(srcX.x, srcX.y);
                glm::ivec3 destPixel = destImage(x, y);
                unsigned char* newPixel = image.m_Data + (image.m_Channels * (y * image.m_Width + x));
                newPixel[0] = sourcePixel.r;
                newPixel[1] = sourcePixel.g;
                newPixel[2] = sourcePixel.b;

            } // ! pixel row
        } // ! pixel col

        image.GetTexture().Destroy(); // TODO: Janky!
        image.CreateTexture();

        result.push_back(image);
    }

    
    
    return result;
}

