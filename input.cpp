#include "input.h"

#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

#include <imgui/backends/imgui_impl_sdl2.h>

#include "event_handler.h"
#include "ievent.h"
#include "events.h"

struct MouseState {
    int x, y;
    int oldX, oldY;
    int dX, dY;
    bool leftButtonDown;
    bool rightButtonDown;
    bool leftButtonWentUp;
    bool rightButtonWentUp;
    bool leftButtonPressed;
    bool rightButtonPressed;
};


// INTERNAL PROTOTYPES
void UpdateKeys(SDL_Event& e); 

static bool keys[SDL_NUM_SCANCODES] = { false };
static bool keysPrev[SDL_NUM_SCANCODES] = { false };
static MouseState mouseState;

bool LeftMouseButtonWentUp() {
    return mouseState.leftButtonWentUp;
}

bool RightMouseButtonWentUp() {
    return mouseState.rightButtonWentUp;
}

bool LeftMouseButtonDown() {
    return mouseState.leftButtonDown;
}

bool RightMouseButtonDown() {
    return mouseState.rightButtonDown;
}

int MouseX() {
    return mouseState.x;
}

int MouseY() {
    return mouseState.y;
}

bool LeftMouseButtonPressed() {
    return mouseState.leftButtonPressed;
}

bool RightMouseButtonPressed() {
    return mouseState.rightButtonPressed;
}

bool KeyReleased(SDL_Scancode scancode) {
    return keysPrev[scancode] && !keys[scancode];
}

bool KeyPressed(SDL_Scancode scancode) {
    return keys[scancode];
}

bool KeyDown(SDL_Scancode scancode) {
    return !keysPrev[scancode] && keys[scancode];
}

// MAIN SYSTEM EVENT HANDLER

void HandleSystemEvents(bool* shouldClose, SDL_Window* window, EventHandler* eventHandler) {
    
    // Remember keystate from prev frame.

    memcpy(keysPrev, keys, SDL_NUM_SCANCODES * sizeof(bool));

    // Set MouseState for this frame

    SDL_GetMouseState(&mouseState.x, &mouseState.y);
    mouseState.oldX = mouseState.x;
    mouseState.oldY = mouseState.y;
    mouseState.leftButtonDown = false;
    mouseState.rightButtonDown = false;
    mouseState.leftButtonWentUp = false;
    mouseState.rightButtonWentUp = false;
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        // Drag and drop from OS

        if (event.type == SDL_DROPFILE) {
            char* droppedFile = event.drop.file;            
            DropEvent de(droppedFile);
            eventHandler->Notify(&de);
            SDL_free(droppedFile);
        }

        UpdateKeys(event);

        if (event.type == SDL_QUIT) {
            *shouldClose = true;
            eventHandler->Notify(new RenderStopEvent());
        }

        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                SDL_Log("Window size has changed\n");
            }
        }
        else if (event.type == SDL_MOUSEMOTION) {
        }
        else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseState.leftButtonPressed = false;
                mouseState.leftButtonDown = false;
                mouseState.leftButtonWentUp = true;
            }
            if (event.button.button == SDL_BUTTON_RIGHT) {
                mouseState.rightButtonPressed = false;
                mouseState.rightButtonDown = false;
                mouseState.rightButtonWentUp = true;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseState.leftButtonPressed = true;
                mouseState.leftButtonDown = true;
            }
            if (event.button.button == SDL_BUTTON_RIGHT) {
                mouseState.rightButtonPressed = true;
                mouseState.rightButtonDown = true;
            }
        }
        else if (event.type == SDL_MOUSEMOTION) {
            mouseState.x = event.motion.x;
            mouseState.y = event.motion.y;
        }

        if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
            *shouldClose = true;
        }
        if (event.key.keysym.scancode == SDL_SCANCODE_F) {
            //SDL_SetWindowSize(window, x, y);
        }
    }
}

// INTERNAL

void UpdateKeys(SDL_Event& e) {    

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


