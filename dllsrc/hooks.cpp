//
// Created by Nick on 10/21/2021.
//

#include "hooks.h"
#include "eveloader_util.hpp"

#include <loguru/loguru.hpp>
#include <algorithm>
#include <filesystem>
#include <wincrypt.h>

HMODULE WINAPI _LoadLibraryA(const char *libname) {
    std::string overlay_path = get_overlay_path();

    if (strcmp(libname, "blue.dll") == 0) {
        overlay_path.append("\\bin\\blue.dll");
        LOG_F(INFO, "Overloading DLL %s -> %s\n", libname, overlay_path.c_str());
        return LoadLibraryA(overlay_path.c_str());
    } else {
        return LoadLibraryA(libname);
    }

    return nullptr;
}

static const char *mapped_files[] = {
        "script/compiled.code",
        "script/devtools.code",
        "start.ini",
        "common.ini",
        "lib/carbonlib.ccp",
        "lib/carbonstdlib.ccp",
        "lib/evelib.ccp",
        NULL,
};

bool mapped_found[] = { false, false, false, false, false, false, false };

std::string wide_tostr(LPCWSTR pwsz) {
    int cch = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, 0, 0, NULL, NULL);

    char *psz = new char[cch];

    WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, cch, NULL, NULL);

    std::string st(psz);
    delete[] psz;

    return st;
}

HANDLE WINAPI _CreateFileW(LPCWSTR name, DWORD access, DWORD shareMode, LPSECURITY_ATTRIBUTES sec, DWORD disp, DWORD flags, HANDLE templ) {
    int i = 0;
    const char *t = mapped_files[i];

    std::string n = wide_tostr(name);

    while (true) {
        std::string _path(t);
        std::replace(_path.begin(), _path.end(), '\\', '/');
        std::replace(n.begin(), n.end(), '\\', '/');

        if (n.find(_path) == std::string::npos) {
            i++;
            t = mapped_files[i];

            if (t == nullptr) {
                break;
            }
            continue;
        }

        std::string overlay_path = get_overlay_path();
        overlay_path.append("/");
        overlay_path.append(t);
        if (!std::filesystem::exists(overlay_path)) {
            LOG_F(INFO, "eveloader2 had the opportunity to overlay file %s however it didn't exist in the overlay at path %s", n.c_str(), overlay_path.c_str());
            return CreateFileW(name, access, shareMode, sec, disp, flags, templ);
        }

        LOG_F(INFO, "Overloading file %s -> %s\n", n.c_str(), overlay_path.c_str());
        return CreateFileW(std::wstring(overlay_path.begin(), overlay_path.end()).c_str(), access, shareMode, sec, disp, flags, templ);
    }

    return CreateFileW(name, access, shareMode, sec, disp, flags, templ);
}

BOOL _CryptEncrypt(
      HCRYPTKEY  hKey,
      HCRYPTHASH hHash,
      BOOL       Final,
      DWORD      dwFlags,
      BYTE       *pbData,
      DWORD      *pdwDataLen,
      DWORD      dwBufLen
) {
    LOG_F(INFO, "_CryptEncrypt hit!\n");
    *pdwDataLen = dwBufLen;
    return TRUE;
}

BOOL _CryptDecrypt(
      HCRYPTKEY  hKey,
      HCRYPTHASH hHash,
      BOOL       Final,
      DWORD      dwFlags,
      BYTE       *pbData,
      DWORD      *pdwDataLen
) {
    LOG_F(INFO, "_CryptDecrypt hit!\n");
    return TRUE;
};