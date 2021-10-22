//
// Created by Nick on 10/21/2021.
//

#include "overlay.h"
#include "eveloader.h"
#include "configuration.h"
#include "eveloader_util.hpp"

#include <loguru/loguru.hpp>

#include <Windows.h>
#include <shlobj.h>

#include <filesystem>


std::string get_blue_dll_path() {
    std::string base_path = std::string();
    if (cfg.use_fsmapper) {
        base_path = get_overlay_path();
    } else {
        base_path = cfg.eve_installation;
    }

    base_path.append("\\bin\\blue.dll");

    return base_path;
}

void ensure_overlay_setup() {
    std::string overlay_path = get_overlay_path();

    std::string overlay_logs_path = overlay_path;
    std::string overlay_bin_path = overlay_path;
    std::string overlay_script_path = overlay_path;

    std::string overlay_blue_dll = overlay_path;
    std::string base_blue_dll = cfg.eve_installation;

    overlay_logs_path.append("\\logs\\");
    overlay_bin_path.append("\\bin\\");
    overlay_script_path.append("\\script\\");

    overlay_blue_dll.append("\\bin\\blue.dll");
    base_blue_dll.append("\\bin\\blue.dll");

    if (CreateDirectoryA(overlay_path.c_str(), nullptr) || ERROR_ALREADY_EXISTS == GetLastError()) {
        LOG_F(INFO, "fsmapper overlay path exists");
    } else {
        LOG_F(INFO, "Unable to create fsmapper overlay path.");
        exit(-1);
    }


    if (CreateDirectoryA(overlay_logs_path.c_str(), nullptr) || ERROR_ALREADY_EXISTS == GetLastError()) {
        LOG_F(INFO, "fsmapper overlay logs path exists");
    } else {
        LOG_F(INFO, "Unable to create fsmapper overlay logs path.");
        exit(-1);
    }

    if (CreateDirectoryA(overlay_bin_path.c_str(), nullptr) || ERROR_ALREADY_EXISTS == GetLastError()) {
        LOG_F(INFO, "fsmapper overlay bin path exists");
    } else {
        LOG_F(INFO, "Unable to create fsmapper overlay bin path.");
        exit(-1);
    }

    if (CreateDirectoryA(overlay_script_path.c_str(), nullptr) || ERROR_ALREADY_EXISTS == GetLastError()) {
        LOG_F(INFO, "fsmapper overlay script path exists");
    } else {
        LOG_F(INFO, "Unable to create fsmapper overlay script path.");
        exit(-1);
    }

    if (!cfg.use_fsmapper) {
        return;
    }

    if (!std::filesystem::exists(base_blue_dll)) {
        LOG_F(ERROR, "while verifying overlay status, blue.dll found to be missing from EVE installation at %s", base_blue_dll.c_str());
        MessageBoxA(NULL, "while verifying overlay status, blue.dll found to be missing from EVE installation", "ERROR", MB_OK);
        exit(-1);
    }

    if (!std::filesystem::exists(overlay_blue_dll.c_str())) {
        LOG_F(INFO, "overlay was missing blue.dll.  Installing from EVE installation.");
        CopyFileA(base_blue_dll.c_str(), overlay_blue_dll.c_str(), false);
    }
}

