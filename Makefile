#! A refaire Je sais pas comment
#! MakeFile Are Ennemies
CC := gcc
CXX = g++
CFLAGS := -Wall -Wextra -Wpedantic -g
EXE := server client

all: $(EXE)

client: Src/util_func.o Src/tcp.o Src/tlv.o 
server: Src/util_func.o Src/tcp.o Src/tlv.o 

util_func.o: Header/util_func.h
tcp.o: Header/tcp.h
tlv.o: Header/util_func.h Header/tlv.hpp

clean:
	$(RM) $(EXE) *~ Src/*.o

.PHONY: all clean
