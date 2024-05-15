#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <string>
#include <vector>

class Query;
class Record;
class Result;

class DataBase {
   public:
    DataBase(const char *filename);

    const std::vector<Record> &records() const;

    bool load();
    bool save() const;

    Result process(const Query &query);

    void insert(const Record &record);
    void clear();

   private:
    std::string _filename;
    std::vector<Record> _data;

    size_t _processSelect(const Query &query, std::vector<Record> &records) const;
    size_t _processDelete(const Query &query);
};

#endif  
