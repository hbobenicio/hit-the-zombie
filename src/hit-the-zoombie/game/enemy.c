#include "enemy.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#include <SDL2/SDL_image.h>

#include <hit-the-zoombie/util/random.h>
#include "screen.h"
#include "collision.h"

#define MALE_ATTACK_SPRITE_FMT   "./assets/zombiefiles/png/male/Attack (%d).png"
#define MALE_DEAD_SPRITE_FMT     "./assets/zombiefiles/png/male/Dead (%d).png"
#define MALE_IDLE_SPRITE_FMT     "./assets/zombiefiles/png/male/Idle (%d).png"
#define MALE_WALK_SPRITE_FMT     "./assets/zombiefiles/png/male/Walk (%d).png"

#define FEMALE_ATTACK_SPRITE_FMT "./assets/zombiefiles/png/female/Attack (%d).png"
#define FEMALE_DEAD_SPRITE_FMT   "./assets/zombiefiles/png/female/Dead (%d).png"
#define FEMALE_IDLE_SPRITE_FMT   "./assets/zombiefiles/png/female/Idle (%d).png"
#define FEMALE_WALK_SPRITE_FMT   "./assets/zombiefiles/png/female/Walk (%d).png"

// TODO animation speed could raise according to the current score/difficulty (or the velocity itself could raise instead...)
#define ATTACK_SPRITE_ANIMATION_DURATION_MS 32
#define DEAD_SPRITE_ANIMATION_DURATION_MS   64
#define IDLE_SPRITE_ANIMATION_DURATION_MS   32
#define WALK_SPRITE_ANIMATION_DURATION_MS   44

struct enemy_sprites {
    SDL_Surface* attack[HTZ_ENEMY_ATTACK_SPRITE_LEN];
    SDL_Surface* dead[HTZ_ENEMY_DEAD_SPRITE_LEN];
    SDL_Surface* idle[HTZ_ENEMY_IDLE_SPRITE_LEN];
    SDL_Surface* walk[HTZ_ENEMY_WALK_SPRITE_LEN];
};

static struct enemy_sprites enemy_male_sprites   = {0};
static struct enemy_sprites enemy_female_sprites = {0};

static bool enemy_collide_screen(const struct enemy* enemy) {
    bool right_collision = enemy->box.x + enemy->box.w >= SCREEN_WIDTH;
    bool left_collision = enemy->box.x <= 0;
    return right_collision || left_collision;
}

static const char* strgender(enum enemy_gender gender) {
    switch (gender) {
    case ENEMY_GENDER_MALE:
        return "male";

    case ENEMY_GENDER_FEMALE:
        return "female";

    default:
        assert(false && "unsupported enemy gender");
    }
}

static struct enemy_sprites* enemy_sprites(const struct enemy* enemy) {
    switch (enemy->gender)
    {
    case ENEMY_GENDER_MALE:
        return &enemy_male_sprites;

    case ENEMY_GENDER_FEMALE:
        return &enemy_female_sprites;

    default:
        assert(false && "unsupported enemy gender");
    }
}

static const char* enemy_sprite_fmt(enum enemy_gender gender, enum enemy_state state) {
    switch (state) {
    case ENEMY_STATE_ATTACK:
        return (gender == ENEMY_GENDER_MALE) ? MALE_ATTACK_SPRITE_FMT : FEMALE_ATTACK_SPRITE_FMT;

    case ENEMY_STATE_DYING:
    case ENEMY_STATE_DEAD:
        return (gender == ENEMY_GENDER_MALE) ? MALE_DEAD_SPRITE_FMT : FEMALE_DEAD_SPRITE_FMT;

    case ENEMY_STATE_IDLE:
        return (gender == ENEMY_GENDER_MALE) ? MALE_IDLE_SPRITE_FMT : FEMALE_IDLE_SPRITE_FMT;

    case ENEMY_STATE_WALK:
        return (gender == ENEMY_GENDER_MALE) ? MALE_WALK_SPRITE_FMT : FEMALE_WALK_SPRITE_FMT;

    default:
        assert(false && "unsupported enemy state");
    }
}

static uint32_t enemy_sprite_animation_len(const struct enemy* enemy) {
    switch (enemy->state) {
    case ENEMY_STATE_ATTACK:
        return HTZ_ENEMY_ATTACK_SPRITE_LEN;

    case ENEMY_STATE_DYING:
    case ENEMY_STATE_DEAD:
        return HTZ_ENEMY_DEAD_SPRITE_LEN;

    case ENEMY_STATE_IDLE:
        return HTZ_ENEMY_IDLE_SPRITE_LEN;

    case ENEMY_STATE_WALK:
        return HTZ_ENEMY_WALK_SPRITE_LEN;

    default:
        assert(false && "unsupported enemy state");
    }
}

static uint32_t enemy_sprite_animation_duration_ms(const struct enemy* enemy) {
    switch (enemy->state) {
    case ENEMY_STATE_ATTACK:
        return ATTACK_SPRITE_ANIMATION_DURATION_MS;

    case ENEMY_STATE_DYING:
    case ENEMY_STATE_DEAD:
        return DEAD_SPRITE_ANIMATION_DURATION_MS;

    case ENEMY_STATE_IDLE:
        return IDLE_SPRITE_ANIMATION_DURATION_MS;

    case ENEMY_STATE_WALK:
        return WALK_SPRITE_ANIMATION_DURATION_MS;

    default:
        assert(false && "unsupported enemy state");
    }
}

static int load_sprite(enum enemy_gender gender, enum enemy_state state, int sprite_index, SDL_Surface** out_surface)
{
    assert(out_surface != NULL);

    char sprite_file_path[256] = {0};
    int sprite_file_path_size = sizeof(sprite_file_path) / sizeof(sprite_file_path[0]);
    const char* sprite_fmt = enemy_sprite_fmt(gender, state);

    int nprinted = snprintf(sprite_file_path, sprite_file_path_size, sprite_fmt, sprite_index + 1);
    if (nprinted <= 0 || nprinted >= sprite_file_path_size) {
        fprintf(
            stderr,
            "error: enemy: failed to format sprite file path. fmt='%s' maxbufsize=%d gender=%s index=%d\n",
            sprite_fmt, sprite_file_path_size, strgender(gender), sprite_index + 1
        );
        *out_surface = NULL;
        return 1;
    }

    SDL_Surface* surface = IMG_Load(sprite_file_path);
    if (surface == NULL) {
        fprintf(stderr, "error: enemy: failed to load sprite surface for '%s': %s\n", sprite_file_path, IMG_GetError());
        *out_surface = NULL;
        return 1;
    }

    *out_surface = surface;
    return 0;
}

int enemy_init_sprites(void)
{
    for (int i = 0; i < HTZ_ENEMY_ATTACK_SPRITE_LEN; i++) {
        if (load_sprite(ENEMY_GENDER_MALE, ENEMY_STATE_ATTACK, i, &enemy_male_sprites.attack[i]) != 0) {
            return 1;
        }
        if (load_sprite(ENEMY_GENDER_FEMALE, ENEMY_STATE_ATTACK, i, &enemy_female_sprites.attack[i]) != 0) {
            return 1;
        }
    }
    for (int i = 0; i < HTZ_ENEMY_DEAD_SPRITE_LEN; i++) {
        if (load_sprite(ENEMY_GENDER_MALE, ENEMY_STATE_DEAD, i, &enemy_male_sprites.dead[i]) != 0) {
            return 1;
        }
        if (load_sprite(ENEMY_GENDER_FEMALE, ENEMY_STATE_DEAD, i, &enemy_female_sprites.dead[i]) != 0) {
            return 1;
        }
    }
    for (int i = 0; i < HTZ_ENEMY_IDLE_SPRITE_LEN; i++) {
        if (load_sprite(ENEMY_GENDER_MALE, ENEMY_STATE_IDLE, i, &enemy_male_sprites.idle[i]) != 0) {
            return 1;
        }
        if (load_sprite(ENEMY_GENDER_FEMALE, ENEMY_STATE_IDLE, i, &enemy_female_sprites.idle[i]) != 0) {
            return 1;
        }
    }
    for (int i = 0; i < HTZ_ENEMY_WALK_SPRITE_LEN; i++) {
        if (load_sprite(ENEMY_GENDER_MALE, ENEMY_STATE_WALK, i, &enemy_male_sprites.walk[i]) != 0) {
            return 1;
        }
        if (load_sprite(ENEMY_GENDER_FEMALE, ENEMY_STATE_WALK, i, &enemy_female_sprites.walk[i]) != 0) {
            return 1;
        }
    }
    return 0;
}

void enemy_free_sprites(void)
{
    for (int i = 0; i < HTZ_ENEMY_ATTACK_SPRITE_LEN; i++) {
        SDL_FreeSurface(enemy_male_sprites.attack[i]);
        SDL_FreeSurface(enemy_female_sprites.attack[i]);
    }
    for (int i = 0; i < HTZ_ENEMY_DEAD_SPRITE_LEN; i++) {
        SDL_FreeSurface(enemy_male_sprites.dead[i]);
        SDL_FreeSurface(enemy_female_sprites.dead[i]);
    }
    for (int i = 0; i < HTZ_ENEMY_IDLE_SPRITE_LEN; i++) {
        SDL_FreeSurface(enemy_male_sprites.idle[i]);
        SDL_FreeSurface(enemy_female_sprites.idle[i]);
    }
    for (int i = 0; i < HTZ_ENEMY_WALK_SPRITE_LEN; i++) {
        SDL_FreeSurface(enemy_male_sprites.walk[i]);
        SDL_FreeSurface(enemy_female_sprites.walk[i]);
    }
}

int enemy_init(struct enemy* enemy, enum enemy_gender gender)
{
    // TODO check if every animation sprite have these sizes
    static const int sprite_width = 521, sprite_height = 576;

    int x = random_range_int(0, SCREEN_WIDTH - sprite_width - 5);
    int y = random_range_int(150, SCREEN_HEIGHT - sprite_height - 5);
    enemy->box = (SDL_Rect) {
        .x = x,
        .y = y,
        .w = sprite_width / 2,
        .h = sprite_height / 2,
    };
    enemy->gender = gender;
    // enemy->state = ENEMY_STATE_IDLE;
    enemy->state = ENEMY_STATE_WALK;
    enemy->direction = ENEMY_DIRECTION_RIGHT;
    enemy->velocity = 10;
    enemy->animation_tick = SDL_GetTicks();
    enemy->animation_sprite_index = 0;
    return 0;
}

int enemy_render(struct enemy* enemy, SDL_Renderer* renderer)
{
    struct enemy_sprites* sprites = enemy_sprites(enemy);

    SDL_Texture* texture = NULL;
    switch (enemy->state) {
    case ENEMY_STATE_ATTACK:
        texture = SDL_CreateTextureFromSurface(renderer, sprites->attack[enemy->animation_sprite_index]);
        break;

    case ENEMY_STATE_DYING:
        texture = SDL_CreateTextureFromSurface(renderer, sprites->dead[enemy->animation_sprite_index]);
        break;

    case ENEMY_STATE_IDLE:
        texture = SDL_CreateTextureFromSurface(renderer, sprites->idle[enemy->animation_sprite_index]);
        break;

    case ENEMY_STATE_WALK:
        texture = SDL_CreateTextureFromSurface(renderer, sprites->walk[enemy->animation_sprite_index]);
        break;

    case ENEMY_STATE_DEAD:
        texture = SDL_CreateTextureFromSurface(renderer, sprites->dead[enemy_sprite_animation_len(enemy) - 1]);
        break;

    default:
        assert(false && "unsupported enemy gender");
    }
    if (!texture) {
        fprintf(stderr, "error: sdl2: sdl_image: failed create texture from surface: %s\n", SDL_GetError());
        return 1;
    }

    // flip sprite if facing left
    if (enemy->direction == ENEMY_DIRECTION_LEFT) {
        if (SDL_RenderCopyEx(renderer, texture, NULL, &enemy->box, 0.0, NULL, SDL_FLIP_HORIZONTAL) != 0) {
            fprintf(stderr, "error: sdl2: sdl_image: failed create texture from surface: %s\n", SDL_GetError());
            goto err_destroy_texture;
        }
    } else {
        if (SDL_RenderCopy(renderer, texture, NULL, &enemy->box) != 0) {
            fprintf(stderr, "error: sdl2: sdl_image: failed create texture from surface: %s\n", SDL_GetError());
            goto err_destroy_texture;
        }
    }


    #ifdef DEBUG_BOUNDING_BOX
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &enemy->box);
    #endif

    SDL_DestroyTexture(texture);
    return 0;

err_destroy_texture:
    SDL_DestroyTexture(texture);
    return 1;
}

void enemy_set_state(struct enemy* enemy, enum enemy_state new_state)
{
    enemy->state = new_state;
    enemy->animation_sprite_index = 0;
    enemy->animation_tick = SDL_GetTicks();
}

// NOTE current limitation: animations are linear. maybe could be improved in some no linear fashion (function parametrization)
void enemy_update(struct enemy* enemy, bool* out_hit)
{
    // dead enemy => noop
    if (enemy->state == ENEMY_STATE_DEAD) {
        return;
    }

    // Hit collision check (only for alive enemies)
    if (enemy_is_alive(enemy)) {
        int mouse_x, mouse_y;
        uint32_t mouse_btn_state = SDL_GetMouseState(&mouse_x, &mouse_y);
        if (mouse_btn_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (collision_rect_point(&enemy->box, mouse_x, mouse_y)) {
                enemy_set_state(enemy, ENEMY_STATE_DYING);
                *out_hit = true;
            }
        }
    }

    bool should_animate = SDL_GetTicks() - enemy->animation_tick > enemy_sprite_animation_duration_ms(enemy);
    if (should_animate) {
        enemy->animation_sprite_index = (enemy->animation_sprite_index + 1) % enemy_sprite_animation_len(enemy);
        enemy->animation_tick = SDL_GetTicks();

        enemy->box.x += enemy->velocity;
        if (enemy_collide_screen(enemy)) {
            enemy->box.x -= enemy->velocity;
            if (enemy->direction == ENEMY_DIRECTION_RIGHT)
                enemy->direction = ENEMY_DIRECTION_LEFT;
            else
                enemy->direction = ENEMY_DIRECTION_RIGHT;
            enemy->velocity = -enemy->velocity;
        }

        if (enemy->state == ENEMY_STATE_DYING && enemy->animation_sprite_index == 0) {
            enemy->state = ENEMY_STATE_DEAD;
        }
    }
}

bool enemy_is_alive(const struct enemy* enemy)
{
    return enemy->state != ENEMY_STATE_DYING && enemy->state != ENEMY_STATE_DEAD;
}
