//
// Created by Nick on 10/19/2021.
//

#include <easyhook/easyhook.h>


#include "pyloader.h"
#include "pydecls.h"
#include "eveloader.h"

#include <loguru/loguru.cpp>

extern "C" void __declspec(dllexport) __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo);


static PyObject *run_script(const char *script) {
    if (script == nullptr) {
        LOG_F(WARNING, "run_script called with null script value");
        return nullptr;
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
    // @TODO(NP): Save in ProgramFiles dir
    loguru::add_file("eveloader2_dll.log", loguru::Truncate, loguru::Verbosity_INFO);
    if (in_remote_info->UserDataSize != sizeof(eve_startup)) {
        MessageBoxA(nullptr, "UserDataSize != sizeof(eve_startup)\nlikely DLL/loader version mismatch.", "Error", MB_OK);
        exit(-100);
    }
    eve_startup *startup = (eve_startup *)in_remote_info->UserData;

    //Sleep(2000);
    LOG_F(INFO, "Loading python symbols.");
    load_py_symbols();

    RhWakeUpProcess();
    // TODO(NP): Find a way to verify that the game has been initialized enough to proceed.
    Sleep(6000); // The magic number for most people, I hope!
    start_console();

    return;
}
