#ifndef HTZ_GAME_H
#define HTZ_GAME_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_mixer.h>

#include "enemy.h"
#include "score.h"
#include "fps_timer.h"

struct game {
    struct enemy enemy;
    struct score score;
    struct fps_timer fps_timer;

    TTF_Font* jetbrains_mono_regular_font;
    Mix_Chunk* hit_snd;
    uint32_t respawn_timer;
};

int game_init(struct game* game);
void game_free(struct game* game);
int game_render(struct game* game, SDL_Renderer* renderer);
void game_update(struct game* game);

#endif
