#include "result.h"
#include "query.h"
#include "record.h"

#include <stdlib.h>
#include <unistd.h>

Result::Result() : _ok(false) {}

Result::Result(bool ok, size_t count, bool printCount, const std::vector<Record> &records)
    : _ok(ok), _count(count), _printCount(printCount), _records(records) {}

bool Result::ok() const { return _ok; }

size_t Result::count() const { return _count; }

const std::vector<Record> &Result::records() const { return _records; }

void Result::writeTxt(FILE *fout) const {
    fprintf(fout, "STATUS %s\n", _ok ? "ok" : "error");

    if (!_ok) {
        return;
    }

    if (!_records.empty()) {
        printFooter(fout);

        for (size_t i = 0; i < _records.size(); i++) {
            _records[i].writeTxt(fout);
        }
    }

    if (_printCount) {
        fprintf(fout, "Total %5zu\n", _count);
    }
}

IOStatus Result::writeBin(int fd) const {
    if (myWrite(fd, &_ok, sizeof(_ok)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myWrite(fd, &_count, sizeof(_count)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myWrite(fd, &_printCount, sizeof(_printCount)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    size_t n = _records.size();

    if (myWrite(fd, &n, sizeof(n)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    for (size_t i = 0; i < _records.size(); i++) {
        if (_records[i].writeBin(fd) != IOStatus::OK) {
            return IOStatus::ERR;
        }
    }

    return IOStatus::OK;
}

IOStatus Result::readBin(int fd) {
    if (myRead(fd, &_ok, sizeof(_ok)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myRead(fd, &_count, sizeof(_count)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    if (myRead(fd, &_printCount, sizeof(_printCount)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    size_t n;

    if (myRead(fd, &n, sizeof(n)) != IOStatus::OK) {
        return IOStatus::ERR;
    }

    _records.resize(n);

    for (size_t i = 0; i < _records.size(); i++) {
        if (_records[i].readBin(fd) != IOStatus::OK) {
            return IOStatus::ERR;
        }
    }

    return IOStatus::OK;
}
