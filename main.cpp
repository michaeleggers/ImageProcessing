#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static bool keys[SDL_NUM_SCANCODES] = { false };

#define CHECKERBOARD_WIDTH    64
#define CHECKERBOARD_HEIGHT   64
#define CHECKERBOARD_CHANNELS 4
#define CHECKERBOARD_SIZE     (CHECKERBOARD_WIDTH * CHECKERBOARD_HEIGHT)
#define CHECKERBOARD_BYTES    (CHECKERBOARD_SIZE * CHECKERBOARD_CHANNELS)
#define MAX_CHECKERBOARD_INDEX ((CHECKERBOARD_HEIGHT*CHECKERBOARD_WIDTH + CHECKERBOARD_WIDTH)*4)
uint8_t checkerboard[CHECKERBOARD_BYTES];

const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;
const int RENDER_WIDTH =  800;
const int RENDER_HEIGHT = 600;

class Line {
    public:
    Line(glm::vec2 a, glm::vec2 b) {
        m_a = a;
        m_b = b;
    }
    glm::vec2 m_a;
    glm::vec2 m_b;

    void draw(SDL_Renderer * renderer) {
        SDL_RenderDrawLine(renderer, m_a.x, m_a.y, m_b.x, m_b.y);
    }
};

struct Tri {
    glm::vec4 p0, p1, p2; // position
    glm::vec3 c0, c1, c2; // color
    glm::vec2 t0, t1, t2; // texture
};

void initCheckerboardTexture() {
    for (size_t y=0; y < CHECKERBOARD_HEIGHT; y++) {
        for (size_t x=0; x < CHECKERBOARD_WIDTH; x++) {
            size_t index = (y*CHECKERBOARD_WIDTH + x) * 4;
            uint8_t blackOrWhite = ( (y&0x8)==0 ^ (x&0x8)==0 ) * 255;
            checkerboard[index + 0] = blackOrWhite;
            checkerboard[index + 1] = blackOrWhite;
            checkerboard[index + 2] = blackOrWhite;
            checkerboard[index + 3] = SDL_ALPHA_OPAQUE;
        }
    }
    printf("max tex index: %d\n", MAX_CHECKERBOARD_INDEX);
}

// Frontfaces are defined to be Counter-Clockwise.
// Input of triangle's vertices are in a y-down system.
static inline void rasterizeTri(SDL_Renderer * renderer, Tri triWithW, Tri tri) {
    int minX = (int)glm::min(glm::min(tri.p0.x, tri.p1.x), tri.p2.x);
    int maxX = (int)glm::max(glm::max(tri.p0.x, tri.p1.x), tri.p2.x);
    int minY = (int)glm::min(glm::min(tri.p0.y, tri.p1.y), tri.p2.y);
    int maxY = (int)glm::max(glm::max(tri.p0.y, tri.p1.y), tri.p2.y);
    
    // Clip to screen boundaries
    if (minX < 0) minX             = 0;
    if (maxX > RENDER_WIDTH) maxX  = RENDER_WIDTH;
    if (minY < 0) minY             = 0;
    if (maxY > RENDER_HEIGHT) maxY = RENDER_HEIGHT;

    glm::vec4 v0 = tri.p1 - tri.p0;
    glm::vec4 v1 = tri.p2 - tri.p1;
    glm::vec4 v2 = tri.p0 - tri.p2;
    glm::vec4 u = v0;
    glm::vec4 v = -v2;
    float recipW0 = 1.0f/triWithW.p0.w;
    float recipW1 = 1.0f/triWithW.p1.w;
    float recipW2 = 1.0f/triWithW.p2.w;
    float recipAreaParallelogram = 1.0f / (u.x*(v.y) - (u.y)*v.x);

    float C0 = -v0.x*tri.p0.y + v0.y*tri.p0.x;
    float C1 = -v1.x*tri.p1.y + v1.y*tri.p1.x;
    float C2 = -v2.x*tri.p2.y + v2.y*tri.p2.x;

    float Cy0 = C0 + v0.x*minY - v0.y*minX;
    float Cy1 = C1 + v1.x*minY - v1.y*minX;
    float Cy2 = C2 + v2.x*minY - v2.y*minX;

    if (recipAreaParallelogram <= 0) { // Face culling
        for (int y=minY; y < maxY; y++) {
            float Cx0 = Cy0;
            float Cx1 = Cy1;
            float Cx2 = Cy2;
            for (int x=minX; x < maxX; x++) {
                // As y is flipped (top->bottom) we check for <= instead of
                // >= !
                if (Cx0 <= 0 && Cx1 <= 0 && Cx2 <= 0) {
                    float s = Cx2 * recipAreaParallelogram;
                    float t = Cx0 * recipAreaParallelogram;

                    float recipW = (1.0f-s-t)*recipW0 + s*recipW1 + t*recipW2;
                    float oneOverRecipW = 1.0f / recipW;

                    glm::vec3 p0Color = (1.0f-s-t) * tri.c0 * recipW0;
                    glm::vec3 p1Color = s*tri.c1 * recipW1;
                    glm::vec3 p2Color = t*tri.c2 * recipW2;
                    glm::vec2 p0Tex = (1.0f-s-t) * tri.t0 * recipW0;
                    glm::vec2 p1Tex = s*tri.t1 * recipW1;
                    glm::vec2 p2Tex = t*tri.t2 * recipW2;
                    glm::vec3 finalColor = (p0Color + p1Color + p2Color)*oneOverRecipW;
                    uint8_t cR = finalColor.x*255;
                    uint8_t cG = finalColor.y*255;
                    uint8_t cB = finalColor.z*255;
                    glm::vec2 finalTexCoords = (p0Tex + p1Tex + p2Tex)*oneOverRecipW;
                    int uTexSampleX = CHECKERBOARD_WIDTH*finalTexCoords.x;
                    int uTexSampleY = CHECKERBOARD_HEIGHT*finalTexCoords.y;
                    int texIndex = CHECKERBOARD_CHANNELS*(uTexSampleY*CHECKERBOARD_WIDTH + uTexSampleX);
                    if (texIndex > MAX_CHECKERBOARD_INDEX) {
                        printf("tex index: %d\n", texIndex);
                    }
                    uint8_t texR = checkerboard[texIndex + 0];
                    uint8_t texG = checkerboard[texIndex + 1];
                    uint8_t texB = checkerboard[texIndex + 2];
                    uint8_t texA = checkerboard[texIndex + 3];
                    // // printf("r: %f, g: %f, b: %f\n", finalColor.x, finalColor.y, finalColor.z);

                    uint8_t mixR = glm::max((cR + texR), 255);
                    uint8_t mixG = glm::max((cG + texG), 255);
                    uint8_t mixB = glm::max((cB + texB), 255);

                    // SDL_SetRenderDrawColor(renderer, finalColor.x*255, finalColor.y*255, finalColor.z*255, SDL_ALPHA_OPAQUE);
                    SDL_SetRenderDrawColor(renderer, mixR, mixG, mixB, SDL_ALPHA_OPAQUE);
                    SDL_RenderDrawPoint(renderer, x, y);
                }

                Cx0 -= v0.y;
                Cx1 -= v1.y;
                Cx2 -= v2.y;
            }

            Cy0 += v0.x;
            Cy1 += v1.x;
            Cy2 += v2.x;
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect triBoundaries = {minX, minY, maxX-minX, maxY-minY};
    // SDL_RenderDrawRect(renderer, &triBoundaries);

    SDL_Point points[] = { {tri.p0.x, tri.p0.y}, {tri.p1.x, tri.p1.y}, {tri.p2.x, tri.p2.y}, {tri.p0.x, tri.p0.y} };
    SDL_SetRenderDrawColor(renderer, 255, 245, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLines(renderer, points, 4);
}

static inline Tri applyMatToTri(glm::mat4 mat, Tri tri) {
    Tri result = tri;
    result.p0 = mat * tri.p0;
    result.p1 = mat * tri.p1;
    result.p2 = mat * tri.p2;
    
    return result;
}

static inline void applyPerspDivide(Tri& tri) {
    tri.p0 /= tri.p0.w;
    tri.p1 /= tri.p1.w;
    tri.p2 /= tri.p2.w;
}

static inline void applyViewportTransformation(Tri& tri, int x, int y, int width, int height) {
    tri.p0.x = (tri.p0.x + 1) * (width / 2.0f) + x;
    tri.p0.y = (tri.p0.y + 1) * (height / 2.0f) + y;
    tri.p1.x = (tri.p1.x + 1) * (width / 2.0f) + x;
    tri.p1.y = (tri.p1.y + 1) * (height / 2.0f) + y;
    tri.p2.x = (tri.p2.x + 1) * (width / 2.0f) + x;
    tri.p2.y = (tri.p2.y + 1) * (height / 2.0f) + y;
}

static inline float randBetween(float min, float max) {
    float randValue = (float)rand()/(float)RAND_MAX;
    float range = max - min;
    return range/1.0f * randValue + min;
}

void updateKeys(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.scancode == SDL_SCANCODE_UP) {
            keys[SDL_SCANCODE_UP] = true;
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_DOWN) {
            keys[SDL_SCANCODE_DOWN] = true;

        }
        if (e.key.keysym.scancode == SDL_SCANCODE_LEFT) {
            keys[SDL_SCANCODE_LEFT] = true;
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
            keys[SDL_SCANCODE_RIGHT] = true;
        }

        if (e.key.keysym.scancode == SDL_SCANCODE_W) {
            keys[SDL_SCANCODE_W] = true;
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_S) {
            keys[SDL_SCANCODE_S] = true;
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_A) {
            keys[SDL_SCANCODE_A] = true;
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_D) {
            keys[SDL_SCANCODE_D] = true;
        }
    }
    else if (e.type == SDL_KEYUP) {
        if (e.key.keysym.scancode == SDL_SCANCODE_UP) {
            keys[SDL_SCANCODE_UP] = false;
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_DOWN) {
            keys[SDL_SCANCODE_DOWN] = false;

        }
        if (e.key.keysym.scancode == SDL_SCANCODE_LEFT) {
            keys[SDL_SCANCODE_LEFT] = false;
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
            keys[SDL_SCANCODE_RIGHT] = false;
        }

        if (e.key.keysym.scancode == SDL_SCANCODE_W) {
            keys[SDL_SCANCODE_W] = false;
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_S) {
            keys[SDL_SCANCODE_S] = false;
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_A) {
            keys[SDL_SCANCODE_A] = false;
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_D) {
            keys[SDL_SCANCODE_D] = false;
        }
    }
}

void updateCamera(Camera& camera) {
    if (keys[SDL_SCANCODE_UP]) {
        camera.RotateAroundSide(.005f);
    }
    if (keys[SDL_SCANCODE_DOWN]) {
        camera.RotateAroundSide(-.005f);
    }
    if (keys[SDL_SCANCODE_LEFT]) {
        camera.RotateAroundUp(.005f);        
    }      
    if (keys[SDL_SCANCODE_RIGHT]) {
        camera.RotateAroundUp(-.005f);
    }
    if (keys[SDL_SCANCODE_W]) {
        camera.MoveForward(.5f);
    }
    if (keys[SDL_SCANCODE_S]) {
        camera.MoveForward(-.5f);
    }
    if (keys[SDL_SCANCODE_A]) {
        camera.MoveSide(.5f);
    }
    if (keys[SDL_SCANCODE_D]) {
        camera.MoveSide(-.5f);
    }
}

int main (int argc, char ** argv) 
{

    // Load image of which to compute DFT
    if (argc < 2) {
        printf("Usage:\n dft <image-file>\n");
        return -1;
    }
    char* imgName = argv[1];
    printf("Loading image: %s\n", imgName);

    int x,y,n;
    unsigned char *data = stbi_load(imgName, &x, &y, &n, 0);
    if (!data) {
        printf("Failed to load %s! Abort!\n", imgName);
        return -1;
    }

    initCheckerboardTexture();

    SDL_Window *window;                    // Declare a pointer

    SDL_Init(SDL_INIT_EVERYTHING);              // Initialize SDL2

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "DFT",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        WINDOW_WIDTH,                               // width, in pixels
        WINDOW_HEIGHT,                               // height, in pixels
        SDL_WINDOW_RESIZABLE                 // flags - see below
    );

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        printf("Failed to init renderer. Bye!\n");
        return 1;
    }
    SDL_Texture *renderTexture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, 
        RENDER_WIDTH, RENDER_HEIGHT);

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_ShowWindow(window);

    // Test line
    Line line = Line(glm::vec2(10, 10), glm::vec2(600, 600));

    Camera camera = Camera(glm::vec3(0, 0, 20));

    // Main loop
    SDL_Event event;
    bool shouldClose = 0;
    float accumTime = 0.0f;
    while (!shouldClose) {

        Uint32 startTime = SDL_GetTicks();

        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldClose = true;
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                shouldClose = true;
            }
            updateKeys(event);
        }

        updateCamera(camera);


        SDL_SetRenderTarget(renderer, renderTexture);
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);


        // Transform from Worldspace to Viewspace
        // TODO: The viewing mat is janky at the moment because (I think) there is no clipping yet!
        // glm::mat4 viewMat = glm::lookAt(glm::vec3(0, -15, 3.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 viewMat = glm::lookAt(camera.m_Pos, camera.m_Center, camera.m_Up);

        // Transform to Clipspace using projective transformation
        // TODO: Be careful with the FOV. There is no clipping yet. A narrow FOV moves the near clipping plane
        //       farther away from the cam. The image may appear upside down!
        int windowWidth, windowHeight;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        glm::mat4 perspProjMat = glm::perspective(45.0f, (float)windowWidth/(float)windowHeight, 0.1f, 2000.0f);


        SDL_SetRenderDrawColor(renderer, 255, 10, 90, SDL_ALPHA_OPAQUE);

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, renderTexture, NULL, NULL); 

        SDL_RenderPresent(renderer);

        Uint32 endTime = SDL_GetTicks();
        Uint32 timePassed = endTime - startTime;
        float timePassedSeconds = (float)timePassed / 1000.0f;
        float FPS = 1.0f / timePassedSeconds;
        accumTime += timePassedSeconds;
        if (accumTime >= 1.0f) {
            printf("FPS: %f\n", FPS);
            accumTime = 0.0f;
        }
        // SDL_Delay(1000);

    }

    // SDL_Delay(3000);  // Pause execution for 3000 milliseconds, for example

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;

}