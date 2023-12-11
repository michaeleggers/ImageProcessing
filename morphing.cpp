
// IMAGE MORPHING USING BEIER-NEELY ALGORITHM
// 
// Known Bugs:
// 
// 4.12.2023
// - non quare images stretch the result image to the right weirdly
// - if a source line has no matching destination line the program crashes
// - if source and destination images don't have the same dimensions the program crashes
//
// TODOs:
// - Port to MacOS
// - Undo/Redo for lineplacement
// - Save lineset and corresponding images as project 
// - Change from line-mode to select-mode to change/delete existing linepairs
// - Render result images out to disk
// - At the moment the pixels are interpolated linearly from source-line to dest-line. But
//   I noticed that this will result in ignoring rotations. So It would be better to actually
//   interpolate the lines correctly and *then* compute the resulting pixel.
//   This will be computationally more taxing but could result in better warping and therefore
//   less noticable blending between source and destination image.
//
// Fixed Bugs:
// - (12.7.2023) It seems that the sequence is not being fully processed. The last image in the result window still shows parts of the source image



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <cmath>
#include <vector>
#include <algorithm>


//#include <SDL2/SDL.h>
//#include <SDL.h>
#include <SDL2/SDL.h>
#include <glad/glad.h>

#include "imgui.h"
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include "input.h"
#include "camera.h"
#include "shader.h"
#include "fbo.h"
#include "batch.h"
#include "common.h"
#include "editor.h"
#include "image.h"
#include "static_geometry.h"
#include "beierneely.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define PI                      3.14159265359
#define EPSILON                 0.00001

#define CHECKERBOARD_WIDTH    64
#define CHECKERBOARD_HEIGHT   64
#define CHECKERBOARD_CHANNELS 4
#define CHECKERBOARD_SIZE     (CHECKERBOARD_WIDTH * CHECKERBOARD_HEIGHT)
#define CHECKERBOARD_BYTES    (CHECKERBOARD_SIZE * CHECKERBOARD_CHANNELS)
#define MAX_CHECKERBOARD_INDEX ((CHECKERBOARD_HEIGHT*CHECKERBOARD_WIDTH + CHECKERBOARD_WIDTH)*4)
uint8_t checkerboard[CHECKERBOARD_BYTES];


const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

void initCheckerboardTexture() {
    for (size_t y = 0; y < CHECKERBOARD_HEIGHT; y++) {
        for (size_t x = 0; x < CHECKERBOARD_WIDTH; x++) {
            size_t index = (y * CHECKERBOARD_WIDTH + x) * 4;
            uint8_t blackOrWhite = ((y & 0x8) == 0 ^ (x & 0x8) == 0) * 255;
            checkerboard[index + 0] = blackOrWhite;
            checkerboard[index + 1] = blackOrWhite;
            checkerboard[index + 2] = blackOrWhite;
            checkerboard[index + 3] = SDL_ALPHA_OPAQUE;
        }
    }
    printf("max tex index: %d\n", MAX_CHECKERBOARD_INDEX);
}

int main(int argc, char** argv)
{
    std::string exePath = com_GetExePath();

    SDL_Window* window;                    
    SDL_Init(SDL_INIT_EVERYTHING);         

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "Morphing",                  
        SDL_WINDOWPOS_UNDEFINED,           
        SDL_WINDOWPOS_UNDEFINED,           
        WINDOW_WIDTH,                      
        WINDOW_HEIGHT,                     
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    SDL_GLContext sdl_gl_Context = SDL_GL_CreateContext(window);
    if (!sdl_gl_Context) {
        SDL_Log("Unable to create GL context! SDL-Error: %s\n", SDL_GetError());
        exit(-1);
    }

    SDL_GL_MakeCurrent(window, sdl_gl_Context);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("Failed to get OpenGL function pointers via GLAD: %s\n", SDL_GetError());
        exit(-1);
    }

    // Check that the window was successfully created

    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_ShowWindow(window);

    // Static geometry
    InitStaticGeometry();

    // Setup Imgui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, sdl_gl_Context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Load Shaders
        
    Shader finalShader;
#ifdef WIN32
    if (!finalShader.Load(exePath + "../../shaders/final.vert", exePath + "../../shaders/final.frag")) {
        SDL_Log("Could not load shaders!\n");
        exit(-1);
    }
#elif __APPLE__
    if (!finalShader.Load(exePath + "/../shaders/final.vert", exePath + "/../shaders/final.frag")) {
        SDL_Log("Could not load shaders!\n");
        exit(-1);
    }
#endif

    // Start a new batch and add a triangle

    Batch sourceBatch(1024, 3 * 1024);
    Batch destBatch(1024, 3 * 1024);

    // Load image that will be presented in imgui window

#ifdef WIN32
    Image sourceImage(exePath + "../../assets/guy_squared.bmp");
    Image destImage(exePath + "../../assets/gal_squared.bmp");
#elif __APPLE__
    Image sourceImage(exePath + "/../assets/guy_squared.bmp");
    Image destImage(exePath + "/../assets/gal_squared.bmp");
#endif

    // Some OpenGL global settings

    glFrontFace(GL_CW); // front faces are in clockwise order
    glCullFace(GL_FALSE);

    // Create the editor

    Editor editor(sourceImage, destImage);
    Editor editor2(destImage, sourceImage);

    // Main loop
    
    bool shouldClose = 0;
    float accumTime = 0.0f;    
    while (!shouldClose) {

        Uint32 startTime = SDL_GetTicks();

        // Call event handler here
        HandleSystemEvents(&shouldClose);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();

        int windowWidth, windowHeight;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        float windowAspect = (float)windowWidth / (float)windowHeight;
        
        char cpMousePos[256];
        sprintf(cpMousePos, "MousePos: %d, %d", MouseX(), MouseY());
        SDL_SetWindowTitle(window, cpMousePos);

        // Draw stuff

        // Run the editor
        
        editor.Run();        

        // Second pass

        // Tell opengl about window size to make correct transform into screenspace
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0.2f, 0.4f, 0.7f, 1.0f); // Nice blue :)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glBindTexture(GL_TEXTURE_2D, texture);
        //finalShader.Activate();
        //finalBatch.Bind();
        //glDrawElements(GL_TRIANGLES, finalBatch.IndexCount(), GL_UNSIGNED_INT, nullptr);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);

        Uint32 endTime = SDL_GetTicks();
        Uint32 timePassed = endTime - startTime;
        float timePassedSeconds = (float)timePassed / 1000.0f;
        float FPS = 1.0f / timePassedSeconds;
        accumTime += timePassedSeconds;
        if (accumTime >= 1.0f) {
            //printf("FPS: %f\n", FPS);
            accumTime = 0.0f;
        }   
    }

    // Kill OpenGL resources

    sourceBatch.Kill();
    destBatch.Kill();
    DestroyStaticGeometry();

    // Deinit ImGui

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;
}
