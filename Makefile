CFLAGS = -g -std=c99
CC = gcc

a5: question2.c
	$(CC) $(CFLAGS) question2.c -o a5

clean:
	rm -f *.o *.d question2.out a5

style:
	astyle --style=ansi question2.c a5Defs.h

archive:
	tar cvf GuelorEmanuel.tar question2.c a5Defs.h


