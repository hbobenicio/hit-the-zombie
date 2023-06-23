#include "settings.h"

#include <stdio.h>
#include <stdlib.h>

#include <hit-the-zoombie/util/util.h>

#define FRAME_RATE_DEFAULT 60

static struct game_settings settings = {0};

const struct game_settings* game_settings_get(void)
{
    return &settings;
}

void game_settings_init_from_envvars(void)
{
    settings.display_fullscreen = getenv("HTZ_WINDOWED") == NULL;
    settings.display_fps = getenv("HTZ_FPS") != NULL;
    settings.bounding_box_show = getenv("HTZ_BOUNDING_BOX_SHOW") != NULL;

    settings.frame_rate = FRAME_RATE_DEFAULT;
    const char* frame_rate_env_val = getenv("HTZ_FRAME_RATE");
    if (frame_rate_env_val != NULL) {
        int rc = parse_ul(frame_rate_env_val, &settings.frame_rate);
        if (rc != 0) {
            fprintf(stderr, "warn: falling back to using the default FPS value: %d\n", FRAME_RATE_DEFAULT);
        }
    }
}
