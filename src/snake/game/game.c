#include "game.h"

int game_init(struct game* game)
{
    if (enemy_init_sprites() != 0) {
        return 1;
    }

    if (enemy_init(&game->enemy, ENEMY_GENDER_MALE) != 0) {
        enemy_free_sprites();
        return 1;
    }

    return 0;
}

void game_free(struct game* game)
{
    (void) game;
    enemy_free_sprites();
}

int game_render(struct game* game, SDL_Renderer* renderer)
{
    return enemy_render(&game->enemy, renderer);
}

void game_update(struct game* game)
{
    enemy_update(&game->enemy);
}
