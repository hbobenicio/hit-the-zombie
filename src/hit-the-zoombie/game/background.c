#include "background.h"

#include <stdio.h>

#include <SDL2/SDL_image.h>

#include "screen.h"

#define BACKGROUND_SPRITE_FILE_PATH "./assets/background.png"

int background_init(struct background* bg)
{
    bg->sprite = IMG_Load(BACKGROUND_SPRITE_FILE_PATH);
    if (bg->sprite == NULL) {
        fprintf(stderr, "error: background: failed to load sprite at '%s': %s\n", BACKGROUND_SPRITE_FILE_PATH, IMG_GetError());
        return 1;
    }
    return 0;
}

void background_free(struct background* bg)
{
    SDL_FreeSurface(bg->sprite);
    bg->sprite = NULL;
}

int background_render(const struct background* bg, SDL_Renderer* renderer)
{
    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) != 0) {
        fprintf(stderr, "error: background: failed to set renderer draw color: %s\n", SDL_GetError());
        return 1;
    }

    if (SDL_RenderClear(renderer) != 0) {
        fprintf(stderr, "error: background: failed to clear background: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, bg->sprite);
    if (texture == NULL) {
        fprintf(stderr, "error: background: failed to create texture from sprite surface: %s\n", SDL_GetError());
        return 1;
    }

    if (SDL_RenderCopy(renderer, texture, NULL, NULL) != 0) {
        fprintf(stderr, "error: sdl2: sdl_image: failed create texture from surface: %s\n", SDL_GetError());
        SDL_DestroyTexture(texture);
        return 1;
    }

    SDL_DestroyTexture(texture);
    return 0;
}
