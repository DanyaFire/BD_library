#include "query.h"
#include "config.h"

#include <ctype.h>

void printQueryPatterns() {
    printf(
        "Query patterns:\n"
        "    select author=[STR] year=[min, max] number=[min, max]\n"
        "    delete author=[STR] year=[min, max] number=[min, max]\n"
        "    insert journal=[STR] year=[INT] number=[INT] author=[STR] article=[STR]\n"
        "    save\n"
        "    clear\n"
        "\n");
}

Query::Query() { _type = QueryType::INVALID; }

static int skip(const char *txt) {
    int i = 0;
    for (i = 0; isspace(txt[i]); i++);
    return i;
}

static bool isEol(const char *txt) { return txt[skip(txt)] == '\0'; }

bool Query::parse(const char *txt) {
    _filters.clear();

    txt = _parseType(txt);

    bool ok;

    switch (_type) {
        case QueryType::SELECT:
        case QueryType::DELETE:
            ok = _parseFilters(txt);
            break;
        case QueryType::INSERT:
            ok = _parseFields(txt);
            break;
        case QueryType::SAVE:
        case QueryType::CLEAR:
            ok = isEol(txt);
            break;
        case QueryType::INVALID:
            ok = false;
            break;
    }

    if (!ok) {
        _filters.clear();
        _type = QueryType::INVALID;
    }

    return ok;
}

void Query::writeTxt(FILE *fout) const {
    switch (_type) {
        case QueryType::SELECT:
            fprintf(fout, "TYPE select\n");
            break;
        case QueryType::DELETE:
            fprintf(fout, "TYPE delete\n");
            break;
        case QueryType::INSERT:
            fprintf(fout, "TYPE insert\n");
            break;
        case QueryType::SAVE:
            fprintf(fout, "TYPE save\n");
            break;
        case QueryType::CLEAR:
            fprintf(fout, "TYPE clear\n");
            break;
        case QueryType::INVALID:
            fprintf(fout, "TYPE invalid\n");
            break;
    }

    if (_type == QueryType::INVALID) {
        return;
    }

    if (_type == QueryType::SELECT || _type == QueryType::DELETE) {
        for (size_t i = 0; i < _filters.size(); i++) {
            const Filter &f = _filters[i];

            switch (f.type) {
                case FieldType::AUTHOR:
                    fprintf(fout, "AUTHOR \"%s\"\n", f.author);
                    break;
                case FieldType::YEAR:
                    fprintf(fout, "YEAR %d-%d\n", f.yearMin, f.yearMax);
                    break;
                case FieldType::NUMBER:
                    fprintf(fout, "NUMBER %d-%d\n", f.numberMin, f.numberMax);
                    break;
                case FieldType::JOURNAL:
                case FieldType::ARTICLE:
                    break;
            }
        }
    }

    if (_type == QueryType::INSERT) {
        fprintf(fout, "JOURNAL \"%s\"\n", _record.journal());
        fprintf(fout, "YEAR %d\n", _record.year());
        fprintf(fout, "NUMBER %d\n", _record.number());
        fprintf(fout, "AUTHOR \"%s\"\n", _record.author());
        fprintf(fout, "ARTICLE \"%s\"\n", _record.article());
    }
}

QueryType Query::type() const { return _type; }

const Record &Query::record() const { return _record; }

bool Query::check(const Record &r) const {
    bool result = true;

    for (size_t i = 0; i < _filters.size(); i++) {
        const Filter &f = _filters[i];

        switch (f.type) {
            case FieldType::AUTHOR:
                result &= (strcmp(f.author, r.author()) == 0);
                break;
            case FieldType::YEAR:
                result &= (f.yearMin <= r.year()) && (r.year() <= f.yearMax);
                break;
            case FieldType::NUMBER:
                result &= (f.numberMin <= r.number()) && (r.number() <= f.numberMax);
                break;
            case FieldType::JOURNAL:
            case FieldType::ARTICLE:
                break;
        }

        if (!result) {
            break;
        }
    }

    return result;
}

static bool noPostfix(const char *txt) { return (skip(txt) > 0) || (*txt == '\0'); }

static bool checkType(const char *txt, const char *fmt, int &pos) {
    int localPos = 0;
    sscanf(txt, fmt, &localPos);
    pos += localPos;
    return (localPos != 0) && noPostfix(txt + localPos);
}

const char *Query::_parseType(const char *txt) {
    txt += skip(txt);

    int pos = 0;

    bool isSelect = checkType(txt, "select%n", pos);
    bool isDelete = checkType(txt, "delete%n", pos);
    bool isInsert = checkType(txt, "insert%n", pos);
    bool isSave = checkType(txt, "save%n", pos);
    bool isClear = checkType(txt, "clear%n", pos);

    txt += pos;

    if (isSelect) {
        _type = QueryType::SELECT;
    } else if (isInsert) {
        _type = QueryType::INSERT;
    } else if (isDelete) {
        _type = QueryType::DELETE;
    } else if (isSave) {
        _type = QueryType::SAVE;
    } else if (isClear) {
        _type = QueryType::CLEAR;
    } else {
        _type = QueryType::INVALID;
    }

    return txt;
}

static bool parseIntRange(const char *txt, const char *fmt, int &a, int &b, int &pos) {
    int localPos = 0;
    sscanf(txt, fmt, &a, &b, &localPos);
    pos += localPos;
    return (localPos != 0) && noPostfix(txt + localPos);
}

static bool parseInt(const char *txt, const char *fmt, int &a, int &pos) {
    int localPos = 0;
    sscanf(txt, fmt, &a, &localPos);
    pos += localPos;
    return (localPos != 0) && noPostfix(txt + localPos);
}

static bool parseStr(const char *txt, const char *fmt, char *s, int &pos) {
    int localPos = 0;
    sscanf(txt, fmt, s, &localPos);
    pos += localPos;
    return (localPos != 0) && noPostfix(txt + localPos);
}

bool Query::_parseFilters(const char *txt) {
    while (true) {
        txt += skip(txt);

        if (*txt == '\0') {
            break;
        }

        Filter f;
        int pos = 0;

        bool isAuthor = parseStr(txt, "author=[%31[A-Za-z ]]%n", f.author, pos);
        bool isYear = parseIntRange(txt, "year=[%d,%d]%n", f.yearMin, f.yearMax, pos);
        bool isNumber = parseIntRange(txt, "number=[%d,%d]%n", f.numberMin, f.numberMax, pos);

        txt += pos;

        bool ok;

        if (isAuthor) {
            f.type = FieldType::AUTHOR;
            ok = checkAuthor(f.author);
        } else if (isYear) {
            f.type = FieldType::YEAR;
            ok = (f.yearMin <= f.yearMax) && checkYear(f.yearMin) && checkYear(f.yearMax);
        } else if (isNumber) {
            f.type = FieldType::NUMBER;
            ok = (f.numberMin <= f.numberMax) && checkNumber(f.numberMin) && checkNumber(f.numberMax);
        } else {
            ok = false;
        }

        if (!ok) {
            return false;
        }

        _filters.push_back(f);
    }

    return true;
}

bool Query::_parseFields(const char *txt) {
    char journal[64];
    int year;
    int number;
    char author[32];
    char article[128];

    bool isJournalSet = false;
    bool isYearSet = false;
    bool isNumberSet = false;
    bool isAuthorSet = false;
    bool isArticleSet = false;

    while (true) {
        txt += skip(txt);

        if (*txt == '\0') {
            break;
        }

        int pos = 0;

        bool isJournal = parseStr(txt, "journal=[%63[A-Za-z ]]%n", journal, pos);
        bool isYear = parseInt(txt, "year=[%d]%n", year, pos);
        bool isNumber = parseInt(txt, "number=[%d]%n", number, pos);
        bool isAuthor = parseStr(txt, "author=[%31[A-Za-z ]]%n", author, pos);
        bool isArticle = parseStr(txt, "article=[%127[A-Za-z ]]%n", article, pos);

        txt += pos;

        bool ok;

        if (isJournal) {
            ok = checkJournal(journal);
            isJournalSet = true;
        } else if (isYear) {
            ok = checkYear(year);
            isYearSet = true;
        } else if (isNumber) {
            ok = checkNumber(number);
            isNumberSet = true;
        } else if (isAuthor) {
            ok = checkAuthor(author);
            isAuthorSet = true;
        } else if (isArticle) {
            ok = checkArticle(article);
            isArticleSet = true;
        } else {
            ok = false;
        }

        if (!ok) {
            return false;
        }
    }

    bool allFieldsSet = isJournalSet && isYearSet && isNumberSet && isAuthorSet && isArticleSet;

    if (allFieldsSet) {
        _record = Record(journal, year, number, author, article);
    }

    return allFieldsSet;
}
