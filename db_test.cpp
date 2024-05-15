
#include "assert.h"
#include "database.h"
#include "query.h"
#include "record.h"
#include "result.h"

typedef void (*TestFunc)();

void test1() {
    // INSERT test
    DataBase db("");

    const char *cmd = "insert journal=[journal] year=[2020] number=[10] author=[author] article=[article]";
    Record r("journal", 2020, 10, "author", "article");

    assert(db.records().size() == 0);//внутр представление 

    Query q;
    assert(q.parse(cmd) == true);
    assert(q.type() == QueryType::INSERT);
    assert(q.record() == r);// cmd == r
	
    // Result res = db.process(q);
    // assert(res.ok() == true);
    assert(db.process(q).ok() == true);

    assert(db.records().size() == 1);
    assert(db.records()[0] == r);
}

void test2() {
    // SELECT test
    DataBase db("");

    Record r1("journal", 2001, 10, "author", "article");
    Record r2("journal", 2002, 10, "author", "article");
    Record r3("journal", 2005, 10, "author", "article");

    const char *cmd1 = "select year=[2001,2005]";
    const char *cmd2 = "select year=[2003,2004]";

    db.insert(r1);
    db.insert(r2);
    db.insert(r3);

    {
        Query q;
        assert(q.parse(cmd1) == true);

        Result res = db.process(q);
        assert(res.ok() == true);

        assert(res.count() == 3);
        assert(res.records().size() == 3);
        assert(res.records()[0] == r1);
        assert(res.records()[1] == r2);
        assert(res.records()[2] == r3);
    }

    {
        Query q;
        assert(q.parse(cmd2) == true);

        Result res = db.process(q);
        assert(res.ok() == true);

        assert(res.count() == 0);
        assert(res.records().size() == 0);
    }
}

void test3() {
    // DELETE test
    DataBase db("");

    Record r1("journal", 2001, 10, "author", "article");
    Record r2("journal", 2002, 10, "author", "article");
    Record r3("journal", 2005, 10, "author", "article");

    const char *cmd1 = "delete year=[2003,2004]";
    const char *cmd2 = "delete year=[2001,2005]";

    db.insert(r1);
    db.insert(r2);
    db.insert(r3);

    {
        Query q;
        assert(q.parse(cmd1) == true);

        Result res = db.process(q);
        assert(res.ok() == true);

        assert(res.count() == 0);//те которые хотели удалить

        assert(db.records().size() == 3);
        assert(db.records()[0] == r1);
        assert(db.records()[1] == r2);
        assert(db.records()[2] == r3);
    }

    {
        Query q;
        assert(q.parse(cmd2) == true);

        Result res = db.process(q);
        assert(res.ok() == true);

        assert(res.count() == 3);

        assert(db.records().size() == 0);
    }
}

void test4(){
    DataBase db("a.txt");
    
    Record r1("journal", 2001, 10, "author", "article");
    Record r2("journal", 2002, 10, "author", "article");
    Record r3("journal", 2005, 10, "author", "article");
    
    db.insert(r1);
    db.insert(r2);
    db.insert(r3);
    
    {
        Query q;
        assert(q.parse("save") == true);

        Result res = db.process(q);
        assert(res.ok() == true);

    }

}
void test5(){
    DataBase db("b.txt");
    
    Record r1("journal", 2001, 10, "author", "article");
    Record r2("journal", 2002, 10, "author", "article");
    Record r3("journal", 2005, 10, "author", "article");
    
    db.insert(r1);
    db.insert(r2);
    db.insert(r3);
    
    {
        Query q;
        assert(q.parse("clear") == true);

        Result res = db.process(q);
        assert(res.ok() == true);
        assert(db.records().empty());

    }

}

int main() {
    TestFunc tests[] = {test1, test2, test3, test4, test5};
    int testsCount = sizeof(tests) / sizeof(*tests);

    for (int i = 0; i < testsCount; i++) {
        printf("Test %2d\n", i + 1);
        tests[i]();
        printf("PASSED\n\n");
    }
}

