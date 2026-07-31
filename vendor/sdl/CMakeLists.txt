# Tracks SDL3 Master branch for testing Steam Controller 2 features
# To update, change GIT_TAG to a commit hash from that branch.
# Read the sdl wiki for more info about CMake-configurations: https://wiki.libsdl.org/SDL3/README-cmake

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG        "53b485c7aa873af8d1dd53466d1f050c1f374b96"
)

set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
set(SDL_PRESEED                  OFF CACHE BOOL     "" FORCE)

# Disable all subsystems except those required for gamepad/joystick input
set(SDL_AUDIO        OFF CACHE BOOL "" FORCE)
set(SDL_VIDEO        OFF CACHE BOOL "" FORCE)
set(SDL_GPU          OFF CACHE BOOL "" FORCE)
set(SDL_RENDER       OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA       OFF CACHE BOOL "" FORCE)
set(SDL_POWER        OFF CACHE BOOL "" FORCE)
set(SDL_DIALOG       OFF CACHE BOOL "" FORCE)
set(SDL_TRAY         OFF CACHE BOOL "" FORCE)
set(SDL_NOTIFICATION OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SDL3)
