CC = gcc
CFLAGS = -Wall -O2 -Ilib/xxhash
LIBS = -lrt -lpthread

all: ui search

ui: ui.c
	$(CC) $(CFLAGS) ui.c -o ui $(LIBS)

search: search.c hash.c lib/xxhash/xxhash.c
	$(CC) $(CFLAGS) search.c hash.c lib/xxhash/xxhash.c -o search $(LIBS)

clean:
	rm -f ui search
