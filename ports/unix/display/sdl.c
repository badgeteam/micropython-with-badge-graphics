#include <stdint.h>
#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h> 
#include <SDL2/SDL_timer.h> 

SDL_Window* window = NULL;
SDL_Renderer *renderer = NULL;

void sdlInit() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { 
        printf("error initializing SDL: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_CreateWindowAndRenderer(320, 240, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void sdlFlush(const uint8_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    for (uint16_t x = x0; x < x1; x++) {
        for (uint16_t y = y0; y < y1; y++) {
            uint8_t r = buffer[(x + y*320) * 3 + 0];
            uint8_t g = buffer[(x + y*320) * 3 + 1];
            uint8_t b = buffer[(x + y*320) * 3 + 2];
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    SDL_RenderPresent(renderer);
}
