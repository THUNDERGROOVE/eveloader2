//
// Created by Nick on 10/19/2021.
//

#ifndef EVELOADER2_EVELOADER_H
#define EVELOADER2_EVELOADER_H

#include <stdint.h>
#include <windows.h>

#define EVE_STARTUP_FLAG_CONSOLE = 1 << 1
#define EVE_STARTUP_FLAG_NOFSMAP = 1 << 2

#define LOADER_CONFIG "config.ini"

struct eve_startup {
    uint64_t flags;
    uint16_t alt_port;
    char launcher_path[MAX_PATH];
    char username[64];
    char password[64];
    char host[128];
};

#endif //EVELOADER2_EVELOADER_H
