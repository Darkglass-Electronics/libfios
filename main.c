#undef NDEBUG
#define DEBUG
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define CMD_SIZE 2 /* 'w ' */ + 6 /* 0xffff */ + 1 /* ' ' */ + 1 /* null */
#define MAX_PAYLOAD_SIZE 0x2000

#define DEBUG_PRINT(...) printf(__VA_ARGS__)

static int usage(char* argv[])
{
    fprintf(stderr, "Usage: %s [r|w] [device-path|auto] [file-path]\n", argv[0]);
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc <= 3)
        return usage(argv);

    bool writing;
    if (!strcmp(argv[1], "r"))
        writing = false;
    else if (!strcmp(argv[1], "w"))
        writing = true;
    else
        return usage(argv);

#ifdef _WIN32
    BOOL ret;

    const HANDLE hCom = CreateFile(argv[2], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(hCom != NULL && hCom != INVALID_HANDLE_VALUE);
    
    DCB params = { 0 };
    params.DCBlength = sizeof(params);

    ret = GetCommState(hCom, &params);
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

    ret = SetCommState(hCom, &params);
    assert(ret != FALSE);

    COMMTIMEOUTS timeouts = { 0 };

    ret = SetCommTimeouts(hCom, &timeouts);
    assert(ret != FALSE);

    ret = PurgeComm(hCom, PURGE_RXCLEAR);
    assert(ret != FALSE);

    ret = PurgeComm(hCom, PURGE_TXCLEAR);
    assert(ret != FALSE);
#else
    const char* devpath;
    if (!strcmp(argv[2], "auto"))
    {
       #ifdef __aarch64__
        devpath = "/dev/ttyGS0";
       #else
        devpath = "/dev/ttyACM0";
       #endif
    }
    else
    {
        devpath = argv[2];
    }

    const int fd = open(devpath, O_RDWR | O_NOCTTY);
    assert(fd > 0);

//     fcntl(fd, O_NONBLOCK);

    struct termios options = { 0 };
    tcgetattr(fd, &options);

    // cfmakeraw(&options);
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF);
    options.c_oflag &= ~(OPOST | ONLCR | OCRNL);
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    options.c_cflag &= ~(CSIZE | CSTOPB | CRTSCTS | PARENB);
    options.c_cflag |= CS8;

    // no timeout
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 1;

    tcsetattr(fd, TCSANOW, &options); // TCSAFLUSH

//     if (writing)
//     {
//         tcflush(fd, TCIFLUSH);
//         tcflush(fd, TCOFLUSH);
//     }
#endif

    char buf[MAX_PAYLOAD_SIZE];
    char cmd[CMD_SIZE];

    if (writing)
    {
        DEBUG_PRINT("receiving data and writing into '%s'\n", argv[3]);

        FILE* out = fopen(argv[3], "w");
        assert(out);

        fseek(out, 0, SEEK_SET);

        for (int r, w;;)
        {
            DEBUG_PRINT("waiting for command\n");

            // read command
            memset(cmd, 0, CMD_SIZE);
#ifdef _WIN32
            unsigned long r2;
            if (ReadFile(hCom, cmd, CMD_SIZE, &r2, NULL) == FALSE)
                r = -2;
            else
                r = r2;
#else
            r = read(fd, cmd, CMD_SIZE);

            if (r == -1)
            {
                sleep(1);
                continue;
            }
#endif
            DEBUG_PRINT("command size %d | %02x:'%c' %02x:'%c'\n", r, cmd[0], cmd[0], cmd[1], cmd[1]);

            if (r != CMD_SIZE)
            {
                perror("error invalid command size");
                break;
            }

            if (cmd[0] == 'q' && cmd[1] == 0)
                break;

            if (cmd[0] != 'w' || cmd[1] != ' ')
            {
                perror("error invalid command type");
                break;
            }

            cmd[CMD_SIZE - 1] = 0;

            // size comes as 2nd arg
            long int remaining = strtol(cmd + 2, NULL, 16);
            DEBUG_PRINT("waiting for payload of size %ld | 0x%04lx\n", remaining, remaining);

            while (remaining != 0)
            {
#ifdef _WIN32
                if (ReadFile(hCom, buf, remaining, &r2, NULL) == FALSE)
                    r = -2;
                else
                    r = r2;
#else
                r = read(fd, buf, remaining);
                assert(r > 0);
#endif
                DEBUG_PRINT("read %04d | 0x%04x bytes, remaining %04ld | 0x%04lx bytes\n", r, r, remaining - r, remaining - r);

                if (r <= 0)
                {
                    printf("read %d bytes\n", r);
                    break;
                }

                for (int total = 0; total < r; total += w)
                {
                    w = fwrite(buf + total, 1, r - total, out);

                    if (w < 0)
                    {
                        perror("error partial write");
                        break;
                    }
                }

                remaining -= r;
                assert(remaining >= 0);
            }

            DEBUG_PRINT("payload received, sending ok back\n");

            // write back ok
#ifdef _WIN32
            WriteFile(hCom, "ok", 3, NULL, NULL);
#else
            w = write(fd, "ok", 3);
            assert(w == 3);
//             tcdrain(fd);
#endif
        }

        fclose(out);
    }
    else
    {
        DEBUG_PRINT("writing data by reading '%s'\n", argv[3]);

        FILE* in = fopen(argv[3], "r");
        assert(in);

        fseek(in, 0, SEEK_SET);

        for (int r, w;;)
        {
            r = fread(buf, 1, MAX_PAYLOAD_SIZE, in);

            DEBUG_PRINT("main file read return %d | 0x%x\n", r, r);

            if (r <= 0)
                break;

            DEBUG_PRINT("writing command for %d | 0x%x bytes\n", r, r);

            // encode write command as first byte, followed by expected size, and then the payload
            snprintf(cmd, CMD_SIZE, "w 0x%04x ", r);
#ifdef _WIN32
            WriteFile(hCom, cmd, CMD_SIZE, NULL, NULL);
#else
            w = write(fd, cmd, CMD_SIZE);
            assert(w == CMD_SIZE);
//             tcdrain(fd);
#endif

            for (int total = 0, w; total < r; total += w)
            {
                DEBUG_PRINT("writing payload for %d | 0x%x bytes\n", r - total, r - total);
#ifdef _WIN32
                unsigned long w2;
                if (WriteFile(hCom, buf + total, r - total, &w2, NULL) == FALSE)
                    w = -2;
                else
                    w = w2;
#else
                w = write(fd, buf + total, r - total);
                assert(w == r - total);
//                 tcdrain(fd);
                DEBUG_PRINT("wrote payload as %d | 0x%x bytes\n", w, w);
#endif

                if (w < 0)
                {
                    perror("error partial write");
                    break;
                }
            }

#ifdef _WIN32
#else
//             syncfs(fd);
//             tcdrain(fd);
//             tcflush(fd, TCOFLUSH);
#endif

            // wait for ok message
            // for (;;)
            {
                DEBUG_PRINT("waiting for ok signal for %d | 0x%x bytes\n", r, r);
#ifdef _WIN32
                unsigned long r2;
                if (ReadFile(hCom, cmd, 3, &r2, NULL) == FALSE)
                    r2 = -2;
                else
                    r = r2;
#else
                r = read(fd, cmd, 3);
#endif

                DEBUG_PRINT("reply read return %d | 0x%x\n", r, r);

                if (r != 3 || cmd[0] != 'o' || cmd[1] != 'k' || cmd[2] != 0)
                {
                    perror("error waiting for ok after write");
                    break;
                }
            }
        }

        DEBUG_PRINT("writing command for close\n");
        memset(cmd, 0, CMD_SIZE);
        snprintf(cmd, CMD_SIZE, "q");
#ifdef _WIN32
        WriteFile(hCom, cmd, CMD_SIZE, NULL, NULL);
#else
        int w = write(fd, cmd, CMD_SIZE);
        assert(w == CMD_SIZE);
//         tcdrain(fd);
#endif

        fclose(in);
    }

#ifdef _WIN32
    CloseHandle(hCom);
#else
    close(fd);
#endif
    return 0;
}
