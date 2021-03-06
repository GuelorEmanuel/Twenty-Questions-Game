CFLAGS = -g -std=c99
CC = gcc

a5: node.c
	$(CC) $(CFLAGS) node.c -o node

clean:
	rm -f *.o *.d node.out node

style:
	astyle --style=ansi node.c a5Defs.h

archive:
	tar cvf GuelorEmanuel.tar node.c a5Defs.h


