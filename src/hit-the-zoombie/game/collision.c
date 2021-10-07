#include "collision.h"

bool collision_rect_point(const SDL_Rect* rect, int x, int y)
{
    bool x_collision = (rect->x <= x) && (x <= rect->x + rect->w);
    bool y_collision = (rect->y <= y) && (y <= rect->y + rect->h);
    return x_collision && y_collision;
}
