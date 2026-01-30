CC=g++
CFLAGS=-Wall -std=c++17 -pthread
DYFLAGS=-Lbuild -ljournal -Wl,-rpath=build

.PHONY: all setup clean run

all: setup app clean run

run: app
	./build/app journal low

app: main.o journal.so
	$(CC) $(CFLAGS) -o build/app build/main.o $(DYFLAGS)

main.o: src/main/main.cpp
	$(CC) $(CFLAGS) -c -o build/main.o src/main/main.cpp

journal.so: journal.o
	$(CC) $(CFLAGS) -shared build/journal.o -o build/libjournal.so

journal.o: src/libJournal/journal.cpp
	$(CC) $(CFLAGS) -fPIC -c -o build/journal.o src/libJournal/journal.cpp

setup:
	mkdir -p build

clean:
	rm build/*.o