# SDL3 built from source via FetchContent:
# Tracking the SDL main branch. To pin to a release, switch back to URL/URL_HASH.
# See: https://wiki.libsdl.org/SDL3/README-cmake

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG        3c02de2f7cd4c42eaa71f9a0ad9e7d9dfa26b000
)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)