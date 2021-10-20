//
// Created by Nick on 10/19/2021.
//

#include <easyhook/easyhook.h>

extern "C" void __declspec(dllexport) __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo);

void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* in_remote_info) {

}
