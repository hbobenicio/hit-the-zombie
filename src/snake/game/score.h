#ifndef SNAKE_GAME_SCORE_H
#define SNAKE_GAME_SCORE_H

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_ttf.h>

struct score {
    SDL_Rect box;
    SDL_Color color;
    TTF_Font* font;
    int value;
};

int score_init_font(void);
void score_free_font(void);

void score_init(struct score* score);
void score_inc(struct score* score);
int score_render(struct score* score, SDL_Renderer* renderer);

#endif
