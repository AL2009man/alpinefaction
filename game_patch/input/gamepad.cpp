#include "gamepad.h"
#include <algorithm>
#include <optional>
#include <patch_common/FunHook.h>
#include <xlog/xlog.h>
#include "../os/console.h"
#include "../rf/input.h"
#include "../rf/player/control_config.h"
#include <SDL3/SDL.h>

static SDL_Gamepad* g_gamepad = nullptr;
static bool g_btn_prev[SDL_GAMEPAD_BUTTON_COUNT] = {};
static bool g_btn_curr[SDL_GAMEPAD_BUTTON_COUNT] = {};
static float g_look_sensitivity = 5.0f;
static float g_deadzone = 0.25f;

static float apply_deadzone(float v, float dz)
{
    if (v >  dz) return (v - dz) / (1.0f - dz);
    if (v < -dz) return (v + dz) / (1.0f - dz);
    return 0.0f;
}

static float get_stick_axis(SDL_GamepadAxis axis)
{
    if (!g_gamepad) return 0.0f;
    return apply_deadzone(SDL_GetGamepadAxis(g_gamepad, axis) / (float)SDL_MAX_SINT16, g_deadzone);
}

static float get_trigger_axis(SDL_GamepadAxis axis)
{
    if (!g_gamepad) return 0.0f;
    float v = SDL_GetGamepadAxis(g_gamepad, axis) / (float)SDL_MAX_SINT16;
    return v > 0.05f ? v : 0.0f;
}

// Returns the SDL button index for a stock action, or -1 if unmapped.
static int action_to_button(rf::ControlConfigAction action)
{
    switch (action) {
        case rf::CC_ACTION_PRIMARY_ATTACK:  return SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER; // R1
        case rf::CC_ACTION_JUMP:            return SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;  // L1
        case rf::CC_ACTION_USE:             return SDL_GAMEPAD_BUTTON_SOUTH;          // Cross
        case rf::CC_ACTION_RELOAD:          return SDL_GAMEPAD_BUTTON_NORTH;          // Triangle
        case rf::CC_ACTION_NEXT_WEAPON:     return SDL_GAMEPAD_BUTTON_EAST;           // Circle
        case rf::CC_ACTION_PREV_WEAPON:     return SDL_GAMEPAD_BUTTON_WEST;           // Square
        case rf::CC_ACTION_HIDE_WEAPON:     return SDL_GAMEPAD_BUTTON_DPAD_LEFT;      // D-pad left
        case rf::CC_ACTION_MESSAGES:        return SDL_GAMEPAD_BUTTON_DPAD_RIGHT;     // D-pad right
        default:                            return -1;
    }
}

static bool gamepad_action_is_down(rf::ControlConfigAction action)
{
    if (!g_gamepad) return false;

    // Triggers → alt fire (R2) / crouch (L2)
    if (action == rf::CC_ACTION_SECONDARY_ATTACK)
        return get_trigger_axis(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.5f;
    if (action == rf::CC_ACTION_CROUCH)
        return get_trigger_axis(SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > 0.5f;

    // Left stick → movement
    float lx = get_stick_axis(SDL_GAMEPAD_AXIS_LEFTX);
    float ly = get_stick_axis(SDL_GAMEPAD_AXIS_LEFTY);
    if (action == rf::CC_ACTION_FORWARD)      return ly < -0.5f;
    if (action == rf::CC_ACTION_BACKWARD)     return ly >  0.5f;
    if (action == rf::CC_ACTION_SLIDE_LEFT)   return lx < -0.5f;
    if (action == rf::CC_ACTION_SLIDE_RIGHT)  return lx >  0.5f;

    int btn = action_to_button(action);
    return btn >= 0 && g_btn_curr[btn];
}

static bool gamepad_action_just_pressed(rf::ControlConfigAction action)
{
    if (!g_gamepad) return false;
    int btn = action_to_button(action);
    return btn >= 0 && g_btn_curr[btn] && !g_btn_prev[btn];
}

static void try_open_gamepad(SDL_JoystickID id)
{
    g_gamepad = SDL_OpenGamepad(id);
    if (g_gamepad)
        xlog::info("Gamepad connected: {}", SDL_GetGamepadName(g_gamepad));
    else
        xlog::warn("Failed to open gamepad: {}", SDL_GetError());
}

static void gamepad_update()
{
    // Snapshot the previous frame's button state before processing new events.
    std::copy(std::begin(g_btn_curr), std::end(g_btn_curr), std::begin(g_btn_prev));

    // SDL_UpdateGamepads() reads hardware state and pushes gamepad events into SDL's
    // internal queue without needing to pump the event system.
    SDL_UpdateGamepads();

    // Drain all pending gamepad events. SDL_PeepEvents is thread-safe and, unlike
    // SDL_PollEvent, does NOT call SDL_PumpEvents — safe to call on RF's game thread.
    SDL_Event ev;
    while (SDL_PeepEvents(&ev, 1, SDL_GETEVENT,
                          SDL_EVENT_GAMEPAD_AXIS_MOTION,
                          SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED) > 0) {
        switch (ev.type) {
        case SDL_EVENT_GAMEPAD_ADDED:
            if (!g_gamepad)
                try_open_gamepad(ev.gdevice.which);
            break;
        case SDL_EVENT_GAMEPAD_REMOVED:
            if (g_gamepad && SDL_GetGamepadID(g_gamepad) == ev.gdevice.which) {
                xlog::info("Gamepad disconnected");
                SDL_CloseGamepad(g_gamepad);
                g_gamepad = nullptr;
                // Clear button state so no inputs remain stuck after disconnect.
                std::fill(std::begin(g_btn_curr), std::end(g_btn_curr), false);
            }
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            if (g_gamepad && SDL_GetGamepadID(g_gamepad) == ev.gbutton.which
                && ev.gbutton.button < SDL_GAMEPAD_BUTTON_COUNT)
                g_btn_curr[ev.gbutton.button] = true;
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            if (g_gamepad && SDL_GetGamepadID(g_gamepad) == ev.gbutton.which
                && ev.gbutton.button < SDL_GAMEPAD_BUTTON_COUNT)
                g_btn_curr[ev.gbutton.button] = false;
            break;
        }
    }
}

// Hooked into rf::mouse_get_delta (0x0051E630) to drive the gamepad update once per frame
// and inject right-stick look deltas into the existing mouse delta pipeline.
FunHook<void(int&, int&, int&)> mouse_get_delta_hook{
    0x0051E630,
    [](int& dx, int& dy, int& dz) {
        mouse_get_delta_hook.call_target(dx, dy, dz);
        gamepad_update();

        // START button → inject a KEY_ESC press/release to open the in-game menu.
        const bool start_curr = g_gamepad && g_btn_curr[SDL_GAMEPAD_BUTTON_START];
        const bool start_prev = g_btn_prev[SDL_GAMEPAD_BUTTON_START];
        if (start_curr && !start_prev) {
            rf::key_process_event(rf::KEY_ESC, 1, 0);
            rf::key_process_event(rf::KEY_ESC, 0, 0);
        }
        if (g_gamepad && rf::keep_mouse_centered) {
            float rx = get_stick_axis(SDL_GAMEPAD_AXIS_RIGHTX);
            float ry = get_stick_axis(SDL_GAMEPAD_AXIS_RIGHTY);
            const float scale = g_look_sensitivity;
            dx += static_cast<int>(rx * scale);
            dy += static_cast<int>(ry * scale);
        }
    },
};

// Extend held-button detection to include gamepad inputs.
FunHook<bool(rf::ControlConfig*, rf::ControlConfigAction)> control_is_control_down_hook{
    0x00430F40,
    [](rf::ControlConfig* ccp, rf::ControlConfigAction action) -> bool {
        return control_is_control_down_hook.call_target(ccp, action)
            || gamepad_action_is_down(action);
    },
};

// Extend just-pressed detection to include gamepad inputs.
FunHook<bool(rf::ControlConfig*, rf::ControlConfigAction, bool*)> control_config_check_pressed_hook{
    0x0043D4F0,
    [](rf::ControlConfig* ccp, rf::ControlConfigAction action, bool* just_pressed) -> bool {
        bool result = control_config_check_pressed_hook.call_target(ccp, action, just_pressed);
        if (!result && gamepad_action_just_pressed(action)) {
            if (just_pressed) *just_pressed = true;
            return true;
        }
        return result;
    },
};

ConsoleCommand2 gp_sens_cmd{
    "gp_sens",
    [](std::optional<float> val) {
        if (val) g_look_sensitivity = std::max(0.1f, val.value());
        rf::console::print("Gamepad look sensitivity: {:.2f}", g_look_sensitivity);
    },
    "Set gamepad look sensitivity (default 5.0)",
    "gp_sens [value]",
};

ConsoleCommand2 gp_deadzone_cmd{
    "gp_deadzone",
    [](std::optional<float> val) {
        if (val) g_deadzone = std::clamp(val.value(), 0.0f, 0.9f);
        rf::console::print("Gamepad stick deadzone: {:.2f}", g_deadzone);
    },
    "Set gamepad stick deadzone 0.0-0.9 (default 0.25)",
    "gp_deadzone [value]",
};

void gamepad_apply_patch()
{
    // Allow gamepad input even when the RF window is not in the foreground
    // (needed for exclusive fullscreen and alt-tab scenarios).
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        xlog::error("Failed to initialize SDL gamepad subsystem: {}", SDL_GetError());
        return;
    }

    // Open any gamepad already connected at startup
    if (SDL_HasGamepad()) {
        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        if (ids) {
            for (int i = 0; i < count; ++i)
                xlog::info("Gamepad found: {}", SDL_GetGamepadNameForID(ids[i]));
            if (count > 0)
                try_open_gamepad(ids[0]);
            SDL_free(ids);
        }
    }

    mouse_get_delta_hook.install();
    control_is_control_down_hook.install();
    control_config_check_pressed_hook.install();
    gp_sens_cmd.register_cmd();
    gp_deadzone_cmd.register_cmd();
    xlog::info("Gamepad support initialized");
}
