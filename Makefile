#! A refaire Je sais pas comment
#! MakeFile Are Ennemies
CC := gcc
CXX = g++
CFLAGS := -Wall -Wextra -Wpedantic -fanalyzer
CXXFLAGS  := -Wall -Werror -Wextra -pedantic 
EXE := server client

all: $(EXE)

client: Libs/Src/util_func.o Libs/Src/tcp.o Libs/Src/tlv.o Puiss4/p4.o
server: Libs/Src/util_func.o Libs/Src/tcp.o Libs/Src/tlv.o Libs/Src/server_func.o Puiss4/p4.o

server_func.o: Libs/server_func.h
util_func.o: Libs/util_func.h
tcp.o: Libs/tcp.h
tlv.o: Libs/util_func.h Libs/tlv.hpp
game.o: Puiss4/game.hpp

clean:
	$(RM) $(EXE) *~ $(shell find . -name "*.o")

.PHONY: all clean
