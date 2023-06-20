#ifndef HTZ_GAME_H
#define HTZ_GAME_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_mixer.h>

#include "background.h"
#include "zoombie.h"
#include "score.h"
#include "fps_timer.h"

struct game_settings;

struct game {
    const struct game_settings* settings;
    struct background background;
    struct score score;

    // arraylist os zoombies.
    // arrlen(zoombies) should always be equal to arrlen(respawn_timers)
    struct zoombie* zoombies;
    uint32_t* respawn_timers;

    struct fps_timer fps_timer;

    TTF_Font* jetbrains_mono_regular_font;
    Mix_Chunk* hit_snd;
};

int game_init(struct game* game, const struct game_settings* settings);
void game_free(struct game* game);
int game_render(struct game* game, SDL_Renderer* renderer);
void game_update(struct game* game);

#endif
