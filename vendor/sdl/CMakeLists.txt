# Using a custom branch for WASAPI investigation.
# To switch back to a release, replace the GIT_REPOSITORY/GIT_TAG/GIT_SHALLOW lines with:
#   URL "https://github.com/libsdl-org/SDL/releases/download/release-<version>/SDL3-<version>.tar.gz"
#   URL_HASH SHA256=<hash>
#   DOWNLOAD_EXTRACT_TIMESTAMP FALSE
# Read the sdl wiki for more info about CMake-configurations: https://wiki.libsdl.org/SDL3/README-cmake

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/AL2009man/SDL.git
    GIT_TAG        wasapi-investigation
    GIT_SHALLOW    TRUE
)

set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)