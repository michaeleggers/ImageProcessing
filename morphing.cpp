#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <cmath>
#include <vector>

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
    
    Shader imageShader;
    if (!imageShader.Load(exePath + "../../shaders/basic.vert", exePath + "../../shaders/basic.frag")) {
        SDL_Log("Could not load shaders!\n");
        exit(-1);
    }   
    Shader finalShader;
    if (!finalShader.Load(exePath + "../../shaders/final.vert", exePath + "../../shaders/final.frag")) {
        SDL_Log("Could not load shaders!\n");
        exit(-1);
    }

    // Start a new batch and add a triangle

    Batch batch(1024, 3 * 1024);
    std::vector<Vertex> vertices = {
        {glm::vec3(-0.5, 0.5, 1.0), glm::vec3(1.0, 0.0, 0.0),  glm::vec2(0.0, 1.0)},
        {glm::vec3( 0.5, 0.5, 1.0), glm::vec3(0.0, 1.0, 0.0),  glm::vec2(1.0, 1.0)},
        {glm::vec3( 0.5, -0.5, 1.0), glm::vec3(0.0, 0.0, 1.0), glm::vec2(1.0, 0.0)},
        {glm::vec3(-0.5, -0.5, 1.0), glm::vec3(0.0, 1.0, 1.0), glm::vec2(0.0, 0.0)}
    };
    std::vector<uint32_t> indices = {
        0, 1, 2,
        2, 3, 0
    };
    std::vector<Vertex> vertices2 = {
        {glm::vec3(-0.3, 0.7, 1.0), glm::vec3(1.0, 0.0, 0.0),  glm::vec2(0.0, 1.0)},
        {glm::vec3(0.7, 0.7, 1.0), glm::vec3(0.0, 1.0, 0.0),   glm::vec2(1.0, 1.0)},
        {glm::vec3(0.7, -0.3, 1.0), glm::vec3(0.0, 0.0, 1.0),  glm::vec2(1.0, 0.0)},
        {glm::vec3(-0.3, -0.3, 1.0), glm::vec3(0.0, 1.0, 1.0), glm::vec2(0.0, 0.0)}
    };
    std::vector<uint32_t> indices2 = { // TODO: Simplify this. Maybe don't access batches directly.
        4, 5, 6,
        6, 7, 4
    };
    batch.Add(&vertices[0], vertices.size(), &indices[0], indices.size());
    batch.Add(&vertices2[0], vertices2.size(), &indices2[0], indices2.size());


    // Load image that will be presented in imgui window
    Image sourceImage(exePath + "../../assets/baboon.bmp");
    Image destImage(exePath + "../../assets/girlface.bmp");

    // Create Framebuffer that will be rendered to and displayed in a imgui frame
    Framebuffer sourceFBO(sourceImage.m_Width, sourceImage.m_Height);
    Framebuffer destFBO(destImage.m_Width, destImage.m_Height);

    // Some OpenGL global settings

    glFrontFace(GL_CW); // front faces are in clockwise order
    glCullFace(GL_FALSE);

    // Main loop
    
    bool shouldClose = 0;
    float accumTime = 0.0f;    
    while (!shouldClose) {

        Uint32 startTime = SDL_GetTicks();

        // Call event handler here
        HandleSystemEvents(&shouldClose);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();

        int windowWidth, windowHeight;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        float windowAspect = (float)windowWidth / (float)windowHeight;
        
        char cpMousePos[256];
        sprintf(cpMousePos, "MousePos: %d, %d", MouseX(), MouseY());
        SDL_SetWindowTitle(window, cpMousePos);

        // Draw stuff

        // Own imgui window we render the fbo into

        ShowWindow("Source", sourceFBO, imageShader, sourceImage, batch);
        //ShowWindow("Destination", destFBO, imageShader, destImage, batch);

        
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

    batch.Kill();
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