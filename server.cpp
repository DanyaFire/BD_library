#include "common.h"
#include "config.h"
#include "database.h"
#include "query.h"
#include "record.h"
#include "result.h"
#include "server.h"


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

static void loop(int ld, DataBase &db) {
    struct pollfd fds[QUERY_SIZE];
    int nfds = 1;
    struct Server* servers[2];
    servers[0]  = new Server_HTTP();
    servers[1] = new ServerHTTP();

    fds[0].fd = ld;
    fds[0].events = POLLIN;

    for (;;) {
        int rc = poll(fds, nfds, -1);

        if (rc == -1) {
            if (errno == EINTR) {
                break;
            }

            fprintf(stderr, "Can't poll\n");
            return;
        }

        if (fds[0].revents & POLLIN) {
            int fd = connectClient(ld);

            if (fd == -1) {
                continue;
            }

            if (nfds < QUERY_SIZE) {
                fds[nfds].fd = fd;
                fds[nfds].events = POLLIN;
                nfds++;
            } else {
                disconnectClient(fd);
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                if (!(*servers[0]).process(fds[i].fd, db)) {
                    disconnectClient(fds[i].fd);
                    fds[i].fd = -1;
                    fds[i].events = 0;
                }
            }
        }

        int j = 1;
        
        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd == -1) {
                continue;
            }

            if (i != j) {
                fds[j].fd = fds[i].fd;
                fds[j].events = fds[i].events;
            }

            j++;
        }

        nfds = j;
    }
}
