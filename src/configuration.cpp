//
// Created by Nick on 10/19/2021.
//

#include "configuration.h"

configuration::configuration() {
    this->eve_installation = std::string("");
    this->use_console = false;
    this->use_fsmapper = false;
}

void configuration::save() {
    FILE *f = fopen(LOADER_CONFIG, "w");
    fprintf(f, "[eveloader]\n");
    fprintf(f, "eve_installation = %s\n", this->eve_installation.c_str());
    fprintf(f, "use_console = %s\n", this->use_console ? "true" : "false");
    fprintf(f, "use_fsmapper = %s\n", this->use_fsmapper ? "true" : "false");
    fflush(f);
    fclose(f);
}

void configuration::load() {
    INIReader r = INIReader(LOADER_CONFIG);
    if (r.ParseError() != 0) {
        MessageBoxA(NULL, "an error occured while parsing configuration file.  Please check it's correctness.", "eveloader2 - Parse Error", MB_OK);
        exit(-2);
    }

    this->eve_installation = r.Get("Launcher", "eve_installation", "C:\\Crucible\\");
    this->use_console = r.GetBoolean("Launcher", "use_console", false);
    this->use_fsmapper = r.GetBoolean("Launcher", "use_fsmapper", true);
}

configuration cfg;
