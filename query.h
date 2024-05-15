#ifndef _QUERY_H_
#define _QUERY_H_

#include <ctype.h>
#include <string.h>
#include <vector>
#include "record.h"

enum class QueryType {
    SELECT,
    DELETE,
    INSERT,
    SAVE,
    CLEAR,

    INVALID,
};

class Query {
   public:
    Query();

    bool parse(const char *txt);

    void writeTxt(FILE *fout) const;

    QueryType type() const;
    const Record &record() const;
    bool check(const Record &record) const;

   private:
    enum class FieldType {
        JOURNAL,
        YEAR,
        NUMBER,
        AUTHOR,
        ARTICLE,
    };

    // For select/delete
    struct Filter {
        FieldType type;

        char author[32];
        int yearMin, yearMax;
        int numberMin, numberMax;
    };

   private:
    QueryType _type;
    std::vector<Filter> _filters;  // For select/delete
    Record _record;                // For insert

    const char *_parseType(const char *txt);
    bool _parseFilters(const char *txt);
    bool _parseFields(const char *txt);
};

void printQueryPatterns();

#endif  
