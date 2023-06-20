#ifndef HTZ_GAME_SETTINGS_H
#define HTZ_GAME_SETTINGS_H

#include <stdbool.h>

/**
 * General and simple user controlled game settings.
 */
struct game_settings {
    /**
     * Indicates whether the display should be fullscreen or not.
     */
    bool display_fullscreen;
    /**
     * Indicates whether the display should render the FPS label or not.
    */
    bool display_fps;
};

void game_settings_init_from_envvars(struct game_settings* gs);

#endif
