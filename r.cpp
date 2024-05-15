#include "common.h"
#include "record.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Use: %s FILE_NAME RANGE\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        fprintf(stderr, "Can't open file\n");
        return -1;
    }

    std::vector<Record> data;

    for (;;) {
        Record rec;

        IOStatus s = rec.readBin(fd);

        if (s != IOStatus::OK) {
            break;
        }

        data.push_back(rec);
    }

    close(fd);

    int s, e;

    if (!toIntRange(argv[2], data.size(), s, e)) {
        fprintf(stderr, "Invalid RANGE value\n");
        return -1;
    }

    printFooter(stdout);
	printf("\n");

    for (int i = s; i < e; i++) {
        data[i].writeTxt(stdout);
    }

    printf("Total: %5d\n", e - s);
	printf("\n");
}
