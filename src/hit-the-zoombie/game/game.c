#include "game.h"

#include <stdio.h>
#include <stdbool.h>

#include "background.h"
#include "score.h"

#define JETBRAINS_MONO_REGULAR_TTF_FILE_PATH "/home/hugo/.local/share/fonts/JetBrainsMono-Regular.ttf"

int game_init(struct game* game)
{
    if (zoombie_init_sprites() != 0) {
        fprintf(stderr, "error: game: failed to init zoombie sprites\n");
        return 1;
    }

    game->jetbrains_mono_regular_font = TTF_OpenFont(JETBRAINS_MONO_REGULAR_TTF_FILE_PATH, 24);
    if (game->jetbrains_mono_regular_font == NULL) {
        fprintf(stderr, "error: game: failed to init font '%s': %s", JETBRAINS_MONO_REGULAR_TTF_FILE_PATH, TTF_GetError());
        goto err_zoombie_free_sprites;
    }

    // const char* hit_snd_file_path = "./assets/mixkit-small-hit-in-a-game-2072.wav";
    const char* hit_snd_file_path = "./assets/mixkit-boxer-getting-hit-2055.wav";
    game->hit_snd = Mix_LoadWAV(hit_snd_file_path);
    if (game->hit_snd == NULL) {
        fprintf(stderr, "error: game: failed to load hit sound effects from wav '%s': %s\n", hit_snd_file_path, Mix_GetError());
        goto err_ttf_close_font;
    }

    if (zoombie_init(&game->zoombie) != 0) {
        fprintf(stderr, "error: game: failed to init zoombie\n");
        goto err_mix_close_wav;
    }

    score_init(&game->score, game->jetbrains_mono_regular_font);
    fps_timer_init(&game->fps_timer, game->jetbrains_mono_regular_font);

    return 0;

err_mix_close_wav:
    Mix_FreeChunk(game->hit_snd);

err_ttf_close_font:
    TTF_CloseFont(game->jetbrains_mono_regular_font);

err_zoombie_free_sprites:
    zoombie_free_sprites();

    return 1;
}

void game_free(struct game* game)
{
    Mix_FreeChunk(game->hit_snd);
    game->hit_snd = NULL;

    TTF_CloseFont(game->jetbrains_mono_regular_font);
    game->jetbrains_mono_regular_font = NULL;

    zoombie_free_sprites();
}

int game_render(struct game* game, SDL_Renderer* renderer)
{
    if (background_render(renderer) != 0) {
        fprintf(stderr, "error: game: background rendering failed\n");
        return 1;
    }

    if (zoombie_render(&game->zoombie, renderer) != 0) {
        fprintf(stderr, "error: game: zoombie rendering failed\n");
        return 1;
    }

    if (score_render(&game->score, renderer) != 0) {
        fprintf(stderr, "error: game: score rendering failed\n");
        return 1;
    }

    if (fps_timer_render(&game->fps_timer, renderer) != 0) {
        fprintf(stderr, "error: game: fps timer rendering failed\n");
    }

    return 0;
}

void game_update(struct game* game)
{
    fps_timer_update(&game->fps_timer);
    
    if (game->zoombie.state != ZOOMBIE_STATE_DEAD) {
        bool zoombie_hit = false;
        zoombie_update(&game->zoombie, &zoombie_hit);
        if (zoombie_hit) {
            game->respawn_timer = SDL_GetTicks();
            if (Mix_PlayChannel(-1, game->hit_snd, 0) != 0) {
                fprintf(stderr, "error: game: failed to play hit sound effects: %s\n", Mix_GetError());
            }
            score_inc(&game->score);
        }
    } else {
        if (SDL_GetTicks() - game->respawn_timer > 2000) {
            zoombie_init(&game->zoombie);
        }
    }
}
