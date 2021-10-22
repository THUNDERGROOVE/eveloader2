//
// Created by Nick on 10/21/2021.
//

#ifndef EVELOADER2_EVELOADER_UTIL_HPP
#define EVELOADER2_EVELOADER_UTIL_HPP

#include <string>
#include "eveloader.h"

#include <shlobj.h>

static void *memmem(const void *l, size_t l_len, const void *s, size_t s_len) {
    char *cur, *last;
    const char *cl = (const char *)l;
    const char *cs = (const char *)s;

    if (l_len == 0 || s_len == 0)
        return NULL;

    if (l_len < s_len)
        return NULL;

    if (s_len == 1)
        return (void *)memchr(l, (int)*cs, l_len);

    last = (char *)cl + l_len - s_len;

    for (cur = (char *)cl; cur <= last; cur++)
        if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0)
            return cur;

    return NULL;
}

// compiler is requiring this forward declaration
SHFOLDERAPI SHGetFolderPathA(_Reserved_ HWND hwnd, _In_ int csidl, _In_opt_ HANDLE hToken, _In_ DWORD dwFlags, _Out_writes_(MAX_PATH) LPSTR pszPath);

static std::string get_overlay_path() {
    char path[MAX_PATH] = {0};
    SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
    std::string o = std::string(path);
    o.append(OVERLAY_PATH);
    return o;
}


#endif //EVELOADER2_EVELOADER_UTIL_HPP
