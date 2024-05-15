#include "record.h"
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>


Record::Record() {
    _journal[0] = _author[0] = _article[0] = '\0';
    _year = _number = 0;
}

Record::Record(const char *journal, int year, int number, const char *author, const char *article) {
    strcpy(_journal, journal);
    _year = year;
    _number = number;
    strcpy(_author, author);
    strcpy(_article, article);
}

static void setRandStr(char *s, int min_len, int max_len) {
    int len = min_len + rand() % (max_len - min_len + 1);
    s[0] = 'A' + rand() % ('Z' - 'A' + 1);
    for (int i = 1; i < len; i++) {
        s[i] = 'a' + rand() % ('z' - 'a' + 1);
    }
    s[len] = '\0';
}

static int randInt(int min, int max) { return min + rand() % (max - min + 1); }

void Record::setRand() {
    setRandStr(_journal, JOURNAL_MIN_LEN, JOURNAL_MAX_LEN);
    _year = randInt(YEAR_MIN, YEAR_MAX);
    _number = randInt(NUMBER_MIN, NUMBER_MAX);
    setRandStr(_author, AUTHOR_MIN_LEN, AUTHOR_MAX_LEN);
    setRandStr(_article, ARTICLE_MIN_LEN, ARTICLE_MAX_LEN);
}

const char *Record::journal() const { return _journal; }

int Record::year() const { return _year; }

int Record::number() const { return _number; }

const char *Record::author() const { return _author; }

const char *Record::article() const { return _article; }

void printFooter(FILE *fout) {
    fprintf(fout, "| %-20s | %-4s | %-6s | %-20s | %-40s |\n", "JOURNAL", "YEAR", "NUMBER", "AUTHOR", "ARTICLE");
}

void Record::writeTxt(FILE *fout) const {
    fprintf(fout, "| %-20s | %4d |    %03d | %-20s | %-40s |\n", _journal, _year, _number, _author, _article);
}

IOStatus Record::writeBin(int fd) const {
    if (myWrite(fd, _journal, sizeof(_journal)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myWrite(fd, &_year, sizeof(_year)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myWrite(fd, &_number, sizeof(_number)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myWrite(fd, _author, sizeof(_author)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myWrite(fd, _article, sizeof(_article)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    return IOStatus::OK;
}

IOStatus Record::readBin(int fd) {
    IOStatus s = myRead(fd, _journal, sizeof(_journal));
    if (s != IOStatus::OK) {
        return s;
    }

    if (myRead(fd, &_year, sizeof(_year)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myRead(fd, &_number, sizeof(_number)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myRead(fd, _author, sizeof(_author)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myRead(fd, _article, sizeof(_article)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    return IOStatus::OK;
}

static bool eq(const char *s1, const char *s2) { return (strcmp(s1, s2) == 0); }

bool operator==(const Record &lhs, const Record &rhs) {
    return eq(lhs.journal(), rhs.journal()) && eq(lhs.author(), rhs.author()) && eq(lhs.article(), rhs.article()) &&
           (lhs.year() == rhs.year()) && (lhs.number() == rhs.number());
}

bool operator!=(const Record &lhs, const Record &rhs) { return !(lhs == rhs); }

static bool checkInt(int n, int min, int max) { return (min <= n) && (n <= max); }

static bool checkStr(const char *str, int min_len, int max_len) {
    int n = strlen(str);
    return (min_len <= n) && (n <= max_len);
}

bool checkJournal(const char *journal) { return checkStr(journal, JOURNAL_MIN_LEN, JOURNAL_MAX_LEN); }

bool checkYear(int year) { return checkInt(year, YEAR_MIN, YEAR_MAX); }

bool checkNumber(int number) { return checkInt(number, NUMBER_MIN, NUMBER_MAX); }

bool checkAuthor(const char *author) { return checkStr(author, AUTHOR_MIN_LEN, AUTHOR_MAX_LEN); }

bool checkArticle(const char *article) { return checkStr(article, ARTICLE_MIN_LEN, ARTICLE_MAX_LEN); }
