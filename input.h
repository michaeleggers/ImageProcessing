#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL2/SDL.h>

#include "event_handler.h"

void HandleSystemEvents(bool* shouldClose, SDL_Window* window, EventHandler* eventHandler);

// MOUSE

bool LeftMouseButtonWentUp();
bool RightMouseButtonWentUp();
bool LeftMouseButtonDown();
bool RightMouseButtonDown();
int  MouseX();
int  MouseY();
bool LeftMouseButtonPressed();
bool RightMouseButtonPressed();

// KEYBOARD

bool KeyReleased(SDL_Scancode scancode);
bool KeyPressed(SDL_Scancode scancode);
bool KeyDown(SDL_Scancode scancode);

#endif
