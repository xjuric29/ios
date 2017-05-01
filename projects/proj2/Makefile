#CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
CFLAGS=-std=gnu99 -Wall -Wextra

proj2: main.o proc.o
	gcc main.o proc.o -o proj2 -lpthread

proc.o: proc.c
	gcc ${CFLAGS} proc.c -c -lpthread

main.o: main.c proc.h
	gcc ${CFLAGS} main.c proc.h -c -lpthread
