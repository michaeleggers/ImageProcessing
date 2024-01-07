
// IMAGE MORPHING USING BEIER-NEELY ALGORITHM
// 
// Known Bugs:
// 
// Fixed Bugs:
// - (7.12.2023)  It seems that the sequence is not being fully processed. The last image in the result window still shows parts of the source image
// - (13.12.2023) Don't crash the program if an image could not be loaded from disk. Instead show checkerboard texture
// - (20.12.2023) if a source line has no matching destination line the program crashes
// - (22.12.2023) If the image index in the result window is higher than the range specified for a new sequence, the index causes
//                an index out of range error and the program crashes.
// - (30.12.2023) Non square images stretch the result image to the right weirdly
// - (some time ago) If source and destination images don't have the same dimensions the program crashes.
// 
// TODOs:
// - Redo for lines
// - Change from line-mode to select-mode to change/delete existing linepairs
// - At the moment the pixels are interpolated linearly from source-line to dest-line. But
//   I noticed that this will result in ignoring rotations. So It would be better to actually
//   interpolate the lines correctly and *then* compute the resulting pixel.
//   This will be computationally more taxing but could result in better warping and therefore
//   less noticable blending between source and destination image.
//
// TODOs Done:
// - (7.12.2023)  Port to MacOS
// - (12.12.2023) Save linesets and corresponding images as project 
// - (12.12.2023) Load linesets and images and weights as project
// - (13.12.2023) Support loading and writing project files with spaces in filename
// - (19.12.2023) Render result images out to disk
// - (19.12.2023) (Primitive) undo (Ctrl+Z) to kill lines
// - (some time ago) Add buttons to load src/dst images during runtime


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <cmath>
#include <vector>
#include <algorithm>

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
#include "static_image_data.h"
#include "beierneely.h"
#include "event_handler.h"
#include "processor.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#define PI                      3.14159265359
#define EPSILON                 0.00001

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

EventHandler* eventHandler;

int main(int argc, char** argv)
{
    std::string exePath = com_GetExePath();

    SDL_Window* window;                    
    SDL_Init(SDL_INIT_EVERYTHING);         

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "Morphing",                  
        SDL_WINDOWPOS_UNDEFINED,           
        SDL_WINDOWPOS_UNDEFINED,           
        WINDOW_WIDTH,                      
        WINDOW_HEIGHT,                     
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
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

    // GL Vsync on
    if (SDL_GL_SetSwapInterval(1) != 0) {
        SDL_Log("Failed to enable vsync!\n");
    }
    else {
        SDL_Log("vsync enabled\n");
    }

    // Setup Window icon

    Uint32 rmask = 0x000000ff;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x00ff0000;
    Uint32 amask = 0xff000000;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
#else // little endian, like x86
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    // Keep for reference. Not really needed on Windows with .rc file

    //Image windowIcon(exePath + "../../res/icon2.bmp");
    //SDL_Surface* windowIconSurf = SDL_CreateRGBSurfaceFrom(
    //    windowIcon.m_Data,
    //    windowIcon.m_Width, windowIcon.m_Height,
    //    24, // depth
    //    windowIcon.m_Width * 3, // 3 bytes/pixel
    //    rmask,
    //    gmask,
    //    bmask,
    //    amask
    //);
    //SDL_SetWindowIcon(window, windowIconSurf);
    //SDL_FreeSurface(windowIconSurf);

    // Static geometry

    InitStaticGeometry();

    // Checkerboard texture for images that could not be found on disk

    InitCheckerboardTexture(64, 64, 3);   

    // Setup Imgui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, sdl_gl_Context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Load Shaders
        
    Shader finalShader;
#ifdef WIN32
    if (!finalShader.Load(exePath + "shaders/final.vert", exePath + "shaders/final.frag")) {
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
    Image sourceImage(exePath + "assets/guy_squared.bmp");
    Image destImage(exePath + "assets/gal_squared.bmp");
#elif __APPLE__
    Image sourceImage(exePath + "/../assets/guy_squared.bmp");
    Image destImage(exePath + "/../assets/gal_squared.bmp");
#endif

    // Some OpenGL global settings

    glFrontFace(GL_CW); // front faces are in clockwise order
    glCullFace(GL_FALSE);

    // Create the event manager

    eventHandler = new EventHandler();

    // Processor. Does the heavy lifting in this program

    Processor processor(eventHandler);

    // Create the editor

    Editor editor(sourceImage, destImage, eventHandler);    

    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    printf("%s, %s\n", vendor, renderer);

    // Main loop
    
    bool shouldClose = 0;
    float accumTime = 0.0f;    
    while (!shouldClose) {

        Uint32 startTime = SDL_GetTicks();

        // Call event handler here
        HandleSystemEvents(&shouldClose, window, eventHandler);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();

        int windowWidth, windowHeight;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        float windowAspect = (float)windowWidth / (float)windowHeight;
        

        // Draw stuff

        // Processor: Check if rendering

        processor.CheckRenderThread();

        // Run the editor
        
        editor.Run();

        // Update Win32 Window title

        SDL_SetWindowTitle(window, ("MagicMorph - " + editor.GetProjectName()).c_str());

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

    delete eventHandler;

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
