#include "background.h"

#include <stdio.h>

int background_render(SDL_Renderer* renderer)
{
    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) != 0) {
        fprintf(stderr, "error: background: failed to set renderer draw color: %s\n", SDL_GetError());
        return 1;
    }

    if (SDL_RenderClear(renderer) != 0) {
        fprintf(stderr, "error: background: failed to clear background: %s\n", SDL_GetError());
        return 1;
    }

    // TODO improve the background by loading some green fields texture
    
    return 0;
}
