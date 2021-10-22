#pragma once

// These are modified in easyhook and I don't want the redef errors
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#include <easyhook/easyhook.h>


#include <vector>
#include <Windows.h>
#include <Psapi.h>

#define DYNLOAD(name, module, pat, patlen, toval)\
  _log->Logf(" >> Checking if %s is exported from module %s!\n", name, module);\
  toval = (_ ## toval)GetProcAddress(GetModuleHandleA(module), name);\
  if (toval == NULL) {\
    _log->Logf(" >> %s Not exported!\n", name);\
    _log->Logf("Finding unexported symbol for %s.  Wish me luck\n", name);\
    MODULEINFO modinfo;\
    memset(&modinfo, 0, sizeof(modinfo));\
    if (GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(module), &modinfo, sizeof(modinfo)) == 0) { \
      _log->Logf("Failed to Get MODULEINFO for %s\n", module);\
    } else {\
      _log->Logf("Got MODULEINFO for %s!\n", module);\
    }\
    char *start = (char *)modinfo.lpBaseOfDll;\
    int size = modinfo.SizeOfImage;\
    _log->Logf("%s Start %p size %d\n", module,  start, size);\
    void *symbol = memmem(start, size, pat, patlen);\
    if (symbol == NULL) {\
      _log->Logf("Failed to get %s from dynamic lookup\n", name);\
    } else {\
      _log->Logf("Got %s from dynamic lookup\n", name);\
      toval = (_ ## toval)symbol;\
    }\
  }

// The Hook struct keeps track of each individual hook.
struct f_hook {
	HOOK_TRACE_INFO trace_info;
	void *original_call;
	void *hooked_call;
	char *name;
	char *mod;
};

// HookManager keeps track of all installed hooks
class hook_manager {
public:

	std::vector<f_hook> hooks;
	int install_hook(void *entry_point, void *hook, const char *name = "unk");
	int install_module_hook(const char *module, const char *entry_point_name, void *hook);
	int install_pattern_hook(const char *module, const unsigned char *pattern, uint32_t pattern_size, void *hook, const char *name);
};

extern hook_manager *hooks;
