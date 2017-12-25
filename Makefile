CURRENT_DIR = $(shell pwd)

INCLUDES = -I$(CURRENT_DIR)/include

CC = gcc
CXX = g++

LIBS = -L$(CURRENT_DIR)/lib -lglfw -lassimp -ldl
CXXFLAGS = -Wl,-rpath=$(CURRENT_DIR)/lib -Wall -std=c++11

OBJS = Main.o modelLoader.o camera.o scanLineZBuffer.o glad.o

Z-Buffer: $(OBJS)
	${CXX} ${CXXFLAGS} ${INCLUDES} ${OBJS} -o $@ ${LIBS}

clean:
	-rm -f *.o core *.core

.cpp.o:
	${CXX} ${CXXFLAGS} ${INCLUDES} -c -o $@ $<

.c.o:
	${CC} ${INCLUDES} -c -o $@ $<