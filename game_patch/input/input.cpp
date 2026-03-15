// Central SDL event pump for all input subsystems.
#include <SDL3/SDL.h>
#include "input.h"

void sdl_input_poll()
{
    if (!mouse_get_sdl_window())
        return;

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_EVENT_MOUSE_MOTION:
                mouse_on_sdl_motion(ev.motion.xrel, ev.motion.yrel);
                break;
        }
    }
}
