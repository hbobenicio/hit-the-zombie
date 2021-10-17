#include "fps_timer.h"
#include <SDL2/SDL_timer.h>
#include "screen.h"

#include <assert.h>

void fps_timer_init(struct fps_timer* fps_timer, TTF_Font* font)
{
    fps_timer->frame_counter = 0;
    fps_timer->start_time = SDL_GetTicks();
    fps_timer->fps = 0;
    fps_timer->font = font;
    fps_timer->color = (SDL_Color) { .r = 255, .g = 255, .b = 255, .a = 255 };
}

void fps_timer_update(struct fps_timer* fps_timer)
{
    fps_timer->frame_counter++;

    uint32_t elapsed_time = SDL_GetTicks() - fps_timer->start_time;
    if (elapsed_time >= 1000) {
        fps_timer->fps = fps_timer->frame_counter / (elapsed_time / 1000.0);
        fps_timer->frame_counter = 0;
        fps_timer->start_time = SDL_GetTicks();
    }
}

int fps_timer_render(const struct fps_timer* fps_timer, SDL_Renderer* renderer)
{
    // enough space to support strings like "FPS: NNNN"
    char fps_txt[16];
    const int fps_txt_size = sizeof(fps_txt) / sizeof(fps_txt[0]);

    int nprinted = snprintf(fps_txt, fps_txt_size, "FPS: %d", fps_timer->fps);
    assert(nprinted > 0 && nprinted < fps_txt_size);

    SDL_Surface* fps_surface = TTF_RenderText_Solid(fps_timer->font, fps_txt, fps_timer->color);
    if (fps_surface == NULL) {
        fprintf(stderr, "error: game: fps_timer: failed render fps text: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Texture* fps_texture = SDL_CreateTextureFromSurface(renderer, fps_surface);
    SDL_FreeSurface(fps_surface);
    fps_surface = NULL;
    if (fps_texture == NULL) {
        fprintf(stderr, "error: game: fps_timer: failed to create fps texture: %s\n", SDL_GetError());
        return 1;
    }

    int margin_bottom = 10, height = 85;
    SDL_Rect fps_txt_box = {
        .x = 10,
        .y = SCREEN_HEIGHT - height - margin_bottom,
        .w = 150,
        .h = height
    };
    assert(SDL_RenderCopy(renderer, fps_texture, NULL, &fps_txt_box) == 0);

    SDL_DestroyTexture(fps_texture);
    return 0;
}
