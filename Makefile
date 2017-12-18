INCLUDES = -I./include

CC = gcc
CXX = g++

LIBS = -L./lib -lglfw -lassimp -dl
CXXFLAGS = -Wall -std=c++11

OBJS = Main.o modelLoader.o camera.o

Z-Buffer: $(OBJS)
	${CXX} ${CXXFLAGS} ${INCLUDES} ${OBJS} -o $@ ${LIBS}

clean:
	-rm -f *.o core *.core

.cpp.o:
	${CXX} ${CXXFLAGS} ${INCLUDES} -c -o $@ $<