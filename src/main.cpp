#include <iostream>

#include <easyhook/easyhook.h>
#include <loguru/loguru.cpp>
#include <loguru/loguru.hpp>

#include "eveloader.h"
#include "eveloader_util.hpp"

#include "configuration.h"
#include "overlay.h"
#include "blue_patcher.h"

#include <filesystem>
#include <vector>

#include <assert.h>
#include <shlobj.h>

static bool parse_argument(const char *argument, char **value) {
    for (int i = 0; i < __argc; i++) {
        char *t = __argv[i];
        if (strcmp(argument, t) == 0) {
            if (__argc >= i + 1) {
                *value = __argv[i + 1];
            } else {
                *value = NULL;
            }
            return true;
        }
    }

    return false;
}

static bool has_argument(const char *argument) {
    for (int i = 0; i < __argc; i++) {
        char *t = __argv[i];
        if (strcmp(argument, t) == 0) {
            return true;
        }
    }
    return false;
}


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

    cfg.use_console = false;
    /*
    int res2 = MessageBoxA(nullptr,
                           "Would you like to enable the interactive console?\nThis option is intended for developers",
                           "eveloader2 - First run",
                           MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL);
    if (res2 == IDYES) {
        cfg.use_console = true;
    } else {
        cfg.use_console = false;
    }
     */

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

std::wstring to_wide (const std::string &multi) {
    std::wstring wide; wchar_t w; mbstate_t mb {};
    size_t n = 0, len = multi.length () + 1;
    while (auto res = mbrtowc (&w, multi.c_str () + n, len - n, &mb)) {
        if (res == size_t (-1) || res == size_t (-2))
            throw "invalid encoding";

        n += res;
        wide += w;
    }
    return wide;
}


void runtime_validation() {
    std::string exefile_path = std::string(cfg.eve_installation);
    exefile_path.append("\\bin\\exefile.exe");

    std::string dll_path = std::string("eveloader2_dll.dll");

    assert(std::filesystem::exists(cfg.eve_installation.c_str()));
    assert(std::filesystem::exists(exefile_path));
    assert(std::filesystem::exists(dll_path.c_str()));
}

int start_eve_client(char *host, char *username, char *password) {
    char loader_path[MAX_PATH] = {0};
    GetCurrentDirectoryA(MAX_PATH, loader_path);
    std::string dll_path = std::string(loader_path);
    dll_path.append("\\eveloader2_dll.dll");

    PROCESS_INFORMATION proc_info;
    memset(&proc_info, 0, sizeof(proc_info));

    STARTUPINFOA start_info;
    memset(&start_info, 0, sizeof(start_info));
    start_info.cb = sizeof(start_info);

    std::vector<std::string> arguments;

    if (cfg.use_console) {
        arguments.push_back("/console");
    } else {
        arguments.push_back("/noconsole");
    }

    if (username != nullptr && password != nullptr) {
        std::string login_arg = std::string(" /login:");
        login_arg.append(username);
        login_arg.append(":");
        login_arg.append(password);

        arguments.push_back(login_arg);
    }

    if (host != nullptr) {
        char host_buffer[128] = {0};
        snprintf(host_buffer, 127, " /server:%s", host);
        arguments.push_back(std::string(host_buffer));
    }

    std::string joined_args = std::string();
    for (int i = 0; i < arguments.size(); i++) {
        joined_args.append(arguments[i]);
        //joined_args.append("");
    }

    std::string exe_path = cfg.eve_installation;
    exe_path.append("\\bin\\exefile.exe");

    eve_startup startup = {0};

    if (cfg.use_console) {
        startup.flags |= EVE_STARTUP_FLAG_CONSOLE;
    }

    if (!cfg.use_fsmapper) {
        startup.flags |= EVE_STARTUP_FLAG_NOFSMAP;
    }

    if (std::filesystem::exists("boot.py")) {
        LOG_F(INFO, "boot.py found, setting as boot script.");
        startup.flags |= EVE_STARTUP_FLAG_DO_BOOT_SCRIPT;
        GetFullPathNameA("boot.py", MAX_PATH, startup.boot_script_path, NULL);
    }

    NTSTATUS status = RhCreateAndInject(
            (wchar_t *)to_wide(exe_path).c_str(),
            (wchar_t *)to_wide(joined_args).c_str(),
            0,
            EASYHOOK_INJECT_DEFAULT,
            (wchar_t *)to_wide(dll_path).c_str(),
            nullptr,
            &startup,
            sizeof(startup),
            &proc_info.dwProcessId);

    if (status != 0) {
        auto err = RtlGetLastErrorString();
        MessageBoxW(NULL, err, L"ERROR", MB_OK);
        DebugBreak();
        return -1;
    }

    return 0;
}

int main() {
    std::string log_path = get_overlay_path();
    log_path.append("\\eveloader2.log");
    loguru::add_file(log_path.c_str(), loguru::Truncate, loguru::Verbosity_INFO);
    loguru::g_colorlogtostderr = false;
    SETUP_LOGURU_HANDLER

    LOG_F(INFO, "eveloader2 is starting.");
    std::string overlay_path = get_overlay_path();
    LOG_F(INFO, "fsmapper overlay path found to be %s", overlay_path.c_str());
    if (!std::filesystem::exists(LOADER_CONFIG)) {
        LOG_F(INFO, "%s is missing, triggering initial setup", LOADER_CONFIG);
        run_initial_setup();
    } else {
        LOG_F(INFO, "Loading %s", LOADER_CONFIG);
        cfg.load();
    }

    ensure_overlay_setup();

    runtime_validation();
    check_blue_patch();

    char *host = nullptr;
    char *username = nullptr;
    char *password = nullptr;

    //char *port = nullptr;
    parse_argument("-h", &host);
    parse_argument("-u", &username);
    parse_argument("-p", &password);
    //parse_argument("-port", &port);

    start_eve_client(host, username, password);

    return 0;
}

