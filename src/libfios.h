// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#include <stddef.h>
#endif

#ifdef FIOS_SHARED_LIBRARY
 #ifdef _WIN32
  #define FIOS_API __declspec(dllexport)
 #else
  #define FIOS_API __attribute__((visibility("default")))
 #endif
#else
 #define FIOS_API
#endif

#define CMD_SIZE 2 /* 'w ' */ + 10 /* 0xffffffff */ + 1 /* null */
#define MAX_PAYLOAD_SIZE 0x2000

// --------------------------------------------------------------------------------------------------------------------

typedef struct _fios_serial_t fios_serial_t;

FIOS_API
fios_serial_t* fios_serial_open(const char* devpath);

FIOS_API
void fios_serial_close(fios_serial_t* s);

FIOS_API
bool fios_serial_read_cmd(fios_serial_t* s, char cmd[CMD_SIZE]);

FIOS_API
bool fios_serial_read_payload(fios_serial_t* s, void* payload, size_t size);

// NOTE null-terminated string, maximum CMD_SIZE
FIOS_API
bool fios_serial_write_cmd(fios_serial_t* s, const char* cmd);

FIOS_API
bool fios_serial_write_payload(fios_serial_t* s, const void* payload, size_t size);

// --------------------------------------------------------------------------------------------------------------------

typedef struct _fios_file_t fios_file_t;

FIOS_API
fios_file_t* fios_file_receive(fios_serial_t* s, const char* outpath);

FIOS_API
fios_file_t* fios_file_send(fios_serial_t* s, const char* inpath);

FIOS_API
bool fios_file_idle(fios_file_t* s, float* progress);

FIOS_API
bool fios_file_close(fios_file_t* s);

// --------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
