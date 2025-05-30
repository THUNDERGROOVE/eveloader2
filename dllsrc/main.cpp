//
// Created by Nick on 10/19/2021.
//

#include <easyhook/easyhook.h>

#include "pyloader.h"
#include "pydecls.h"
#include "eveloader.h"
#include "hook_manager.h"
#include "hooks.h"

#include <loguru/loguru.hpp>
#include <eveloader_util.hpp>
#include <conio.h>

extern "C" void __declspec(dllexport) __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo);

static void run_script(const char *script) {
    if (script == nullptr) {
        LOG_F(WARNING, "run_script called with null script value");
    }

    // @TODO(np): properly dispose of these values.
    PyGILState_STATE s = EVEPyGILState_Ensure();
    PyObject *main = EVEPyImport_AddModule("__main__");
    PyObject *main_dict = EVEPyModule_GetDict(main);
    PyObject *startup = EVEPy_CompileStringFlags(script, "<eveloader2>", Py_file_input, nullptr);

    // @TODO(np): properly dispose of these values.
    PyObject *p_type, *p_value, *p_traceback;

    if (startup == nullptr) {
        LOG_F(WARNING, "run_script call to Py_CompileStringFlags failed");
        PyErr_Fetch(&p_type, &p_value, &p_traceback);
        if (p_value != nullptr) {
           char *err = PyString_AsString(p_value);
           EVEPyErr_Print();
           LOG_F(WARNING, "Python error message: %s", err);
        }
    } else {
        EVEPyEval_EvalCode((PyCodeObject *)startup, main_dict, main_dict);
        PyErr_Fetch(&p_type, &p_value, &p_traceback);
        if (p_value != nullptr) {
            char *err = PyString_AsString(p_value);
            EVEPyErr_Print();
            LOG_F(WARNING, "Python error message: %s", err);
        }
    }

    EVEPyGILState_Release(s);
}

static void start_console() {
    while (true) {
        LOG_F(INFO, "Starting python console");
        run_script("import stackless\nimport code\nstackless.tasklet(code.interact)()\nstackless.run()\n");
        PyErr_Clear();
    }
}

void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* in_remote_info) {
    std::string log_path = get_overlay_path();
    log_path.append("\\eveloader2_dll.log");
    // @TODO(NP): Save in ProgramFiles dir
    loguru::add_file(log_path.c_str(), loguru::Truncate, loguru::Verbosity_INFO);
    SETUP_LOGURU_HANDLER
    if (in_remote_info->UserDataSize != sizeof(eve_startup)) {
        MessageBoxA(nullptr, "UserDataSize != sizeof(eve_startup)\nlikely DLL/loader version mismatch.", "Error", MB_OK);
        exit(-100);
    }
    eve_startup *startup = (eve_startup *)in_remote_info->UserData;
    hooks = new hook_manager();


    if ((startup->flags & EVE_STARTUP_FLAG_NOFSMAP) == 0) {
        LOG_F(INFO, "Installing LoadLibraryA(FSMapper) hook!");
        hooks->install_hook(LoadLibraryA, _LoadLibraryA, "LoadLibraryA(FSMapper)");
        LOG_F(INFO, "Installing CreateFileW(FSMapper) hook!");
        hooks->install_hook(CreateFileW, _CreateFileW, "CreateFileW(FSMapper)");
    }

    if ((startup->flags & EVE_STARTUP_FLAG_DISABLE_CRYPTO) != 0) {
        LOG_F(INFO, "Installing CryptEncrypt(disable_crypto) hook!");
        hooks->install_hook(CryptEncrypt, _CryptEncrypt, "CryptEncrypt(disable_crypto)");
        LOG_F(INFO, "Installing CryptDecrypt(disable_crypto) hook!");
        hooks->install_hook(CryptDecrypt, _CryptDecrypt, "CryptEncrypt(disable_crypto)");
    }

    //Sleep(2000);
    LOG_F(INFO, "Loading python symbols.");
    load_py_symbols();
    if ((startup->flags & EVE_STARTUP_FLAG_DEBUG_WAIT) != 0) {
        LOG_F(INFO, "Delaying start to allow debugger attach\n");
        system("pause");
    }

    RhWakeUpProcess();

    // TODO(NP): Find a way to verify that the game has been initialized enough to proceed.
    Sleep(6000); // The magic number for most people, I hope!

    if ((startup->flags & EVE_STARTUP_FLAG_DO_BOOT_SCRIPT) != 0) {
        FILE *f = fopen(startup->boot_script_path, "r");
        if (f == nullptr) {
            LOG_F(FATAL, "Failed to open boot script at %s", startup->boot_script_path);
        }

        fseek(f, 0, SEEK_END);
        size_t s = ftell(f);
        rewind(f);

        char *data = (char *)calloc(1, s+1);
        fread(data, 1, s, f);
        fclose(f);

        LOG_F(INFO, "Running boot script at %s", startup->boot_script_path);
        run_script(data);
    }
    if ((startup->flags & EVE_STARTUP_FLAG_CONSOLE) != 0) {
        start_console();
    }


    return;
}
