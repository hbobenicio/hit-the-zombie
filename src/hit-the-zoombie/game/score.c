#include "score.h"

#include <assert.h>
#include <SDL2/SDL_ttf.h>

void score_init(struct score* score, TTF_Font* font)
{
    score->box = (SDL_Rect) {
        .x = 10,
        .y = 10,
        .w = 300,
        .h = 65,
    };
    score->color = (SDL_Color) {
        .r = 255,
        .g = 255,
        .b = 255,
        .a = 255,
    };
    score->font = font;
    score->value = 0;
}

void score_inc(struct score* score)
{
    score->value++;
}

int score_render(struct score* score, SDL_Renderer* renderer)
{
    char score_txt[16];
    const int score_txt_size = sizeof(score_txt) / sizeof(score_txt[0]);

    int nprinted = snprintf(score_txt, score_txt_size, "Score: %d", score->value);
    assert(nprinted > 0 && nprinted < score_txt_size);

    SDL_Surface* surface = TTF_RenderText_Solid(score->font, score_txt, score->color);
    if (surface == NULL) {
        fprintf(stderr, "error: score: failed to render text surface: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        fprintf(stderr, "error: score: failed to create texture from text surface: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return 1;
    }

    SDL_FreeSurface(surface);
    surface = NULL;

    if (SDL_RenderCopy(renderer, texture, NULL, &score->box) != 0) {
        fprintf(stderr, "error: score: failed to render: %s\n", SDL_GetError());
        SDL_DestroyTexture(texture);
        return 1;
    }

    SDL_DestroyTexture(texture);
    return 0;
}
