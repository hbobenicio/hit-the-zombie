#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <errno.h>

#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

#include "game/game.h"
#include "game/screen.h"

#define FRAME_RATE 60 //fps

int main() {
    srand(time(NULL));

    // Maybe all of these initialization calls could be encapsulated in the game init function...
    // or at least in another function.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "error: sdl2: init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        fprintf(stderr, "error: sdl2: failed to init SDL2_image: %s\n", IMG_GetError());
        goto err_sdl_quit;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "error: sdl2: failed to init SDL2_ttf: %s\n", TTF_GetError());
        goto err_img_quit;
    }
    {
        int page_size = (int) sysconf(_SC_PAGESIZE);
        int chunk_size = (page_size == -1) ? 4098 : page_size;
        if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, chunk_size) != 0) {
            fprintf(stderr, "error: sdl2: failed to init SDL2_mixer: %s\n", Mix_GetError());
            goto err_ttf_quit;
        }
    }
    SDL_Window* window = SDL_CreateWindow(
        "Hit The Zoombie!",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        // SDL_WINDOW_SHOWN
        SDL_WINDOW_FULLSCREEN
    );
    if (window == NULL) {
        fprintf(stderr, "error: sdl2: failed to create window: %s\n", SDL_GetError());
        goto err_mix_close_audio;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "error: sdl: failed to create renderer: %s\n", SDL_GetError());
        goto err_sdl_destroy_window;
    }

    struct game game = {0};
    if (game_init(&game) != 0) {
        fprintf(stderr, "error: failed to init game\n");
        goto err_sdl_destroy_renderer;
    }

    const uint32_t expected_frame_duration_ms = 1000 / FRAME_RATE;
    while (true) {
        uint32_t frame_start_ms = SDL_GetTicks();
        {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_QUIT:
                    goto exit_main_loop;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        goto exit_main_loop;
                    break;

                default:
                    break;
                }
            }
        }
        game_update(&game);

        if (game_render(&game, renderer) != 0) {
            fprintf(stderr, "error: game: rendering failed\n");
            goto err_game_free;
        }

        SDL_RenderPresent(renderer);

        uint32_t frame_duration_ms = SDL_GetTicks() - frame_start_ms;
        if (frame_duration_ms < expected_frame_duration_ms) {
            uint32_t delay = expected_frame_duration_ms - frame_duration_ms;
            SDL_Delay(delay);
        }
    }

exit_main_loop:
    game_free(&game);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;

err_game_free:
    game_free(&game);

err_sdl_destroy_renderer:
    SDL_DestroyRenderer(renderer);

err_sdl_destroy_window:
    SDL_DestroyWindow(window);

err_mix_close_audio:
    Mix_CloseAudio();

err_ttf_quit:
    TTF_Quit();

err_img_quit:
    IMG_Quit();

err_sdl_quit:
    SDL_Quit();
    return 1;
}
