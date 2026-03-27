#pragma once

// Must be called before the first SDL API call.
// Detects SDL3.dll alongside the AF module or host process and activates
// the SDL3_DYNAMIC_API override so SDL's jump table redirects through it.
void sdl_dynapi_init();
