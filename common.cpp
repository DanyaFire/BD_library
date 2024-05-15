#include "common.h"

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/signal.h>

bool toInt(const char *str, int &result) {
    char *str_end;
    errno = 0;

    long tmp = strtol(str, &str_end, 10);

    if (errno == 0 && *str_end == '\0' && INT_MIN <= tmp && tmp <= INT_MAX) {
        result = tmp;
        return true;
    }

    return false;
}

bool toUint16(const char *str, uint16_t &result) {
    int tmp;
    bool ok = toInt(str, tmp) && (tmp >= 0) && (tmp < 65536);  // 2^16 = 65536
    result = tmp;
    return ok;
}

bool toIntRange(const char *str, int n, int &start, int &end) {
    bool ok = false;
    int a, b;

    if (sscanf(str, "%d:%d", &a, &b) == 2) {
        ok = (a <= b) && (b <= n);
        start = a;
        end = b;
    } else if (sscanf(str, "-%d", &a) == 1) {
        ok = (a <= n);
        start = n - a;
        end = n;
    } else if (sscanf(str, "%d", &a) == 1) {
        ok = (a <= n);
        start = 0;
        end = a;
    }

    return ok;
}

#define LEN 128

bool readLine(std::vector<char> &line) {
    char buf[LEN];

    line.clear();

    printf("> ");

    for (;;) {
        if (fgets(buf, LEN, stdin) == nullptr) {
            return false;
        }

        for (int i = 0; buf[i] != '\0'; i++) {
            if (buf[i] == '\n' || buf[i] == '\r') {
                line.push_back('\0');
                return true;
            }
            line.push_back(buf[i]);
        }
    }
}

static void handler(int signo) { (void)signo; }

bool signalIgnoring() {
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGINT, &act, 0) == -1) {
        fprintf(stderr, "Can't setup SIGINT ignoring\n");
        return false;
    }

    if (sigaction(SIGPIPE, &act, 0) == -1) {
        fprintf(stderr, "Can't setup SIGPIPE ignoring\n");
        return false;
    }

    return true;
}

IOStatus myRead(int fd, void *_buf, size_t nbyte) {//позволяет читать данные из файлового дескриптора в буфер по частям, пока не будет прочитано заданное количество байт или не произойдет ошибка
    char *buf = (char *)_buf;
    size_t cur = 0;

    while (cur < nbyte) {
        ssize_t s = read(fd, buf + cur, nbyte - cur);

        if (s > 0) {
            cur += s;
        } else {
            break;
        }
    }

    return (cur == 0) ? IOStatus::EOD : (cur == nbyte) ? IOStatus::OK : IOStatus::ERR;
}

IOStatus myWrite(int fd, const void *_buf, size_t nbyte) {
    const char *buf = (const char *)_buf;
    size_t cur = 0;

    while (cur < nbyte) {
        ssize_t s = write(fd, buf + cur, nbyte - cur);

        if (s > 0) {
            cur += s;
        } else {
            break;
        }
    }

    return (cur == 0) ? IOStatus::EOD : (cur == nbyte) ? IOStatus::OK : IOStatus::ERR;
}
