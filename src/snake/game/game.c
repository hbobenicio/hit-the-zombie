#include "game.h"

#include <stdio.h>
#include <stdbool.h>

#include "background.h"
#include "score.h"

int game_init(struct game* game)
{
    // static initialization

    if (score_init_font() != 0) {
        fprintf(stderr, "error: game: failed to init font\n");
        return 1;
    }

    if (enemy_init_sprites() != 0) {
        fprintf(stderr, "error: game: failed to init enemy sprites\n");
        goto err_score_free_font;
    }

    // objects initialization

    if (enemy_init(&game->enemy, ENEMY_GENDER_FEMALE) != 0) {
        fprintf(stderr, "error: game: failed to init enemy\n");
        goto err_enemy_free_sprites;
    }

    score_init(&game->score);

    return 0;

err_enemy_free_sprites:
    enemy_free_sprites();

err_score_free_font:
    score_free_font();

    return 1;
}

void game_free(struct game* game)
{
    (void) game;
    enemy_free_sprites();
    score_free_font();
}

int game_render(struct game* game, SDL_Renderer* renderer)
{
    if (background_render(renderer) != 0) {
        fprintf(stderr, "error: game: background rendering failed\n");
        return 1;
    }

    if (enemy_render(&game->enemy, renderer) != 0) {
        fprintf(stderr, "error: game: enemy rendering failed\n");
        return 1;
    }

    if (score_render(&game->score, renderer) != 0) {
        fprintf(stderr, "error: game: score rendering failed\n");
        return 1;
    }

    return 0;
}

void game_update(struct game* game)
{
    bool enemy_hit = false;
    enemy_update(&game->enemy, &enemy_hit);
    if (enemy_hit) {
        score_inc(&game->score);
    }
}
