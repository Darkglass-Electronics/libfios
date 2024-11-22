// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#include <stddef.h>
#endif

#ifdef _WIN32
typedef void* HANDLE;
#endif

#ifdef USC_SHARED_LIBRARY
 #ifdef _WIN32
  #define USC_API __declspec(dllexport)
 #else
  #define USC_API __attribute__((visibility("default")))
 #endif
#else
 #define USC_API
#endif

typedef struct {
   #ifdef _WIN32
    HANDLE h;
   #else
    int fd;
   #endif
} usc_serial_t;

#define CMD_SIZE 2 /* 'w ' */ + 10 /* 0xffffffff */ + 1 /* ' ' */ + 1 /* null */
#define MAX_PAYLOAD_SIZE 0x2000

USC_API
usc_serial_t* usc_serial_open(const char* devpath);

USC_API
void usc_serial_close(usc_serial_t* s);

USC_API
bool usc_serial_read_cmd(usc_serial_t* s, char cmd[CMD_SIZE]);

USC_API
bool usc_serial_read_payload(usc_serial_t* s, void* payload, size_t size);

// NOTE null-terminated string, maximum CMD_SIZE
USC_API
bool usc_serial_write_cmd(usc_serial_t* s, const char* cmd);

USC_API
bool usc_serial_write_payload(usc_serial_t* s, const void* payload, size_t size);

#ifdef __cplusplus
}
#endif
