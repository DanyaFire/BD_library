#include "common.h"
#include "config.h"
#include "database.h"
#include "query.h"
#include "record.h"
#include "result.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

static int connect(const char *host, uint16_t port);
static int disconnect(int fd);
static void loop(int ld, DataBase &db);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Use: %s FILENAME\n", argv[0]);
        return -1;
    }

    DataBase db(argv[1]);

    if (!db.load()) {
        fprintf(stderr, "Can't load database from file\n");
        return -1;
    }

    if (!signalIgnoring()) {
        return -1;
    }

    int ld = connect(HOST, PORT);

    if (ld == -1) {
        return -1;
    }

    loop(ld, db);

    if (disconnect(ld) != 0) {
        return -1;
    }
}

static int connect(const char *host, uint16_t port) {
    sockaddr_in addr;
    int fd, on;

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

    on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        fprintf(stderr, "Can't setsockopt\n");
        if (close(fd) == -1) {
            fprintf(stderr, "Can't release file descriptor\n");
        }
        return -1;
    }

    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "Can't bind\n");
        if (close(fd) == -1) {
            fprintf(stderr, "Can't release file descriptor\n");
        }
        return -1;
    }

    if (listen(fd, QUERY_SIZE) == -1) {
        fprintf(stderr, "Can't listen\n");
        if (close(fd) == -1) {
            fprintf(stderr, "Can't release file descriptor\n");
        }
        return -1;
    }

    return fd;
}

static int disconnect(int fd) {
    if (close(fd) == -1) {
        fprintf(stderr, "Can't release file descriptor\n");
        return -1;
    }

    return 0;
}

static int connectClient(int ld) {
    sockaddr_in addr;
    socklen_t addrlen;

    for (;;) {
        memset(&addr, 0, sizeof(addr));
        addrlen = sizeof(addr);

        int fd = accept(ld, (sockaddr *)&addr, &addrlen);

        if (fd == -1) {
            if (errno == EINTR) {
                return -1;
            }

            if (errno != EAGAIN) {
                fprintf(stderr, "Can't accept");
            }

            continue;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
        printf("New client: fd=%d ip=%s port=%d\n", fd, ip, addr.sin_port);

        return fd;
    }
}

static void disconnectClient(int fd) {
    if (fd == -1) {
        return;
    }

    printf("Disconnecting client: fd=%d\n", fd);

    if (shutdown(fd, SHUT_RDWR) == -1 && errno != ENOTCONN) {
        fprintf(stderr, "Can't shutdown socket\n");
    }

    if (close(fd) == -1) {
        fprintf(stderr, "Can't release file descriptor\n");
    }
}

static bool process(int fd, DataBase &db) {
    // 1. Read string from client
    int n;

    if (myRead(fd, &n, sizeof(n)) != IOStatus::OK) {
        return false;
    }

    std::vector<char> cmd(n);

    if (myRead(fd, &cmd[0], n) != IOStatus::OK) {
        return false;
    }
	
    printf("Body: %s\n", &cmd[0]);
	
    // 2. Process input data
    Query query;
    query.parse(&cmd[0]);
    Result record = db.process(query);

    // 3. Send result
    if (record.writeBin(fd) != IOStatus::OK) {
        return false;
    }

    return true;
}

static void loop(int ld, DataBase &db) {
    std::vector<pollfd> fds;
    fds.reserve(POLL_SIZE);

    pollfd pfd = {ld, POLLIN, 0};
    fds.push_back(pfd);

    for (;;) {
        int nready = poll(&fds[0], fds.size(), -1);
        if (nready == -1) {
            if (errno == EINTR) {
                break;
            }
            fprintf(stderr, "Can't poll\n");
            return;
        }

        if (fds[0].revents & POLLIN) {
            int fd = connectClient(ld);
            if (fd != -1) {
                pollfd pfd = {fd, POLLIN, 0};
                fds.push_back(pfd);
            }
        }

        for (size_t i = 1; i < fds.size(); i++) {
            if (fds[i].revents & POLLIN) {
                if (!process(fds[i].fd, db)) {
                    disconnectClient(fds[i].fd);
                    fds[i] = fds.back();
                    fds.pop_back();
                    i--;
                }
            }
        }
    }
}

