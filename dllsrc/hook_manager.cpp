#include "hook_manager.h"

#include <Psapi.h>
#include <eveloader_util.hpp>
#include <loguru/loguru.hpp>


hook_manager *hooks = NULL;

int hook_manager::install_hook(void *entry_point, void *hook, const char *name) {
	ULONG ACLEntries[1] = { 0 };
	f_hook h = {0 };
	h.original_call = entry_point;
	h.hooked_call = hook;
	h.name = "unk";
	h.mod = "exefile.exe";
	NTSTATUS status = LhInstallHook(entry_point, hook, NULL, &h.trace_info);
	if (FAILED(status)) {
		LOG_F(ERROR, "Failed to install hook\n");
		return status;
	}

	LhSetExclusiveACL(ACLEntries, 1, &h.trace_info);
	return 0;
}

int hook_manager::install_module_hook(const char *module, const char *entry_point_name, void *hook) {
	ULONG ACLEntries[1] = { 0 };
	f_hook h = {0 };
	HMODULE mod = GetModuleHandleA(module);

	h.original_call = GetProcAddress(mod, entry_point_name);
	h.hooked_call = hook;
	h.name = (char *)entry_point_name;
	h.mod = _strdup(module);

	NTSTATUS status = LhInstallHook(h.original_call, hook, NULL, &h.trace_info);
	if (FAILED(status)) {
		LOG_F(ERROR, "Failed installing hook %s\n", entry_point_name);
		return status;
	}

	LhSetExclusiveACL(ACLEntries, 1, &h.trace_info);
	return 0;
}

int hook_manager::install_pattern_hook(const char *module, const unsigned char *pattern, uint32_t pattern_size, void *hook, const char *name) {
	ULONG ACLEntries[1] = { 0 };
	f_hook h = {0 };

	HMODULE mod = GetModuleHandleA(module);
	if (mod == NULL) {
		return -1;
	}

	MODULEINFO modinfo;
	memset(&modinfo, 0, sizeof(modinfo));

	h.name = "unk";
	h.mod = _strdup(module);
	if (GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(module), &modinfo, sizeof(modinfo)) == 0) {
		return -1;
	}

	char *start = (char *)modinfo.lpBaseOfDll;
	int size = modinfo.SizeOfImage;

	void *symbol = memmem(start, size, pattern, pattern_size);
	if (symbol == NULL) {
		return -1;
	}

	h.original_call = symbol;
	h.hooked_call = hook;
	NTSTATUS status = LhInstallHook(h.original_call, hook, NULL, &h.trace_info);
	if (FAILED(status)) {
		return status;
	}

	LhSetExclusiveACL(ACLEntries, 1, &h.trace_info);
	return 0;
}
