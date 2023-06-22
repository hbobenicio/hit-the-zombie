#include "settings.h"

#include <stdlib.h>

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
}
