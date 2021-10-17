#ifndef HTZ_GAME_ZOOMBIE_H
#define HTZ_GAME_ZOOMBIE_H

#include <stdbool.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>

#define HTZ_ZOOMBIE_DEAD_SPRITE_LEN 9
#define HTZ_ZOOMBIE_WALK_SPRITE_LEN 10

enum zoombie_gender {
    ZOOMBIE_GENDER_MALE,
    ZOOMBIE_GENDER_FEMALE,
};

enum zoombie_state {
    ZOOMBIE_STATE_WALK,
    ZOOMBIE_STATE_DYING,
    ZOOMBIE_STATE_DEAD,
};

enum zoombie_direction {
    ZOOMBIE_DIRECTION_LEFT,
    ZOOMBIE_DIRECTION_RIGHT,
};

struct zoombie {
    SDL_Rect box;

    enum zoombie_gender gender;
    enum zoombie_state state;
    enum zoombie_direction direction;
    int velocity;

    uint32_t animation_tick;
    uint32_t animation_sprite_index;
};

int zoombie_init_sprites(void);
void zoombie_free_sprites(void);

int zoombie_init(struct zoombie* zoombie);
int zoombie_render(struct zoombie* zoombie, SDL_Renderer* renderer);
void zoombie_set_state(struct zoombie* zoombie, enum zoombie_state new_state);
void zoombie_update(struct zoombie* zoombie);
bool zoombie_is_alive(const struct zoombie* zoombie);

#endif
