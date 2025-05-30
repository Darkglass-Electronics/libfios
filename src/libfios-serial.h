// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#pragma once

#include "libfios.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
typedef void* HANDLE;
const char* GetLastErrorString(short error);
#endif

typedef struct _fios_serial_t {
    char* devpath;
   #ifdef _WIN32
    HANDLE h;
   #else
    int fd;
   #endif
} fios_serial_t;

#ifdef __cplusplus
}
#endif
