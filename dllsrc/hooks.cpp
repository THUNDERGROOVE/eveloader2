//
// Created by Nick on 10/21/2021.
//

#include "hooks.h"
#include "eveloader_util.hpp"

#include <loguru/loguru.hpp>

HMODULE WINAPI _LoadLibraryA(const char *libname) {
    std::string overlay_path = get_overlay_path();

    if (strcmp(libname, "blue.dll") == 0) {
        overlay_path.append("\\bin\\blue.dll");
        LOG_F(INFO, "Overloading DLL %s -> %s\n", libname, overlay_path.c_str());
        return LoadLibraryA(overlay_path.c_str());
    } else {
        return LoadLibraryA(libname);
    }

    return nullptr;
}
