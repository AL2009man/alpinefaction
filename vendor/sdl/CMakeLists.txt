# Tracking the SDL3 release-3.4.x branch.
# Read the sdl wiki for more info about CMake-configurations: https://wiki.libsdl.org/SDL3/README-cmake

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG        e759cc850e29d8aa8e13e0cdec66fe285f54ba89
)

set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)

if(MSVC)
    set(SDL_PRESEED                 OFF CACHE BOOL     "" FORCE)
    set(HAVE_WINDOWS_GAMING_INPUT_H 0   CACHE INTERNAL "" FORCE)
endif()

FetchContent_MakeAvailable(SDL3)