UNAME_S := $(shell uname -s)
CC = gcc
CFLAGS=-ggdb -Wall
ifeq ($(UNAME_S),Darwin)
    CC = clang
	CFLAGS=-g -Wall -fsanitize=address
endif
PROG=split_text

all: main.o processing.o io_utils.o alloc_utils.o
	$(CC) $(CFLAGS) $^ -o $(PROG)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f *.o $(PROG)

.PHONY: clean
