#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_mixer.h>

#include "enemy.h"
#include "score.h"

struct game {
    struct enemy enemy;
    struct score score;
    Mix_Chunk* hit_snd;
};

int game_init(struct game* game);
void game_free(struct game* game);
int game_render(struct game* game, SDL_Renderer* renderer);
void game_update(struct game* game);

#endif
