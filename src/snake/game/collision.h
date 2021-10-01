#ifndef SNAKE_GAME_COLLISION_H
#define SNAKE_GAME_COLLISION_H

#include <stdbool.h>

#include <SDL2/SDL_rect.h>

bool collision_rect_point(const SDL_Rect* rect, int x, int y);

#endif
