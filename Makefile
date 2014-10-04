C      = gcc
CFLAGS  = -g3 -std=c99 -Wall -pedantic
HWK     = /c/cs323/Hwk4

mainLex: mainLex.o lex.o history.o getLine.o
	$(CC) $(CFLAGS) -o mainLex mainLex.o lex.o history.o getLine.o


mainLex.o: mainLex.c
	$(CC) $(CFLAGS) -c mainLex.c


lex.o: lex.c
	$(CC) $(CFLAGS) -c lex.c

history.o: history.c 
	$(CC) $(CFLAGS) -c history.c

getLine.o: getLine.c
	$(CC) $(CFLAGS) -c getLine.c