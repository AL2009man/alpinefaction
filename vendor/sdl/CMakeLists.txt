# Tracks SDL3 Master branch for testing Steam Controller 2 features
# To update, change GIT_TAG to a commit hash from that branch.
# Read the sdl wiki for more info about CMake-configurations: https://wiki.libsdl.org/SDL3/README-cmake

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG        "6243fa2ffd8de2cf310ec4851eb0d9faef83b2fa"
)

set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
set(SDL_PRESEED                  OFF CACHE BOOL     "" FORCE)
FetchContent_MakeAvailable(SDL3)