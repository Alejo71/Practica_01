CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
OBJS = xxhash.o hash.o menu.o search.o p1-dataProgram.o

all: p1

p1: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) p1


