CC = gcc -I ./include -c -fsanitize=address -g
LD = gcc -fsanitize=address -g


SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)



%.o: %.c
	$(CC) $< -o $@

all: $(OBJECTS)
	$(LD) $^ -o ld
	#./ld main.elf func.elf -o main
	./ld std_bios.elf -o std_bios


clean:
	rm *.o ld -rf
