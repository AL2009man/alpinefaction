# To update SDL3, change SDL3_COMMIT below to the desired commit SHA from the SDL master branch.
# Commit history: https://github.com/libsdl-org/SDL/commits/main
# Read the sdl wiki for more info about CMake-configurations: https://wiki.libsdl.org/SDL3/README-cmake

set(SDL3_COMMIT "5e483dc16626b937f24bc10c90dedf8e1e92003e")

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        ${SDL3_COMMIT}
    GIT_SHALLOW    FALSE
)

set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)