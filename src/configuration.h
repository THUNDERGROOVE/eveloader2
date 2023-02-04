//
// Created by Nick on 10/19/2021.
//

#ifndef EVELOADER2_CONFIGURATION_H
#define EVELOADER2_CONFIGURATION_H

#include "eveloader.h"
#include "INIReader.h"
#include <string>

struct configuration {
    configuration();
    void save();
    void load();

    std::string eve_installation;
    bool use_console;
    bool use_fsmapper;
    bool debug_wait;
    bool disable_crypto;
};

extern configuration cfg;

#endif //EVELOADER2_CONFIGURATION_H
