# Compiler
CC = gcc

# Compiler flags
CFLAGS = -std=c99 -Werror -g

myshell: myshell.c
	$(CC) $(CFLAGS) $^ -o myshell

clean:
	rm -f myshell myhsell.o

