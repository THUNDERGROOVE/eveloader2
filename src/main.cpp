#include <iostream>

#include <easyhook/easyhook.h>
#include <filesystem>

#include <shlobj.h>

#include "eveloader.h"
#include "configuration.h"


static int CALLBACK browse_callback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    if(uMsg == BFFM_INITIALIZED) {
        //std::string tmp = (const char *) lpData;
        //std::cout << "path: " << tmp << std::endl;
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }

    return 0;
}

std::string find_eve_install() {
    char path[MAX_PATH];
    BROWSEINFOA bi = {0};
    bi.lpszTitle = "Browse for EVE installation...";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = browse_callback;
    bi.lParam = (LPARAM)nullptr;

    LPITEMIDLIST id_list = SHBrowseForFolderA(&bi);

    if (id_list != 0) {
        SHGetPathFromIDList(id_list, path);
    }

    return std::string(path);
}

void run_initial_setup() {
    int res = MessageBoxA(nullptr,
            "no config.ini detected.  Would you like to run the interactive setup?",
            "eveloader2 - First run",
            MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL);
    if (res == IDNO) {
        exit(-1);
    }

    cfg.eve_installation = find_eve_install();

    int res2 = MessageBoxA(nullptr,
                           "Would you like to enable the interactive console?\nThis option is intended for developers",
                           "eveloader2 - First run",
                           MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL);
    if (res2 == IDYES) {
        cfg.use_console = true;
    } else {
        cfg.use_console = false;
    }

    int res3 = MessageBoxA(nullptr,
                           "Would you like to enable the filesystem overlay system?\nIt is recommended that this option is enabled.",
                           "eveloader2 - First run",
                           MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL);

    if (res3 == IDYES) {
        cfg.use_fsmapper = true;
    } else {
        cfg.use_fsmapper = false;
    }

    cfg.save();
}

int main() {
    if (!std::filesystem::exists(LOADER_CONFIG)) {
        run_initial_setup();
    } else {
        cfg.load();
    }

    return 0;
}

