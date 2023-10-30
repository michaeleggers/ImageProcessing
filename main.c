#include <stdio.h>
#include <SDL2/SDL.h>

void initInt(int* out_int) {
    *out_int = 10;
}

int initInt2() {
    return 10;
}

typedef struct Entity {
    int posX, posY;
    float vel;
    char * name;
} EntityT;

void createEntity(EntityT ** out_e) {
    *out_e = malloc(sizeof(EntityT));
    printf("e2 addr in createEntity: %p\n", (void*)*out_e);
    (**out_e).posX = 10;
}

int main (int argc, char ** argv) 
{
    EntityT e = { .posY=10, .posX=20, .vel=.5f, .name=malloc(10) };
    strcpy(e.name, "hallo");
    printf("entity name: %s\n", e.name);

    EntityT * e2;
    printf("e2 addr: %p\n", (void*)e2);
    createEntity(&e2);
    printf("e2 addr after createEntity: %p\n", (void*)e2);
    printf("e2 posX: %d\n", e2->posX);

    int myInt;
    initInt(&myInt);
    int myInt2 = initInt2();


    SDL_Window *window;                    // Declare a pointer

    SDL_Init(SDL_INIT_EVERYTHING);              // Initialize SDL2

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "Bene is impatient",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        640,                               // width, in pixels
        480,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Failed to init renderer. Bye!\n");
        return 1;
    }

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_ShowWindow(window);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

    // The window is open: could enter program loop here (see SDL_PollEvent())
    SDL_Event event;
    int shouldClose = 0;
    while (!shouldClose) {
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldClose = 1;
            }
        }

        SDL_SetRenderDrawColor(renderer, 100, 0, 90, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawPoint(renderer, 10, 10);

        SDL_RenderPresent(renderer);

    }

    // SDL_Delay(3000);  // Pause execution for 3000 milliseconds, for example

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
    return 0;

}