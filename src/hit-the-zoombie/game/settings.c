#include "settings.h"

#include <stdlib.h>

void game_settings_init_from_envvars(struct game_settings* gs)
{
    gs->display_fullscreen = getenv("HTZ_WINDOWED") == NULL;
    gs->display_fps = getenv("HTZ_FPS") != NULL;
}
