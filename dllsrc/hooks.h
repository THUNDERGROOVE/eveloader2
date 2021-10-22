//
// Created by Nick on 10/21/2021.
//

#ifndef EVELOADER2_HOOKS_H
#define EVELOADER2_HOOKS_H

#include <Windows.h>

HMODULE WINAPI _LoadLibraryA(const char *filename);

HANDLE WINAPI _CreateFileW(LPCWSTR name, DWORD access, DWORD shareMode, LPSECURITY_ATTRIBUTES sec, DWORD disp, DWORD flags, HANDLE templ);

#endif //EVELOADER2_HOOKS_H
