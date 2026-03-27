#include "sdl_dynapi.h"
#include <windows.h>
#include <xlog/xlog.h>
#include <common/utils/os-utils.h>
#include "../main/main.h"

void sdl_dynapi_init()
{
    char existing[MAX_PATH];
    if (GetEnvironmentVariableA("SDL3_DYNAMIC_API", existing, sizeof(existing)) != 0) {
        xlog::info("SDL Dynamic API: SDL3_DYNAMIC_API already set to '{}'", existing);
        return;
    }

    // Search for SDL3.dll next to the AF DLL first, then next to the host process (launcher).
    for (const auto& dir : {get_module_dir(g_hmodule), get_module_dir(nullptr)}) {
        std::string path = dir + "SDL3.dll";
        if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES)
            continue;
        SetEnvironmentVariableA("SDL3_DYNAMIC_API", path.c_str());
        xlog::info("SDL Dynamic API: override activated via '{}'", path);
        return;
    }

    xlog::info("SDL Dynamic API: no SDL3.dll override found, using built-in SDL");
}
