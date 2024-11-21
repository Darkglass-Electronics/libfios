// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#include "libusc.h"

#undef NDEBUG
#define DEBUG
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    unsigned long flag;
    const char* str;
} libusc_flag_str_t;

#if _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#endif

#define DEBUG_PRINT(...)
// #define DEBUG_PRINT(...) printf(__VA_ARGS__)
// #include <stdio.h>

#ifndef _WIN32

#define STR(s) #s

#ifndef IUCLC
#define IUCLC 0001000
#endif

#ifndef OLCUC
#define OLCUC 0000002
#endif

#ifndef XCASE
#define XCASE 0000004
#endif

#ifndef CMSPAR
#define CMSPAR 010000000000
#endif

#define LIBUSC_SUPPORTED_TERMIOS_IFLAGS (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK|ISTRIP|INLCR|IGNCR|ICRNL|IUCLC|IXON|IXANY|IXOFF|IUTF8)
static const libusc_flag_str_t k_termios_iflags[] = {
    { LIBUSC_SUPPORTED_TERMIOS_IFLAGS, "" },
    { IGNBRK, STR(IGNBRK) },   /* Ignore break condition.  */
    { BRKINT, STR(BRKINT) },   /* Signal interrupt on break.  */
    { IGNPAR, STR(IGNPAR) },   /* Ignore characters with parity errors.  */
    { PARMRK, STR(PARMRK) },   /* Mark parity and framing errors.  */
    { INPCK, STR(INPCK) },     /* Enable input parity check.  */
    { ISTRIP, STR(ISTRIP) },   /* Strip 8th bit off characters.  */
    { INLCR, STR(INLCR) },     /* Map NL to CR on input.  */
    { IGNCR, STR(IGNCR) },     /* Ignore CR.  */
    { ICRNL, STR(ICRNL) },     /* Map CR to NL on input.  */
    { IUCLC, STR(IUCLC) },     /* Map uppercase characters to lowercase on input (not in POSIX).  */
    { IXON, STR(IXON) },       /* Enable start/stop output control.  */
    { IXANY, STR(IXANY) },     /* Enable any character to restart output.  */
    { IXOFF, STR(IXOFF) },     /* Enable start/stop input control.  */
    { IMAXBEL, STR(IMAXBEL) }, /* Ring bell when input queue is full (not in POSIX). */
    { IUTF8, STR(IUTF8) },     /* Input is UTF8 (not in POSIX).  */
};

#define LIBUSC_SUPPORTED_TERMIOS_OFLAGS (OPOST|OLCUC|ONLCR|OCRNL|ONOCR|ONLRET|OFILL|OFDEL)
static const libusc_flag_str_t k_termios_oflags[] = {
    { LIBUSC_SUPPORTED_TERMIOS_OFLAGS, "" },
    { OPOST, STR(OPOST) },   /* Post-process output.  */
    { OLCUC, STR(OLCUC) },   /* Map lowercase characters to uppercase on output. (not in POSIX).  */
    { ONLCR, STR(ONLCR) },   /* Map NL to CR-NL on output.  */
    { OCRNL, STR(OCRNL) },   /* Map CR to NL on output.  */
    { ONOCR, STR(ONOCR) },   /* No CR output at column 0.  */
    { ONLRET, STR(ONLRET) }, /* NL performs CR function.  */
    { OFILL, STR(OFILL) },   /* Use fill characters for delay.  */
    { OFDEL, STR(OFDEL) },   /* Fill is DEL.  */
};


#define LIBUSC_SUPPORTED_TERMIOS_LFLAGS (ISIG|ICANON|XCASE|ECHO|ECHOE|ECHOK|ECHONL|NOFLSH|TOSTOP|ECHOCTL|ECHOPRT|ECHOKE|FLUSHO|IEXTEN|EXTPROC)
static const libusc_flag_str_t k_termios_lflags[] = {
    { LIBUSC_SUPPORTED_TERMIOS_LFLAGS, "" },
    { ISIG, STR(ISIG) },       /* Enable signals.  */
    { ICANON, STR(ICANON) },   /* Canonical input (erase and kill processing).  */
    { XCASE, STR(XCASE) },
    { ECHO, STR(ECHO) },       /* Enable echo.  */
    { ECHOE, STR(ECHOE) },     /* Echo erase character as error-correcting backspace.  */
    { ECHOK, STR(ECHOK) },     /* Echo KILL.  */
    { ECHONL, STR(ECHONL) },   /* Echo NL.  */
    { NOFLSH, STR(NOFLSH) },   /* Disable flush after interrupt or quit.  */
    { TOSTOP, STR(TOSTOP) },   /* Send SIGTTOU for background output.  */
    { ECHOCTL, STR(ECHOCTL) }, /* If ECHO is also set, terminal special characters other than TAB, NL, START, and STOP are echoed as ^X,
                                  where X is the character with ASCII code 0x40 greater than the special character (not in POSIX). */
    { ECHOPRT, STR(ECHOPRT) }, /* If ICANON and ECHO are also set, characters are printed as they are being erased (not in POSIX). */
    { ECHOKE, STR(ECHOKE) },   /* If ICANON is also set, KILL is echoed by erasing each character on the line,
                                  as specified by ECHOE and ECHOPRT (not in POSIX). */
    { FLUSHO, STR(FLUSHO) },   /* Output is being flushed.
                                  This flag is toggled by typing the DISCARD character (not in POSIX).  */
    { IEXTEN, STR(IEXTEN) },   /* All characters in the input queue are reprinted when the next character is read (not in POSIX). */
    { EXTPROC, STR(EXTPROC) }, /* Enable implementation-defined input processing.  */
};

#define LIBUSC_SUPPORTED_TERMIOS_CFLAGS (CSIZE|CSTOPB|CREAD|PARENB|PARODD|HUPCL|CLOCAL|CMSPAR|CRTSCTS)
static const libusc_flag_str_t k_termios_cflags[] = {
    { LIBUSC_SUPPORTED_TERMIOS_CFLAGS, "" },
    { CS6, STR(CS6) },
    { CS7, STR(CS7) },
    { CS8, STR(CS8) },
    { CSTOPB, STR(CSTOPB) },
    { CREAD, STR(CREAD) },
    { PARENB, STR(PARENB) },
    { PARODD, STR(PARODD) },
    { HUPCL, STR(HUPCL) },
    { CLOCAL, STR(CLOCAL) },
    { CMSPAR, STR(CMSPAR) },
    { CRTSCTS, STR(CRTSCTS) },
};

#define print_flags(n, o, f) _print_flags(n, o, f, sizeof(f)/sizeof(f[0]))
static void _print_flags(const char* const name, const unsigned optflags, const libusc_flag_str_t flags[], const unsigned numflags)
{
    fprintf(stderr, "libusb: debug termios %s: ", name);
    bool found = false;
    for (unsigned i = 1; i < numflags; ++i)
    {
        if ((optflags & flags[i].flag) == flags[i].flag)
        {
            if (found)
                fprintf(stderr, "| ");
            else
                found = true;

            fprintf(stderr, "%s ", flags[i].str);
        }
    }
    if (! found)
        fprintf(stderr, "(none)");
    fprintf(stderr, "\n");

    unsigned test = optflags;
    test &= ~flags[0].flag;
    if (test != 0)
        fprintf(stderr, "libusb: unknown termios %s 0o%o\n", name, test);
}
#endif

usc_serial_t* usc_serial_open(const char* const devpath)
{
    usc_serial_t* const s = malloc(sizeof(usc_serial_t));

    if (s == NULL)
        return NULL;

#ifdef _WIN32
    const HANDLE h = CreateFile(devpath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (h == NULL || h == INVALID_HANDLE_VALUE)
        goto error_free;

    DCB params = { 0 };
    params.DCBlength = sizeof(params);

    BOOL ret;
    ret = GetCommState(h, &params);
    assert(ret != FALSE);

    params.BaudRate = CBR_115200;
    params.fBinary = TRUE;
    params.fParity = FALSE;
    params.fOutxCtsFlow = FALSE;
    params.fOutxDsrFlow = FALSE;
    params.fDtrControl = DTR_CONTROL_DISABLE;
    params.fDsrSensitivity = FALSE;
    params.fTXContinueOnXoff = FALSE;
    params.fOutX = FALSE;
    params.fInX = FALSE;
    params.fErrorChar = FALSE;
    params.fNull = FALSE;
    params.fRtsControl = RTS_CONTROL_DISABLE;
    params.fAbortOnError = FALSE;
    params.XonLim = 0;
    params.XoffLim = 0;
    params.ByteSize = 8;
    params.Parity = NOPARITY;
    params.StopBits = ONESTOPBIT;
    params.XonChar = 0;
    params.XoffChar = 0;
    params.ErrorChar = 0;
    params.EofChar = 0;
    params.EvtChar = 0;

    ret = SetCommState(h, &params);
    assert(ret != FALSE);

    COMMTIMEOUTS timeouts = { 0 };

    ret = SetCommTimeouts(h, &timeouts);
    assert(ret != FALSE);

    ret = PurgeComm(h, PURGE_RXCLEAR);
    assert(ret != FALSE);

    ret = PurgeComm(h, PURGE_TXCLEAR);
    assert(ret != FALSE);

    s->h = h;
#else
    const int fd = open(devpath, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        fprintf(stderr, "libusb: failed to open serial port device '%s', error %d: %s\n", devpath, errno, strerror(errno));
        goto error_free;
    }

    struct termios options = { 0 };
    tcgetattr(fd, &options);

    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    fprintf(stderr,
            "libusb: debug termios config: c_iflag 0o%lo, c_oflag 0o%lo, c_lflag 0o%lo, c_cflag 0o%lo\n",
            (unsigned long)options.c_iflag,
            (unsigned long)options.c_oflag,
            (unsigned long)options.c_lflag,
            (unsigned long)options.c_cflag);

    // do not modify input
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IUCLC | IXON | IXANY | IXOFF | IMAXBEL | IUTF8);
    options.c_iflag |= IGNPAR;

    // do not modify output
    options.c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL | ONLRET | OFILL);
    options.c_oflag |= ONOCR;

    // testing
    options.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL | TOSTOP | ECHOCTL | ECHOPRT | ECHOKE | IEXTEN | EXTPROC);

    print_flags("c_iflag", options.c_iflag, k_termios_iflags);
    print_flags("c_oflag", options.c_oflag, k_termios_oflags);
    print_flags("c_lflag", options.c_lflag, k_termios_lflags);
    print_flags("c_cflag", options.c_cflag, k_termios_cflags);

//     cfmakeraw(&options);
//     options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF);
//     options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    options.c_cflag &= ~(CSTOPB | CRTSCTS | PARENB | PARODD);
//     { CSTOPB, STR(CSTOPB) },
//     { CREAD, STR(CREAD) },
//     { PARENB, STR(PARENB) },
//     { PARODD, STR(PARODD) },
//     { HUPCL, STR(HUPCL) },
//     { CLOCAL, STR(CLOCAL) },

    // no timeout
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 1;

    // set speed
    cfsetspeed(&options, B115200);

    tcsetattr(fd, TCSANOW, &options);

    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);

    s->fd = fd;
#endif

    return s;

// error_close:
//    #ifdef _WIN32
//    #else
//     close(fd);
//    #endif

error_free:
    free(s);
    return NULL;
}

void usc_serial_close(usc_serial_t* const s)
{
    assert(s != NULL);

   #ifdef _WIN32
    CloseHandle(s->h);
   #else
    close(s->fd);
   #endif

    free(s);
}

static bool _usc_read(usc_serial_t* const s, uint8_t* const buffer, const uint32_t size)
{
   #ifdef _WIN32
    for (uint32_t r = 0; r < size;)
    {
        unsigned long r2 = 0;
        if (ReadFile(s->h, buffer + r, size - r, &r2, NULL) == FALSE)
            return false;

        r += r2;
        assert(r <= size);
    }
   #else
    for (uint32_t r = 0; r < size;)
    {
        const int r2 = read(s->fd, buffer + r, size - r);
        // printf("_usc_read got %d | %x bytes, total %d | %x bytes, size %u\n", r2, r2, r + r2, r + r2, size);

        if (r2 < 0)
        {
            if (errno == EAGAIN)
            {
                usleep(1000);
                continue;
            }

            perror("_usc_read read < 0");
            return false;
        }

        r += r2;
        assert(r <= size);
    }
   #endif

    return true;
}

static bool _usc_write(usc_serial_t* const s, const uint8_t* const buffer, const uint32_t size)
{
   #ifdef _WIN32
    for (unsigned long w = 0; w < size;)
    {
        unsigned long w2 = 0;
        if (WriteFile(s->h, buffer + w, size - w, &w2, NULL) == FALSE)
            return false;

        w += w2;
        assert(w <= size);
    }
   #else
    for (uint32_t w = 0; w < size;)
    {
        const int w2 = write(s->fd, buffer + w, size - w);
        DEBUG_PRINT("usc_serial_write_cmd got %d | %x bytes, total %d | %x bytes, size %u\n", w2, w2, w + w2, w + w2, size);

        if (w2 < 0)
        {
            if (errno == EAGAIN)
            {
                usleep(1000);
                continue;
            }

            perror("usc_serial_write_cmd write < 0");
            return false;
        }

        w += w2;
        assert(w <= size);
    }
   #endif

    return true;
}

bool usc_serial_read_cmd(usc_serial_t* const s, char cmd[CMD_SIZE])
{
    memset(cmd, 0, CMD_SIZE);

    return _usc_read(s, (uint8_t*)cmd, CMD_SIZE);
}

bool usc_serial_read_payload(usc_serial_t* s, void* payload, size_t size)
{
    return _usc_read(s, payload, size);
}

bool usc_serial_write_cmd(usc_serial_t* const s, const char* const cmd)
{
    char cmdbuf[CMD_SIZE] = { 0 };
    strncpy(cmdbuf, cmd, CMD_SIZE - 1);
    DEBUG_PRINT("usc_serial_write_cmd '%s'\n", cmdbuf);

    return _usc_write(s, (const uint8_t*)cmdbuf, CMD_SIZE);
}

bool usc_serial_write_payload(usc_serial_t* const s, const void* const payload, const size_t size)
{
    return _usc_write(s, payload, size);
}
