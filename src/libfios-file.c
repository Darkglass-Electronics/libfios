// SPDX-FileCopyrightText: 2024-2025 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#include "libfios-serial.h"

#undef NDEBUG
#define DEBUG
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

#define DEBUG_PRINT(...)
// #define DEBUG_PRINT(...) printf(__VA_ARGS__)

typedef struct _fios_file_t {
    fios_serial_t* serial;
    FILE* file;
    const char* error;
   #ifdef _WIN32
    HANDLE thread;
   #else
    pthread_t thread;
   #endif
    long current, size;
    fios_file_status_t status;
} fios_file_t;

#ifdef _WIN32
static unsigned __stdcall _fios_thread_close(fios_file_t* const f)
#else
static void* _fios_thread_close(fios_file_t* const f)
#endif
{
    FILE* const file = f->file;

    if (file != NULL)
    {
        f->file = NULL;
        fclose(file);
    }

   #ifdef _WIN32
    _endthreadex(0);
    return 0;
   #else
    return NULL;
   #endif
}

#ifdef _WIN32
static unsigned __stdcall _fios_receive_thread(void* const arg)
#else
static void* _fios_receive_thread(void* const arg)
#endif
{
    fios_file_t* const f = arg;
    fios_serial_t* const s = f->serial;

    char buf[MAX_PAYLOAD_SIZE];
    char cmd[CMD_SIZE];
    bool test;

    DEBUG_PRINT("waiting for size\n");

    if (! fios_serial_read_cmd(s, cmd))
    {
        fprintf(stderr, "size read failed, forcing reopen of serial port now!\n");

        char* const devpath = s->devpath;
        s->devpath = NULL;
        fios_serial_close(s);
        f->serial = fios_serial_open(devpath);
        free(devpath);

        if (! fios_serial_read_cmd(s, cmd))
        {
            f->error = "serial port reopen failed";
            f->status = fios_file_status_error;
            fprintf(stderr, "serial port reopen failed!\n");
            return _fios_thread_close(f);
        }
    }

    if (cmd[0] != 's' || cmd[1] != ' ')
    {
        f->error = "unexpected data received (invalid first command)";
        f->status = fios_file_status_error;
        fprintf(stderr, "error invalid command type %02x:'%c' %02x:'%c'\n", cmd[0], cmd[0], cmd[1], cmd[1]);
        return _fios_thread_close(f);
    }

    cmd[CMD_SIZE - 1] = 0;

    // size comes as 2nd arg
    const long int size = strtol(cmd + 2, NULL, 16);

    if (cmd[0] != 's' || cmd[1] != ' ')
    {
        f->error = "unexpected data received (invalid size)";
        f->status = fios_file_status_error;
        fprintf(stderr, "invalid file size %ld\n", size);
        return _fios_thread_close(f);
    }

    DEBUG_PRINT("file size %ld\n", size);

    f->size = size;

    while (f->file != NULL && f->status != fios_file_status_error && f->current != size)
    {
        DEBUG_PRINT("waiting for command\n");

        test = fios_serial_read_cmd(s, cmd);
        assert(test);

        if (cmd[0] == 'q' && cmd[1] == 0)
            break;

        if (cmd[0] != 'w' || cmd[1] != ' ')
        {
            f->error = "unexpected data received (invalid command)";
            f->status = fios_file_status_error;
            fprintf(stderr, "error invalid command type %02x:'%c' %02x:'%c'\n", cmd[0], cmd[0], cmd[1], cmd[1]);
            break;
        }

        cmd[CMD_SIZE - 1] = 0;

        // size comes as 2nd arg
        long int size = strtol(cmd + 2, NULL, 16);

        DEBUG_PRINT("waiting for payload of size %ld | 0x%08lx\n", size, size);
        test = fios_serial_read_payload(s, buf, size);
        assert(test);

        DEBUG_PRINT("payload received, sending ok back\n");
        test = fios_serial_write_cmd(s, "ok");
        assert(test);

        // write received buffer to file
        for (int w = 0, total = 0; total < size; total += w)
        {
            w = fwrite(buf + total, 1, size - total, f->file);

            if (w < 0)
            {
                f->error = "failed to write to output file";
                f->status = fios_file_status_error;
                perror("error partial write");
                break;
            }
        }

        f->current += size;
    }

    if (f->file != NULL && f->status != fios_file_status_error && f->current == size)
        f->status = fios_file_status_completed;

    DEBUG_PRINT("_fios_receive_thread done\n");
    return _fios_thread_close(f);
}

#ifdef _WIN32
static unsigned __stdcall _fios_send_thread(void* const arg)
#else
static void* _fios_send_thread(void* const arg)
#endif
{
    fios_file_t* const f = arg;
    fios_serial_t* const s = f->serial;

    char buf[MAX_PAYLOAD_SIZE];
    char cmd[CMD_SIZE];
    bool test;

    DEBUG_PRINT("writing size for %ld | 0x%lx bytes\n", f->size, f->size);

    // encode size command as first byte, followed by size
    snprintf(cmd, CMD_SIZE, "s 0x%08lx", f->size);

    test = fios_serial_write_cmd(s, cmd);
    assert(test);

    while (f->file != NULL)
    {
        const int r = fread(buf, 1, MAX_PAYLOAD_SIZE, f->file);

        DEBUG_PRINT("main file read return %d | 0x%x bytes\n", r, r);

        if (r <= 0)
            break;

        DEBUG_PRINT("writing command for %d | 0x%x bytes\n", r, r);

        // encode write command as first byte, followed by expected size, and then the payload
        snprintf(cmd, CMD_SIZE, "w 0x%08x", r);

        test = fios_serial_write_cmd(s, cmd);
        assert(test);

        DEBUG_PRINT("writing payload for %d | 0x%x bytes\n", r, r);
        test = fios_serial_write_payload(s, buf, r);
        assert(test);

        DEBUG_PRINT("waiting for ok signal for %d | 0x%x bytes\n", r, r);
        test = fios_serial_read_cmd(s, cmd);
        // assert(test);

        if (! test)
        {
            f->error = "serial port writing failed";
            f->status = fios_file_status_error;
            fprintf(stderr, "serial port writing failed!\n");
            return _fios_thread_close(f);
        }

        f->current += r;
    }

    f->status = fios_file_status_completed;

    DEBUG_PRINT("writing command for close\n");
    test = fios_serial_write_cmd(s, "q");
    assert(test);

    DEBUG_PRINT("_fios_send_thread done\n");
    return _fios_thread_close(f);
}

fios_file_t* fios_file_receive(fios_serial_t* const s, const char* const outpath)
{
    fios_file_t* const f = malloc(sizeof(fios_file_t));

    if (f == NULL)
    {
        fprintf(stderr, "fios: out of memory\n");
        return NULL;
    }

    FILE* const file = fopen(outpath, "wb");

    if (file == NULL)
    {
        fprintf(stderr, "fios: failed to open file '%s' for writing, error %d: %s\n", outpath, errno, strerror(errno));
        goto error_free;
    }

    fseek(file, 0, SEEK_SET);

    f->serial = s;
    f->file = file;
    f->error = NULL;
    f->current = f->size = 0;
    f->status = fios_file_status_in_progress;

   #ifdef _WIN32
    f->thread = (HANDLE)_beginthreadex(NULL, 0, _fios_receive_thread, f, 0, NULL);
    if (f->thread == NULL)
    {
        fprintf(stderr, "fios: failed to create sender thread, error %d: %s\n",
                GetLastError(), GetLastErrorString(GetLastError()));
        goto error_close;
    }
   #else
    if (pthread_create(&f->thread, NULL, _fios_receive_thread, f) != 0)
    {
        fprintf(stderr, "fios: failed to create sender thread, error %d: %s\n", errno, strerror(errno));
        goto error_close;
    }
   #endif

    return f;

error_close:
    fclose(file);

error_free:
    free(f);
    return NULL;
}

fios_file_t* fios_file_send(fios_serial_t* const s, const char* const inpath)
{
    fios_file_t* const f = malloc(sizeof(fios_file_t));

    if (f == NULL)
    {
        fprintf(stderr, "fios: out of memory\n");
        return NULL;
    }

    FILE* const file = fopen(inpath, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "fios: failed to open file '%s' for reading, error %d: %s\n", inpath, errno, strerror(errno));
        goto error_free;
    }

    fseek(file, 0, SEEK_END);
    const long size = ftell(file);

    if (size > MAX_FILE_SIZE)
    {
        fprintf(stderr, "fios: file is too big! must be < 2GiB\n");
        goto error_close;
    }

    fseek(file, 0, SEEK_SET);

    f->serial = s;
    f->file = file;
    f->error = NULL;
    f->current = 0;
    f->size = size > 0 ? size : 0;
    f->status = fios_file_status_in_progress;

   #ifdef _WIN32
    f->thread = (HANDLE)_beginthreadex(NULL, 0, _fios_send_thread, f, 0, NULL);
    if (f->thread == NULL)
    {
        fprintf(stderr, "fios: failed to create sender thread, error %d: %s\n",
                GetLastError(), GetLastErrorString(GetLastError()));
        goto error_close;
    }
   #else
    if (pthread_create(&f->thread, NULL, _fios_send_thread, f) != 0)
    {
        fprintf(stderr, "fios: failed to create sender thread, error %d: %s\n", errno, strerror(errno));
        goto error_close;
    }

    pthread_detach(f->thread);
   #endif

    return f;

error_close:
    fclose(file);

error_free:
    free(f);
    return NULL;
}

fios_file_status_t fios_file_idle(fios_file_t* const f, float* const progress)
{
    assert(f != NULL);

    if (progress != NULL)
        *progress = f->size != 0 ? (double)f->current / f->size : 0.f;

    return f->status;
}

float fios_file_get_progress(fios_file_t* const f)
{
    assert(f != NULL);

    return f->size != 0 ? (double)f->current / f->size : 0.f;
}

void fios_file_close(fios_file_t* const f)
{
    assert(f != NULL);

    FILE* const file = f->file;

    if (file != NULL)
    {
        f->file = NULL;
        fclose(file);

        if (f->status != fios_file_status_error)
        {
           #ifdef _WIN32
            WaitForSingleObject(f->thread, INFINITE);
            CloseHandle(f->thread);
           #endif
        }
    }

    free(f);
}

const char* fios_file_get_last_error(fios_file_t* const f)
{
    assert(f != NULL);

    return f->error != NULL ? f->error : "no error";
}
