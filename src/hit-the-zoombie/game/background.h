#ifndef HTZ_GAME_BACKGROUND_H
#define HTZ_GAME_BACKGROUND_H

#include <SDL2/SDL_render.h>

struct background {
    SDL_Surface* sprite;
};

int background_init(struct background* bg);
void background_free(struct background* bg);
int background_render(const struct background* bg, SDL_Renderer* renderer);

#endif
