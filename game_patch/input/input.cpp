// Central SDL event pump for all input subsystems.
#include <SDL3/SDL.h>
#include "input.h"

void sdl_input_poll()
{
    keyboard_sdl_poll();
    mouse_sdl_poll();
}
