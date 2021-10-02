#include "score.h"

#include <assert.h>
#include <SDL2/SDL_ttf.h>

#define JETBRAINS_MONO_REGULAR_TTF_FILE_PATH "/home/hugo/.local/share/fonts/JetBrainsMono-Regular.ttf"

static TTF_Font* jetbrains_mono_regular_font = NULL;

int score_init_font(void)
{
    jetbrains_mono_regular_font = TTF_OpenFont(JETBRAINS_MONO_REGULAR_TTF_FILE_PATH, 24);
    if (jetbrains_mono_regular_font == NULL) {
        fprintf(stderr, "error: score: failed to init font '%s': %s", JETBRAINS_MONO_REGULAR_TTF_FILE_PATH, TTF_GetError());
        return 1;
    }

    return 0;
}

void score_free_font(void)
{
    TTF_CloseFont(jetbrains_mono_regular_font);
}

void score_init(struct score* score)
{
    score->box = (SDL_Rect) {
        .x = 10,
        .y = 10,
        .w = 350,
        .h = 85,
    };
    score->color = (SDL_Color) {
        .r = 255,
        .g = 255,
        .b = 255,
        .a = 255,
    };
    score->font = jetbrains_mono_regular_font;
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
