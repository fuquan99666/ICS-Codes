.PHONY: all compile link runtime clean

CC = gcc
CFLAGS = -Wall -O0

all: compile link runtime

compile: int_compile
	@echo "== compile-time interposition =="
	./int_compile.out

link: int_link
	@echo "== link-time interposition =="
	./int_link.out

runtime: int_runtime mymalloc.so
	@echo "== runtime interposition =="
	LD_PRELOAD=./mymalloc.so ./int_runtime.out

int_compile: int.c mymalloc.c malloc.h
	$(CC) $(CFLAGS) -I. -DCOMPILETIME -c mymalloc.c -o mymalloc_compile.o
	$(CC) $(CFLAGS) -I. -o $@.out int.c mymalloc_compile.o

int_link: int.c mymalloc.c
	$(CC) $(CFLAGS) -DLINKTIME -c mymalloc.c -o mymalloc_link.o
	$(CC) $(CFLAGS) -Wl,--wrap,malloc -Wl,--wrap,free -o $@.out int.c mymalloc_link.o

int_runtime: int.c
	$(CC) $(CFLAGS) -o $@.out int.c

mymalloc.so: mymalloc.c
	$(CC) $(CFLAGS) -DRUNTIME -shared -fPIC -o $@ mymalloc.c -ldl

clean:
	rm -f int_compile int_link int_runtime mymalloc.so mymalloc_compile.o mymalloc_link.o
