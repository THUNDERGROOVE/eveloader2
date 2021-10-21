//
// Created by Nick on 10/20/2021.
//

#ifndef EVELOADER2_PYLOADER_H
#define EVELOADER2_PYLOADER_H

#include <Python.h>
#include <Stackless/stackless.h>
#include <Stackless/core/stackless_impl.h>
#include <frameobject.h>

#define LOADPY(name) EVE ## name = (_EVE ## name)GetProcAddress(py, #name);



void *memmem(const void *l, size_t l_len, const void *s, size_t s_len);
void load_py_symbols();

#endif //EVELOADER2_PYLOADER_H
