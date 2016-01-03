CFLAGS = -g -std=c99
CC = gcc

a5: question2.c
	$(CC) $(CFLAGS) question2.c -o node

clean:
	rm -f *.o *.d question2.out node

style:
	astyle --style=ansi question2.c a5Defs.h

archive:
	tar cvf GuelorEmanuel.tar question2.c a5Defs.h


