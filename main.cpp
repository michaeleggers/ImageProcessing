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

const int WINDOW_WIDTH  = 1024;
const int WINDOW_HEIGHT = 768;


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
    unsigned char *imgData = stbi_load(imgName, &x, &y, &n, 0);
    if (!imgData) {
        printf("Failed to load %s! Abort!\n", imgName);
        return -1;
    }
    printf("Loaded image %s, width(x): %d, height(y): %d, channels(n):%d\n", imgName, x, y, n);
    int stride = n * 3; // We assume we load 8bit/channel pictures
    float aspect = (float)x / (float)y;

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

    SDL_PixelFormatEnum pixelFormat = n == 3 ? SDL_PIXELFORMAT_RGB24 : SDL_PIXELFORMAT_RGBA8888;

    if (pixelFormat == SDL_PIXELFORMAT_RGB24) {
        printf("Pixelformat is: SDL_PIXELFORMAT_RGB24\n");
    }
    else if (pixelFormat == SDL_PIXELFORMAT_RGBA8888) {
        printf("Pixelformat is: SDL_PIXELFORMAT_RGBA8888\n");
    }
    SDL_Texture *renderTexture = SDL_CreateTexture(
        renderer, 
        pixelFormat,
        SDL_TEXTUREACCESS_TARGET, 
        x, y);
    uint32_t format;
    SDL_QueryTexture(renderTexture,
        &format, NULL,
        NULL, NULL);
    printf("Texture Pixel fomat: %s\n", SDL_GetPixelFormatName(format));


    // Copy image data into the texture
    SDL_Rect renderRect = { 0, 0, x, y };
    SDL_UpdateTexture(renderTexture, &renderRect, imgData, n*x);

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_ShowWindow(window);

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
        //SDL_SetRenderDrawColor(renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
        //SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 10, 90, SDL_ALPHA_OPAQUE);

        // Fit image to window size

        int windowWidth, windowHeight;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        float windowAspect = (float)windowWidth / (float)windowHeight;

        SDL_Rect dstRect = { 0 };
        if (aspect > windowAspect) { // horizontal letterbox
            dstRect.w = windowWidth;
            dstRect.h = windowWidth/aspect;
        }
        else { // vertical letterbox
            dstRect.w = windowHeight*aspect;
            dstRect.h = windowHeight;
        }
        dstRect.x = (windowWidth - dstRect.w) / 2.0;
        dstRect.y = (windowHeight - dstRect.h) / 2.0;

        // Blit the render texture (= the image) to the screen

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, renderTexture, &renderRect, &dstRect);

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