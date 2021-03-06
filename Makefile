all: sbds

CC=g++
CXXFLAGS=-std=gnu++17 -O2 -Wall -Werror -g
LDFLAGS=-pthread -g

sbds: sbds.o serverboards.o popc.o symbol.o process.o io.o vm.o


test: sbds
	./sbds test

clean:
	rm -rf *.o sbds
