//
// Created by Nick on 10/21/2021.
//

#include "eveloader_util.hpp"

#include "blue_patcher.h"
#include "crc32.h"
#include "configuration.h"
#include "overlay.h"
#include "patches.h"

#include <loguru/loguru.hpp>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <filesystem>

#define CRUCIBLE_BLUE_DLL_CRC32 0x965BDFFD
#define CRUCIBLE_BLUE_STSCHAKE_DLL_CRC32 0x5DEB44AE

uint32_t get_crc32(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (f == nullptr) {
        LOG_F(ERROR, "get_crc32 failed to open file %s", filename);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    size_t s = ftell(f);
    rewind(f);
    char *data = (char *)calloc(1, s);
    fread(data, 1, s, f);
    uint32_t cur_crc = rhash_get_crc32(0, (uint8_t *)data, s);
    fclose(f);
    free(data);
    return cur_crc;
}

static bool is_our_blue_patch() {
    std::string blue_path = get_blue_dll_path();
    FILE *f = fopen(blue_path.c_str(), "rb");
    if (f == nullptr) {
        LOG_F(ERROR, "unable to open blue.dll for reading");
        exit(-202);
    }

    fseek(f, 0, SEEK_END);
    size_t s = ftell(f);
    rewind(f);
    char *data = (char *)calloc(1, s);
    fread(data, 1, s, f);
    fclose(f);

    patch *p = patches;

    while (p->name != nullptr) {
        char *loc = (char *)memmem(data, s, p->updated, p->size);
        if (loc == nullptr) {
            LOG_F(INFO, "is_our_blue_patch returning false due to missing patch: %s", p->name);
            free(data);
            return false;
        }
        p++;
    }
    free(data);
    return true;
}

static void patch_blue() {
    std::string blue_path = get_blue_dll_path();

    FILE *f = fopen(blue_path.c_str(), "rb");
    if (f == nullptr) {
        LOG_F(ERROR, "Failed to open %s for reading", blue_path.c_str());
        exit(-1);
    }

    fseek(f, 0, SEEK_END);
    size_t s = ftell(f);
    rewind(f);
    char *data = (char *)calloc(1, s);
    fread(data, 1, s, f);
    fclose(f);

    patch *p = patches;

    while (p->name != nullptr) {
        char *loc = (char *)memmem(data, s, p->original, p->size);
        if (loc == nullptr) {
            void *t = memmem(data, s, p->updated, p->size);
            if (p != nullptr) {
                p++;
                continue;
            }

            MessageBoxA(NULL, "Failed to find memory pattern for patch\neveloader2 aborting", p->name, MB_OK);
            LOG_F(ERROR, "Unable to find memory pattern for patch %s", p->name);
            exit(-1);
        } else {
            memcpy(loc, p->updated, p->size);
            LOG_F(INFO, "Patch OK %s", p->name);
        }
        p++;
    }

    FILE *b = fopen(blue_path.c_str(), "wb");

    if (f == nullptr) {
        LOG_F(ERROR, "Failed to open %s for reading", blue_path.c_str());
        exit(-1);
    }

    fwrite(data, 1, s, b);
    fflush(b);
    fclose(b);
}

void check_blue_patch() {
    std::string blue_path = get_blue_dll_path();

    if (!std::filesystem::exists(blue_path)) {
        LOG_F(ERROR, "check_blue_patch was unable to find blue.dll");
        exit(-201);
    }

    uint32_t blue_crc = get_crc32(blue_path.c_str());

    if (blue_crc == CRUCIBLE_BLUE_DLL_CRC32) {
        LOG_F(INFO, "check_blue_patch blue.dll detected to be stock.");
        if (!cfg.use_fsmapper) {
            int res = MessageBoxA(
                    NULL,
                    "FSMapper is not enabled and blue.dll isn't patched.\nDo you want eveloader2 to patch this for you?\neveloader2 will make a backup of this file.\nIf you select no eveloader2 may not work.",
                    "Patch?",
                    MB_YESNO);
            if (res == IDNO) {
                LOG_F(INFO, "eveloader2 user chose not patch blue.dll");
                return;
            }
            patch_blue();
        } else {
            LOG_F(INFO, "eveloader2 is automatically patching blue.dll because FSMapper is enabled.");
            patch_blue();
        }
    } else if (blue_crc == CRUCIBLE_BLUE_STSCHAKE_DLL_CRC32) {
        LOG_F(ERROR, "check_blue_patch blue.dll detected to be patched with stschake/blue_patcher");
        MessageBoxA(
                NULL,
                "eveloader2 has detected that blue.dll was patched with stschake/blue_patcher\nPlease restore a clean version of blue.dll before proceeding\neveloader2 will shut down now.",
                "Error",
                MB_OK
                );
        exit(-204);
    } else if (is_our_blue_patch()) {
        LOG_F(INFO, "check_blue_patch blue.dll detected to be eveloader2 patched");
    } else {
        LOG_F(ERROR, "check_blue_patch couldn't determine blue.dll type");
        MessageBoxA(
                NULL,
                "eveloader2 was unable to detected the type of blue.dll patch\nPlease restore a clean version of blue.dll before proceeding\neveloader2 will shut down now.",
                "Error",
                MB_OK
        );
        exit(-203);
    }
}
