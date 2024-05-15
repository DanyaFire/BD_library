#include "common.h"
#include "record.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Use: %s FILE_NAME N RANGE\n", argv[0]);
        return -1;
    }

    int n, s, e;

    if (!toInt(argv[2], n)) {
        fprintf(stderr, "Invalid N value\n");
        return -1;
    }

    if (!toIntRange(argv[3], n, s, e)) {
        fprintf(stderr, "Invalid RANGE value\n");
        return -1;
    }

    int fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        fprintf(stderr, "Can't open file\n");
        return -1;
    }

    printFooter(stdout);
	printf("\n");

    for (int i = 0; i < n; i++) {
        Record r;
        r.setRand();

        r.writeBin(fd);

        if (s <= i && i < e) {
            r.writeTxt(stdout);
        }
    }

    close(fd);

    printf("Total: %5d\n", e - s);
	printf("\n");
	
}
