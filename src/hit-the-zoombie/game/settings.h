#ifndef HTZ_GAME_SETTINGS_H
#define HTZ_GAME_SETTINGS_H

#include <stddef.h>
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
    /**
     * Indicates whether to render bounding boxes for debugging purposes.
     */
    bool bounding_box_show;
    /**
     * Indicates the desired animation frame rate
     */
    size_t frame_rate;
};

/**
 * Returns a pointer to the global settings.
 */
const struct game_settings* game_settings_get(void);

/**
 * Initializes the global game settings from environment variable definitions.
 */
void game_settings_init_from_envvars(void);

#endif
