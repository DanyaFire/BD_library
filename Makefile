FLAGS = -g -Wall -Wextra -Wfloat-equal -Werror -pedantic -std=gnu++0x

all: r w db db_test server client

r: r.o record.o common.o
	g++ $(FLAGS) -o r r.o record.o common.o

w: w.o record.o common.o
	g++ $(FLAGS) -o w w.o record.o common.o

db: db.o database.o record.o common.o query.o result.o
	g++ $(FLAGS) -o db db.o database.o record.o common.o query.o result.o
	
db_test: db_test.o database.o record.o common.o query.o result.o
	g++ $(FLAGS) -o db_test db_test.o database.o record.o common.o query.o result.o
	
server: server.o record.o common.o query.o database.o result.o
	g++ $(FLAGS) -o server server.o record.o common.o query.o database.o result.o
	
client: client.o record.o common.o query.o database.o result.o
	g++ $(FLAGS) -o client client.o record.o common.o query.o database.o result.o

common: common.o client.o record.o query.o database.o result.o
	g++ $(FLAGS) -o common common.o client.o record.o query.o database.o result.o

r.o: r.cpp record.h common.h
	g++ $(FLAGS) -c -o r.o r.cpp

w.o: w.cpp record.h common.h
	g++ $(FLAGS) -c -o w.o w.cpp
	
database.o: database.cpp record.h common.h query.h
	g++ $(FLAGS) -c -o database.o database.cpp
	
result.o:  result.cpp result.h common.h query.h record.h
	g++ $(FLAGS) -c -o result.o result.cpp

record.o: record.cpp record.h config.h
	g++ $(FLAGS) -c -o record.o record.cpp

common.o: common.cpp common.h 
	g++ $(FLAGS) -c -o common.o common.cpp

query.o: query.cpp query.h record.h config.h
	g++ $(FLAGS) -c -o query.o query.cpp

db.o: db.cpp common.h  query.h record.h
	g++ $(FLAGS) -c -o db.o db.cpp
	
db_test.o: db_test.cpp database.h query.h record.h result.h
	g++ $(FLAGS) -c -o db_test.o db_test.cpp
	
client.o: client.cpp common.h config.h query.h record.h result.h
	g++ $(FLAGS) -c -o client.o client.cpp

server.o: server.cpp common.h config.h database.h query.h record.h result.h
	g++ $(FLAGS) -c -o server.o server.cpp

clean:
	rm -f r w server client db *.o