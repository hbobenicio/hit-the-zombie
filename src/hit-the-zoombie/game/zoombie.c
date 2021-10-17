#include "zoombie.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#include <SDL2/SDL_image.h>

#include <hit-the-zoombie/util/random.h>
#include "screen.h"

#define MALE_DEAD_SPRITE_FMT     "./assets/zombiefiles/png/male/Dead (%d).png"
#define MALE_WALK_SPRITE_FMT     "./assets/zombiefiles/png/male/Walk (%d).png"

#define FEMALE_DEAD_SPRITE_FMT   "./assets/zombiefiles/png/female/Dead (%d).png"
#define FEMALE_WALK_SPRITE_FMT   "./assets/zombiefiles/png/female/Walk (%d).png"

// TODO animation speed could raise according to the current score/difficulty (or the velocity itself could raise instead...)
#define DEAD_SPRITE_ANIMATION_DURATION_MS   64
#define WALK_SPRITE_ANIMATION_DURATION_MS   44

#define SPRITE_SCALING_RATIO 3.5

struct zoombie_sprites {
    SDL_Surface* dead[HTZ_ZOOMBIE_DEAD_SPRITE_LEN];
    SDL_Surface* walk[HTZ_ZOOMBIE_WALK_SPRITE_LEN];
};

static struct zoombie_sprites zoombie_male_sprites   = {0};
static struct zoombie_sprites zoombie_female_sprites = {0};

static bool zoombie_collide_screen(const struct zoombie* zoombie) {
    bool right_collision = zoombie->box.x + zoombie->box.w >= SCREEN_WIDTH;
    bool left_collision = zoombie->box.x <= 0;
    return right_collision || left_collision;
}

static const char* strgender(enum zoombie_gender gender) {
    switch (gender) {
    case ZOOMBIE_GENDER_MALE:
        return "male";

    case ZOOMBIE_GENDER_FEMALE:
        return "female";

    default:
        assert(false && "unsupported zoombie gender");
    }
}

static struct zoombie_sprites* zoombie_sprites(const struct zoombie* zoombie) {
    switch (zoombie->gender)
    {
    case ZOOMBIE_GENDER_MALE:
        return &zoombie_male_sprites;

    case ZOOMBIE_GENDER_FEMALE:
        return &zoombie_female_sprites;

    default:
        assert(false && "unsupported zoombie gender");
    }
}

static const char* zoombie_sprite_fmt(enum zoombie_gender gender, enum zoombie_state state) {
    switch (state) {
    case ZOOMBIE_STATE_DYING:
    case ZOOMBIE_STATE_DEAD:
        return (gender == ZOOMBIE_GENDER_MALE) ? MALE_DEAD_SPRITE_FMT : FEMALE_DEAD_SPRITE_FMT;

    case ZOOMBIE_STATE_WALK:
        return (gender == ZOOMBIE_GENDER_MALE) ? MALE_WALK_SPRITE_FMT : FEMALE_WALK_SPRITE_FMT;

    default:
        assert(false && "unsupported zoombie state");
    }
}

static uint32_t zoombie_sprite_animation_len(const struct zoombie* zoombie) {
    switch (zoombie->state) {
    case ZOOMBIE_STATE_DYING:
    case ZOOMBIE_STATE_DEAD:
        return HTZ_ZOOMBIE_DEAD_SPRITE_LEN;

    case ZOOMBIE_STATE_WALK:
        return HTZ_ZOOMBIE_WALK_SPRITE_LEN;

    default:
        assert(false && "unsupported zoombie state");
    }
}

static uint32_t zoombie_sprite_animation_duration_ms(const struct zoombie* zoombie) {
    switch (zoombie->state) {
    case ZOOMBIE_STATE_DYING:
    case ZOOMBIE_STATE_DEAD:
        return DEAD_SPRITE_ANIMATION_DURATION_MS;

    case ZOOMBIE_STATE_WALK:
        return WALK_SPRITE_ANIMATION_DURATION_MS;

    default:
        assert(false && "unsupported zoombie state");
    }
}

/**
 * walk:
 *     width: 338
 *         start: 17
 *         end: 355
 *     height: 477
 *         start: 41
 *         end: 518
 * dead:
 *     width: 590
 *         start: 0
 *         end: 590
 *     height: 455
 *         start: 34
 *         end: 489
 */
static SDL_Rect zoombie_sprite_male_rect(enum zoombie_state state) {
    switch (state)
    {
    case ZOOMBIE_STATE_WALK:
        return (SDL_Rect) { .x = 17, .y = 41, .w = 338, .h = 477 };

    case ZOOMBIE_STATE_DYING:
    case ZOOMBIE_STATE_DEAD:
        return (SDL_Rect) { .x = 0, .y = 34, .w = 590, .h = 455 };

    default:
        assert(false && "unsupported zoombie state");
    }
}

/**
 * walk:
 *     width:  [93, 438]: 345
 *     height: [35, 576]: 541
 * dead:
 *     width: [27, 604]: 577
 *     height: [42, 598]: 556
 */
static SDL_Rect zoombie_sprite_female_rect(enum zoombie_state state) {
    switch (state)
    {
    case ZOOMBIE_STATE_WALK:
        return (SDL_Rect) { .x = 93, .y = 35, .w = 345, .h =  541 };

    case ZOOMBIE_STATE_DYING:
    case ZOOMBIE_STATE_DEAD:
        return (SDL_Rect) { .x = 27, .y = 42, .w = 577, .h =  556 };

    default:
        assert(false && "unsupported zoombie state");
    }
}

static SDL_Rect zoombie_sprite_rect(enum zoombie_gender gender, enum zoombie_state state) {
    if (gender == ZOOMBIE_GENDER_MALE)   return zoombie_sprite_male_rect(state);
    if (gender == ZOOMBIE_GENDER_FEMALE) return zoombie_sprite_female_rect(state);
    assert(false && "unsupported zoombie gender");
}

static int load_sprite(enum zoombie_gender gender, enum zoombie_state state, int sprite_index, SDL_Surface** out_surface)
{
    assert(out_surface != NULL);

    char sprite_file_path[256] = {0};
    int sprite_file_path_size = sizeof(sprite_file_path) / sizeof(sprite_file_path[0]);
    const char* sprite_fmt = zoombie_sprite_fmt(gender, state);

    int nprinted = snprintf(sprite_file_path, sprite_file_path_size, sprite_fmt, sprite_index + 1);
    if (nprinted <= 0 || nprinted >= sprite_file_path_size) {
        fprintf(
            stderr,
            "error: zoombie: failed to format sprite file path. fmt='%s' maxbufsize=%d gender=%s index=%d\n",
            sprite_fmt, sprite_file_path_size, strgender(gender), sprite_index + 1
        );
        *out_surface = NULL;
        return 1;
    }

    SDL_Surface* surface = IMG_Load(sprite_file_path);
    if (surface == NULL) {
        fprintf(stderr, "error: zoombie: failed to load sprite surface for '%s': %s\n", sprite_file_path, IMG_GetError());
        *out_surface = NULL;
        return 1;
    }

    *out_surface = surface;
    return 0;
}

int zoombie_init_sprites(void)
{
    for (int i = 0; i < HTZ_ZOOMBIE_DEAD_SPRITE_LEN; i++) {
        if (load_sprite(ZOOMBIE_GENDER_MALE, ZOOMBIE_STATE_DEAD, i, &zoombie_male_sprites.dead[i]) != 0) {
            return 1;
        }
        if (load_sprite(ZOOMBIE_GENDER_FEMALE, ZOOMBIE_STATE_DEAD, i, &zoombie_female_sprites.dead[i]) != 0) {
            return 1;
        }
    }
    for (int i = 0; i < HTZ_ZOOMBIE_WALK_SPRITE_LEN; i++) {
        if (load_sprite(ZOOMBIE_GENDER_MALE, ZOOMBIE_STATE_WALK, i, &zoombie_male_sprites.walk[i]) != 0) {
            return 1;
        }
        if (load_sprite(ZOOMBIE_GENDER_FEMALE, ZOOMBIE_STATE_WALK, i, &zoombie_female_sprites.walk[i]) != 0) {
            return 1;
        }
    }
    return 0;
}

void zoombie_free_sprites(void)
{
    for (int i = 0; i < HTZ_ZOOMBIE_DEAD_SPRITE_LEN; i++) {
        SDL_FreeSurface(zoombie_male_sprites.dead[i]);
        SDL_FreeSurface(zoombie_female_sprites.dead[i]);
    }
    for (int i = 0; i < HTZ_ZOOMBIE_WALK_SPRITE_LEN; i++) {
        SDL_FreeSurface(zoombie_male_sprites.walk[i]);
        SDL_FreeSurface(zoombie_female_sprites.walk[i]);
    }
}

int zoombie_init(struct zoombie* zoombie)
{
    // state setup
    static const enum zoombie_state initial_state = ZOOMBIE_STATE_WALK;
    zoombie->state = initial_state;

    // gender setup
    zoombie->gender = (random_range_int(0, 1) == 0) ? ZOOMBIE_GENDER_MALE : ZOOMBIE_GENDER_FEMALE;

    const SDL_Rect sprite_rect = zoombie_sprite_rect(zoombie->gender, initial_state);

    // NOTE the sprite is too big for our scene. This could be improved by resizing all zoombie sprite images
    int width = sprite_rect.w / SPRITE_SCALING_RATIO;
    int height = sprite_rect.h / SPRITE_SCALING_RATIO;

    static const int padding = 5;
    int x = random_range_int(0, SCREEN_WIDTH - width - padding);
    int y = random_range_int(0, SCREEN_HEIGHT - height - padding);
    zoombie->box = (SDL_Rect) {
        .x = x,
        .y = y,
        .w = width,
        .h = height,
    };

    // direction and velocity setup
    if (random_range_int(0, 1) == 0) {
        zoombie->direction = ZOOMBIE_DIRECTION_RIGHT;
        zoombie->velocity = 10; // NOTE this could be improved by a increasing value based on current score
    } else {
        zoombie->direction = ZOOMBIE_DIRECTION_LEFT;
        zoombie->velocity = -10; // NOTE this could be improved by a increasing value based on current score
    }

    // animation setup
    zoombie->animation_tick = SDL_GetTicks();
    zoombie->animation_sprite_index = 0;
    return 0;
}

int zoombie_render(struct zoombie* zoombie, SDL_Renderer* renderer)
{
    struct zoombie_sprites* sprites = zoombie_sprites(zoombie);

    SDL_Texture* texture = NULL;
    switch (zoombie->state) {
    case ZOOMBIE_STATE_DYING:
        texture = SDL_CreateTextureFromSurface(renderer, sprites->dead[zoombie->animation_sprite_index]);
        break;

    case ZOOMBIE_STATE_WALK:
        texture = SDL_CreateTextureFromSurface(renderer, sprites->walk[zoombie->animation_sprite_index]);
        break;

    // case the zoombie is dead, we only texture we use is the last one.
    case ZOOMBIE_STATE_DEAD:
        texture = SDL_CreateTextureFromSurface(renderer, sprites->dead[zoombie_sprite_animation_len(zoombie) - 1]);
        break;

    default:
        assert(false && "unsupported zoombie gender");
    }
    if (!texture) {
        fprintf(stderr, "error: sdl2: sdl_image: failed create texture from surface: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Rect sprite_rect = zoombie_sprite_rect(zoombie->gender, zoombie->state);

    // flip sprite if facing left
    if (zoombie->direction == ZOOMBIE_DIRECTION_LEFT) {
        double angle = 0.0;
        SDL_Point* center = NULL;
        if (SDL_RenderCopyEx(renderer, texture, &sprite_rect, &zoombie->box, angle, center, SDL_FLIP_HORIZONTAL) != 0) {
            fprintf(stderr, "error: sdl2: sdl_image: failed create texture from surface: %s\n", SDL_GetError());
            goto err_destroy_texture;
        }
    } else {
        if (SDL_RenderCopy(renderer, texture, &sprite_rect, &zoombie->box) != 0) {
            fprintf(stderr, "error: sdl2: sdl_image: failed create texture from surface: %s\n", SDL_GetError());
            goto err_destroy_texture;
        }
    }

#ifdef DEBUG_BOUNDING_BOX
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &zoombie->box);
#endif

    SDL_DestroyTexture(texture);
    return 0;

err_destroy_texture:
    SDL_DestroyTexture(texture);
    return 1;
}

void zoombie_set_state(struct zoombie* zoombie, enum zoombie_state new_state)
{
    zoombie->state = new_state;

    SDL_Rect sprite_rect = zoombie_sprite_rect(zoombie->gender, new_state);
    zoombie->box.w = sprite_rect.w / SPRITE_SCALING_RATIO;
    zoombie->box.h = sprite_rect.h / SPRITE_SCALING_RATIO;

    zoombie->animation_sprite_index = 0;
    zoombie->animation_tick = SDL_GetTicks();
}

void zoombie_update(struct zoombie* zoombie)
{
    // if the zoombie is dead then there is nothing to update
    if (zoombie->state == ZOOMBIE_STATE_DEAD) {
        return;
    }

    bool should_animate = SDL_GetTicks() - zoombie->animation_tick > zoombie_sprite_animation_duration_ms(zoombie);
    if (should_animate) {
        zoombie->animation_sprite_index = (zoombie->animation_sprite_index + 1) % zoombie_sprite_animation_len(zoombie);
        zoombie->animation_tick = SDL_GetTicks();

        zoombie->box.x += zoombie->velocity;
        if (zoombie_collide_screen(zoombie)) {
            zoombie->box.x -= zoombie->velocity;
            if (zoombie->direction == ZOOMBIE_DIRECTION_RIGHT)
                zoombie->direction = ZOOMBIE_DIRECTION_LEFT;
            else
                zoombie->direction = ZOOMBIE_DIRECTION_RIGHT;
            zoombie->velocity = -zoombie->velocity;
        }

        if (zoombie->state == ZOOMBIE_STATE_DYING && zoombie->animation_sprite_index == 0) {
            zoombie->state = ZOOMBIE_STATE_DEAD;
        }
    }
}

bool zoombie_is_alive(const struct zoombie* zoombie)
{
    return zoombie->state != ZOOMBIE_STATE_DYING && zoombie->state != ZOOMBIE_STATE_DEAD;
}
