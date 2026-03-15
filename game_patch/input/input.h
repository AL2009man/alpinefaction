#pragma once

#include "../rf/player/control_config.h"

struct SDL_Window;

rf::ControlConfigAction get_af_control(rf::AlpineControlConfigAction alpine_control);
rf::String get_action_bind_name(int action);
void mouse_apply_patch();
void mouse_init_sdl_window();
SDL_Window* mouse_get_sdl_window();
void mouse_on_sdl_motion(float xrel, float yrel);
void sdl_input_poll();
void key_apply_patch();
