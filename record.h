#ifndef _RECORD_H_
#define _RECORD_H_

#include <stdio.h>
#include <vector>
#include "common.h"

class Record {
   public:
    Record();
    Record(const char *journal, int year, int number, const char *author, const char *article);

    bool operator==(const Record &other);

    void setRand();

    const char *journal() const;
    int year() const;
    int number() const;
    const char *author() const;
    const char *article() const;
    std::vector<std::string> values() const;

    void writeTxt(FILE *fout) const;

    IOStatus writeBin(int fd) const;
    IOStatus readBin(int fd);

   private:
    char _journal[64];
    int _year;
    int _number;
    char _author[32];
    char _article[128];
};

bool operator==(const Record &lhs, const Record &rhs);
bool operator!=(const Record &lhs, const Record &rhs);

void printFooter(FILE *fout);

bool checkJournal(const char *journal);
bool checkYear(int year);
bool checkNumber(int number);
bool checkAuthor(const char *author);
bool checkArticle(const char *article);

#endif  
