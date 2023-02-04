//
// Created by Nick on 10/19/2021.
//

#ifndef EVELOADER2_EVELOADER_H
#define EVELOADER2_EVELOADER_H

#include <stdint.h>
#include <windows.h>

#define EVE_STARTUP_FLAG_CONSOLE        ((uint64_t)1 << 1)
#define EVE_STARTUP_FLAG_NOFSMAP        ((uint64_t)1 << 2)
#define EVE_STARTUP_FLAG_DO_BOOT_SCRIPT ((uint64_t)1 << 3)
#define EVE_STARTUP_FLAG_DEBUG_WAIT ((uint64_t)1 << 4)
#define EVE_STARTUP_FLAG_DISABLE_CRYPTO ((uint64_t)1 << 5)

#define LOADER_CONFIG "config.ini"
#define OVERLAY_PATH "\\eveloader2"

struct eve_startup {
    uint64_t flags;
    uint16_t alt_port;
    char launcher_path[MAX_PATH];
    char username[64];
    char password[64];
    char host[128];
    char boot_script_path[MAX_PATH];
};

#define SETUP_LOGURU_HANDLER                                                                             \
    loguru::set_fatal_handler([](const loguru::Message &message){                                        \
    std::string msg = std::string();                                                                     \
    msg.append(message.filename);                                                                        \
    msg.append(":");                                                                                     \
    msg.append(std::to_string(message.line));                                                            \
    msg.append("\n");                                                                                    \
    msg.append(message.message);                                                                         \
    MessageBoxA(nullptr, msg.c_str(), "eveloader2 - A fatal error has occurred", MB_OK | MB_ICONERROR);  \
    });



#endif //EVELOADER2_EVELOADER_H
