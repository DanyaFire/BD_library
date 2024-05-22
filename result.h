#ifndef _RESULT_H_
#define _RESULT_H_

#include <stdio.h>
#include <vector>
#include "common.h"

class Record;

class Result {
   public:
    Result();
    Result(bool ok, size_t count, bool printCount, const std::vector<Record> &records);

    bool ok() const;
    size_t count() const;
    const std::vector<Record> &records() const;
    std::vector<std::pair<std::string, std::string>> getFields() const;
    void writeTxt(FILE *fout) const;

    IOStatus writeBin(int fd) const;
    IOStatus readBin(int fd);

   private:
    bool _ok;
    size_t _count;
    bool _printCount;
    std::vector<Record> _records;
};

#endif  
