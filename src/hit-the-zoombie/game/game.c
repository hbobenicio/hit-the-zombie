#include "game.h"

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <stb_ds.h>

#include "settings.h"
#include "collision.h"
#include "background.h"
#include "score.h"

#define MAX_ZOOMBIES 30
#define JETBRAINS_MONO_REGULAR_TTF_FILE_PATH "./assets/fonts/UbuntuMono-R.ttf"

int game_init(struct game* game)
{
    const struct game_settings* settings = game_settings_get();

    if (zoombie_init_sprites() != 0) {
        fprintf(stderr, "error: game: failed to init zoombie sprites\n");
        return 1;
    }

    if (background_init(&game->background) != 0) {
        fprintf(stderr, "error: game: failed to init background\n");
        goto err_zoombie_free_sprites;
    }

    game->jetbrains_mono_regular_font = TTF_OpenFont(JETBRAINS_MONO_REGULAR_TTF_FILE_PATH, 22);
    if (game->jetbrains_mono_regular_font == NULL) {
        fprintf(stderr, "error: game: failed to init font '%s': %s", JETBRAINS_MONO_REGULAR_TTF_FILE_PATH, TTF_GetError());
        goto err_background_free;
    }

    // TODO 2055 sound wave could be clipped to be smaller and faster
    // const char* hit_snd_file_path = "./assets/mixkit-small-hit-in-a-game-2072.wav";
    const char* hit_snd_file_path = "./assets/mixkit-boxer-getting-hit-2055.wav";
    game->hit_snd = Mix_LoadWAV(hit_snd_file_path);
    if (game->hit_snd == NULL) {
        fprintf(stderr, "error: game: failed to load hit sound effects from wav '%s': %s\n", hit_snd_file_path, Mix_GetError());
        goto err_ttf_close_font;
    }

    struct zoombie zoombie = {0};
    if (zoombie_init(&zoombie) != 0) {
        fprintf(stderr, "error: game: failed to init zoombie\n");
        goto err_mix_close_wav;
    }
    arrpush(game->zoombies, zoombie);
    arrpush(game->respawn_timers, 0);

    score_init(&game->score, game->jetbrains_mono_regular_font);

    if (settings->display_fps) {
        fps_timer_init(&game->fps_timer, game->jetbrains_mono_regular_font);
    }

    return 0;

err_mix_close_wav:
    Mix_FreeChunk(game->hit_snd);

err_ttf_close_font:
    TTF_CloseFont(game->jetbrains_mono_regular_font);

err_background_free:
    background_free(&game->background);

err_zoombie_free_sprites:
    zoombie_free_sprites();

    return 1;
}

void game_free(struct game* game)
{
    arrfree(game->respawn_timers);
    arrfree(game->zoombies);

    Mix_FreeChunk(game->hit_snd);
    game->hit_snd = NULL;

    TTF_CloseFont(game->jetbrains_mono_regular_font);
    game->jetbrains_mono_regular_font = NULL;

    background_free(&game->background);

    zoombie_free_sprites();
}

int game_render(struct game* game, SDL_Renderer* renderer)
{
    const struct game_settings* settings = game_settings_get();

    if (background_render(&game->background, renderer) != 0) {
        fprintf(stderr, "error: game: background rendering failed\n");
        return 1;
    }

    // NOTE worth parallelizing this?
    for (long i = 0; i < arrlen(game->zoombies); i++) {
        if (zoombie_render(&game->zoombies[i], renderer) != 0) {
            fprintf(stderr, "error: game: zoombie rendering failed\n");
            return 1;
        }
    }

    if (score_render(&game->score, renderer) != 0) {
        fprintf(stderr, "error: game: score rendering failed\n");
        return 1;
    }

    if (settings->display_fps) {
        if (fps_timer_render(&game->fps_timer, renderer) != 0) {
            fprintf(stderr, "error: game: fps timer rendering failed\n");
        }
    }

    return 0;
}

void game_update(struct game* game)
{
    const struct game_settings* settings = game_settings_get();

    if (settings->display_fps) {
        fps_timer_update(&game->fps_timer);
    }

    // Hit Detection
    int mouse_x = -1, mouse_y = -1;
    uint32_t mouse_btn_state = SDL_GetMouseState(&mouse_x, &mouse_y);
    if (mouse_btn_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {

        // find a hit collision with the first (top) zoombie visible.
        // NOTE: lower indexes are draw first, so they appear behind.
        //       So we collision detection order is the inverse from the index from the zoombies arraylist.
        for (long i = arrlen(game->zoombies) - 1; i >= 0; i--) {
            struct zoombie* zoombie = &game->zoombies[i];

            // if it hits a zoombie, start it's respawning timer, play the hit sound, score it
            // then quit searching for other zoombie collisions
            if (collision_rect_point(&zoombie->box, mouse_x, mouse_y)) {
                // hit a dead body. skip searching for collisions behind it.
                if (!zoombie_is_alive(zoombie))
                    break;

                zoombie_set_state(zoombie, ZOOMBIE_STATE_DYING);

                game->respawn_timers[i] = SDL_GetTicks();

                // TODO how to prevent sound errors zoombies dies too fast? prevent playing this over and over...
                if (Mix_PlayChannel(-1, game->hit_snd, 0) != 0) {
                    fprintf(stderr, "error: game: failed to play hit sound effects: %s\n", Mix_GetError());
                }
                score_inc(&game->score);

                // We just wanna check 1 hit per update, right?
                break;
            }
        }
    }

    // Sprites Update
    for (long i = 0; i < arrlen(game->zoombies); i++) {

        // if the zoombie is dead, there is no need in updating it.
        // we just wait for it to respawn.
        if (game->zoombies[i].state == ZOOMBIE_STATE_DEAD) {
            if (SDL_GetTicks() - game->respawn_timers[i] > 2000) {

                // The hit zoombie respawns without changing its index in the arraylist, just to make it stable
                assert(zoombie_init(&game->zoombies[i]) == 0);

                // It then respawns
                struct zoombie new_zoombie = {0};
                assert(zoombie_init(&new_zoombie) == 0);

                // But a new one spawns too only if we hadn't reach the limit
                if (arrlen(game->zoombies) < MAX_ZOOMBIES) {
                    arrpush(game->zoombies, new_zoombie);
                    arrpush(game->respawn_timers, 0);
                }
            }
            continue;
        }

        zoombie_update(&game->zoombies[i]);
    }
}
