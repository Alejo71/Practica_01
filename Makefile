# Makefile
CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=gnu11
INCLUDES = -I./lib/xxhash
SRCS = main.c hash.c
OBJS = $(SRCS:.c=.o)
LIBSRC = lib/xxhash/xxhash.c

all: p1_hash

# Sino quieres compilar xxhash desde el repo, puedes ajustar LIBSRC o compilar xxhash como librería.
p1_hash: $(OBJS) $(LIBSRC)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBSRC)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o p1_hash
