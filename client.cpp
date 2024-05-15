#include "common.h"
#include "config.h"
#include "query.h"
#include "record.h"
#include "result.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

static int connect(const char *host, uint16_t port);
static int disconnect(int fd);
static void repl(int fd);

int main() {
    if (!signalIgnoring()) {
        return -1;
    }

    int fd = connect(HOST, PORT);

    if (fd < 0) {
        return -1;
    }

    printQueryPatterns();
    repl(fd);

    if (disconnect(fd) != 0) {
        return -1;
    }
}

static int connect(const char *host, uint16_t port) {
    sockaddr_in addr;
    int fd;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) < 1) {
        fprintf(stderr, "Wrong host\n");
        return -1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        fprintf(stderr, "Can't create socket\n");
        return -1;
    }

    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "Can't connect to the server\n");
        if (close(fd) == -1) {
            fprintf(stderr, "Can't release file descriptor\n");
        }
        return -1;
    }

    return fd;
}

static int disconnect(int fd) {
    if (shutdown(fd, SHUT_RDWR) == -1 && errno != ENOTCONN) {
        fprintf(stderr, "Can't shutdown socket\n");
    }

    if (close(fd) == -1) {
        fprintf(stderr, "Can't release file descriptor\n");
        return -1;
    }

    return 0;
}

static bool process(int fd, const char *txt) {
    // 1. Send text
    int n = strlen(txt) + 1;

    if (myWrite(fd, &n, sizeof(n)) != IOStatus::OK) {
        return false;
    }

    if (myWrite(fd, txt, n) != IOStatus::OK) {
        return false;
    }

    // 2. Receive data
    Result r;

    if (r.readBin(fd) != IOStatus::OK) {
        return false;
    }

    // 3. Print data
    r.writeTxt(stdout);

    return true;
}

static void repl(int fd) {
    std::vector<char> line;

    for (;;) {
        if (!readLine(line)) {
            break;
        }

        if (!process(fd, &line[0])) {
            break;
        }
    }
}
