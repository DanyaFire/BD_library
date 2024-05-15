#include "common.h"
#include "database.h"
#include "query.h"
#include "result.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <vector>


void repl(DataBase &db);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Use: %s FILE_NAME\n", argv[0]);
        return -1;
    }

    DataBase db(argv[1]);

    if (!db.load()) {
        printf("Can not load database\n");
        return -1;
    }

    repl(db);
}

void repl(DataBase &db) {
    std::vector<char> line;
    Query q;

    printQueryPatterns();

    for (;;) {
        if (!readLine(line)) {
            break;
        }

        q.parse(&line[0]);

        db.process(q).writeTxt(stdout);
    }

    printf("\n");
}
