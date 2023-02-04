//
// Created by nick on 1/29/2023.
//
#include "patches.h"
#include "INIReader.h"
#include <string>
#include <sstream>
#include <Windows.h>
#include <assert.h>


std::vector<std::string> split(std::string text, char delim) {
    std::string line;
    std::vector<std::string> vec;
    std::stringstream ss(text);
    while(std::getline(ss, line, delim)) {
        vec.push_back(line);
    }
    return vec;
}

#define HasFile(name) !(INVALID_FILE_ATTRIBUTES == GetFileAttributes(name) && GetLastError() == ERROR_FILE_NOT_FOUND)

char *load_file(const char *filename, int *out_size) {
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    rewind(f);
    char *out = (char *)calloc(size, sizeof(char));
    *out_size = size;
    fread(out, size, sizeof(char), f);
    fclose(f);

    return out;
}

patch load_patch(INIReader reader, std::string patch) {
    std::string name = reader.Get(patch, "name", "");
    std::string original = reader.Get(patch, "original_data", "");
    std::string updated = reader.Get(patch, "patched_data", "");
    if (!HasFile(original.c_str())) {
        printf(" >> while loading patch %s we couldn't find original_data file at %s\n", patch.c_str(), original.c_str());
    }
    if (!HasFile(original.c_str())) {
        printf(" >> while loading patch %s we couldn't find patched_data file at %s\n", patch.c_str(), updated.c_str());
    }

    struct patch p;
    int osize = 0;
    int usize = 0;
    p.original = load_file(original.c_str(), &osize);
    p.updated = load_file(updated.c_str(), &usize);
    p.name = strdup(name.c_str());
    assert(osize == usize);
    p.size = osize;
    return p;
}

std::vector<patch> load_patches_ini() {
    std::vector<patch> out;
    if (!HasFile("patches.ini")) {
        return out;
    }
    INIReader r("patches.ini");

    if (r.ParseError() != 0) {
        MessageBoxA(NULL, "an error occured while parsing patches file.  Please check it's correctness.", "eveloader2 - Parse Error", MB_OK);
        exit(-2);
    }

    std::string patch_list = r.Get("patches", "patch_list", "");
    std::vector<std::string> patches = split(patch_list, ',');
    for (int i = 0; i < patches.size(); i++) {
        std::string patch = patches[i];
        struct patch p = load_patch(r, patch);
        out.push_back(p);
    }


    return out;
}

