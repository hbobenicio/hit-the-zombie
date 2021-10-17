#ifndef HTZ_GAME_FPS_TIMER_H
#define HTZ_GAME_FPS_TIMER_H

#include <stddef.h>

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_ttf.h>

struct fps_timer {
    uint32_t frame_counter;
    uint32_t start_time;
    int fps;
    TTF_Font* font;
    SDL_Color color;
};

void fps_timer_init(struct fps_timer* fps_timer, TTF_Font* font);
void fps_timer_update(struct fps_timer* fps_timer);
int fps_timer_render(const struct fps_timer* fps_timer, SDL_Renderer* renderer);

#endif
