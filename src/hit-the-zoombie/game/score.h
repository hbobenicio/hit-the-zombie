#ifndef HTZ_GAME_SCORE_H
#define HTZ_GAME_SCORE_H

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_ttf.h>

struct score {
    SDL_Rect box;
    SDL_Color color;
    TTF_Font* font;
    int value;
};

void score_init(struct score* score, TTF_Font* font);
void score_inc(struct score* score);
int score_render(struct score* score, SDL_Renderer* renderer);

#endif
