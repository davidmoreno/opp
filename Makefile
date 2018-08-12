all: sbds

CC=g++
CXXFLAGS="-std=gnu++17"

sbds: sbds.o serverboards.o spc.o symbol.o process.o io.o


test: sbds
	./sbds test

clean:
	rm -rf *.o sbds
