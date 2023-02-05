CC = gcc -I ./include -c -fsanitize=address -g
LD = gcc -lasan -g


SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)



%.o: %.c
	$(CC) $< -o $@

all: $(OBJECTS)
	$(LD) $^ -o ld
	./ld main.elf func.elf -o main


clean:
	rm *.o ld -rf
