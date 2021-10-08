#ifndef HTZ_GAME_ENEMY_H
#define HTZ_GAME_ENEMY_H

#include <stdbool.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>

#define HTZ_ENEMY_ATTACK_SPRITE_LEN 8
#define HTZ_ENEMY_DEAD_SPRITE_LEN 9
#define HTZ_ENEMY_IDLE_SPRITE_LEN 15
#define HTZ_ENEMY_WALK_SPRITE_LEN 10

enum enemy_gender {
    ENEMY_GENDER_MALE,
    ENEMY_GENDER_FEMALE,
};

enum enemy_state {
    ENEMY_STATE_ATTACK,
    ENEMY_STATE_DYING,
    ENEMY_STATE_IDLE,
    ENEMY_STATE_WALK,
    ENEMY_STATE_DEAD,
};

enum enemy_direction {
    ENEMY_DIRECTION_LEFT,
    ENEMY_DIRECTION_RIGHT,
};

struct enemy {
    SDL_Rect box;

    enum enemy_gender gender;
    enum enemy_state state;
    enum enemy_direction direction;
    int velocity;

    uint32_t animation_tick;
    uint32_t animation_sprite_index;
};

int enemy_init_sprites(void);
void enemy_free_sprites(void);

int enemy_init(struct enemy* enemy);
int enemy_render(struct enemy* enemy, SDL_Renderer* renderer);
void enemy_set_state(struct enemy* enemy, enum enemy_state new_state);
void enemy_update(struct enemy* enemy, bool* out_hit);
bool enemy_is_alive(const struct enemy* enemy);

#endif
