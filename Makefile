UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    CC=clang++
    CC+=-D_XOPEN_SOURCE
    LIBTHREAD=libthread_macos.o
else
    CC=g++13
    LIBTHREAD=libthread.o
endif

CC+=-g -Wall -std=c++20

# List of source files for your program
SOURCES=pizza.cpp

# Generate the names of the program's object files
OBJS=${SOURCES:.cpp=.o}

all: pizza

# Compile the program and tag this compilation
pizza: ${OBJS} ${LIBTHREAD}
	${CC} -o $@ $^ -ldl -pthread

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${OBJS} pizza
