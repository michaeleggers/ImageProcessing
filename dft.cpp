#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <cmath>
#include <complex>

#include <SDL2/SDL.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std::complex_literals;

static bool keys[SDL_NUM_SCANCODES] = { false };

#define PI                      3.14159265359
#define EPSILON                 0.00001

#define CHECKERBOARD_WIDTH    64
#define CHECKERBOARD_HEIGHT   64
#define CHECKERBOARD_CHANNELS 4
#define CHECKERBOARD_SIZE     (CHECKERBOARD_WIDTH * CHECKERBOARD_HEIGHT)
#define CHECKERBOARD_BYTES    (CHECKERBOARD_SIZE * CHECKERBOARD_CHANNELS)
#define MAX_CHECKERBOARD_INDEX ((CHECKERBOARD_HEIGHT*CHECKERBOARD_WIDTH + CHECKERBOARD_WIDTH)*4)
uint8_t checkerboard[CHECKERBOARD_BYTES];


const int WINDOW_WIDTH  = 1024;
const int WINDOW_HEIGHT = 768;


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

// NOTE:
// Assumptions: 
// - Input is grayscale, 3 channels, 8 bit / channel. All channels have same luminance value
void ComputeDFT(unsigned char* in_grayscaleImg, unsigned char* out, int width, int height) {
    double highestMagnitude = 0.0;
    double* tmp = (double*)malloc(3 * width * height * sizeof(double));
    for (int m = 0; m < height; m++) {
        for (int n = 0; n < width; n++) {
            std::complex<double> sum(0.0, 0.0);
            for (int k = 0; k < height; k++) {
                for (int l = 0; l < width; l++) {
                    unsigned char* pixel = &in_grayscaleImg[3 * (k * width + l)];
                    float fLuminance = (float)(*pixel) / 255.0;
                    //*out = std::complex<double>(fLuminance) * std::exp(1i * PI);
                    double f = ( (double)(k * m) / height) + ( (double)(l * n) / width);
                    std::complex<double> cFreq = std::complex<double>(fLuminance) * std::exp(-1i * 2.0 * PI * f);
                    sum += cFreq;
                }
            }
            double magnitudeSpectrum = std::abs(sum);
            magnitudeSpectrum = std::log(magnitudeSpectrum + 1);
            if (magnitudeSpectrum > highestMagnitude) {
                highestMagnitude = magnitudeSpectrum;
            }
            double* outPixel = tmp + 3 * (m * width + n);
            outPixel[0] = magnitudeSpectrum;
            outPixel[1] = magnitudeSpectrum;
            outPixel[2] = magnitudeSpectrum;
        }
    }

    // Saveguard for division by 0

    if (highestMagnitude < EPSILON) highestMagnitude = 1;

    // Fill the output

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double magnitude = tmp[3 * (i * width + j)];
            magnitude /= highestMagnitude;
            magnitude *= 255.0;
            unsigned char* outPixel = out + 3 * (i * width + j);
            outPixel[0] = (unsigned char)magnitude;
            outPixel[1] = (unsigned char)magnitude;
            outPixel[2] = (unsigned char)magnitude;
        }
    }
    free(tmp);
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

    // Create grayscale version
    unsigned char* imgDataGrayscale = (unsigned char*)malloc(x * y * n);
    unsigned char* pixel = imgData;
    for (int col = 0; col < y; col++) {
        for (int row = 0; row < x; row++) {
            float red = (float)(*pixel++)/255.0;
            float green = (float)(*pixel++) / 255.0;
            float blue = (float)(*pixel++) / 255.0;

            float grayR = 0.3 * red;
            float grayG = 0.59 * green;
            float grayB = 0.11 * blue;
            float gray = 0.3 * red + 0.59 * green + 0.11 * blue;
            
            memset(imgDataGrayscale + 3 * (col * x + row), (unsigned char)(gray * 255.0), 3);
        }
    }

    // Compute DFT
    unsigned char* imgDFT = (unsigned char*)malloc(x * y * n);
    ComputeDFT(imgDataGrayscale, imgDFT, x, y);

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
    SDL_UpdateTexture(renderTexture, &renderRect, imgDFT, n*x);

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_ShowWindow(window);
   

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
            if (event.key.keysym.scancode == SDL_SCANCODE_F) {
                SDL_SetWindowSize(window, x, y);
            }
            updateKeys(event);
        }


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

    stbi_image_free(imgData);

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;

}