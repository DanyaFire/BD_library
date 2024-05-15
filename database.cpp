#include "database.h"
#include "query.h"
#include "record.h"
#include "result.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>


DataBase::DataBase(const char *filename) { _filename = filename; }

const std::vector<Record> &DataBase::records() const { return _data; }

bool DataBase::load() {
    int fd = open(_filename.c_str(), O_RDONLY);

    if (fd == -1) {
        return false;
    }

    std::vector<Record> data;

    for (;;) {
        Record rec;

        IOStatus s = rec.readBin(fd);

        if (s == IOStatus::EOD) {
            break;
        }

        if (s == IOStatus::ERR) {
            close(fd);
            return false;
        }

        data.push_back(rec);
    }

    close(fd);

    _data = std::move(data);
    return true;
}

bool DataBase::save() const {
    int fd = open(_filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        return false;
    }

    for (size_t i = 0; i < _data.size(); i++) {
        IOStatus s = _data[i].writeBin(fd);

        if (s != IOStatus::OK) {
            close(fd);
            return false;
        }
    }

    close(fd);
    return true;
}

void DataBase::insert(const Record &record) { _data.push_back(record); }

void DataBase::clear() { _data.clear(); }

Result DataBase::process(const Query &query) {
    bool ok = true;
    size_t count = 0;
    bool printCount = false;
    std::vector<Record> records;

    switch (query.type()) {
        case QueryType::SELECT:
            count = _processSelect(query, records);
            printCount = true;
            break;
        case QueryType::DELETE:
            count = _processDelete(query);
            printCount = true;
            break;
        case QueryType::INSERT:
            insert(query.record());
            break;
        case QueryType::SAVE:
            ok = save();
            break;
        case QueryType::CLEAR:
            clear();
            break;
        case QueryType::INVALID:
            ok = false;
            break;
    }

    return Result(ok, count, printCount, records);
}

size_t DataBase::_processSelect(const Query &query, std::vector<Record> &records) const {
    records.clear();

    for (size_t i = 0; i < _data.size(); i++) {
        if (query.check(_data[i])) {
            records.push_back(_data[i]);
        }
    }

    return records.size();
}

size_t DataBase::_processDelete(const Query &query) {
    size_t result = 0;

    for (size_t i = 0; i < _data.size();) {
        if (query.check(_data[i])) {
            _data.erase(_data.begin() + i);
            result++;
        } else {
            i++;
        }
    }

    return result;
}
