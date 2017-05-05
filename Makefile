CFLAGS = -g -Wall -std=c99

interpreter: interpreter.o parse.o stmt.o expr.o

interpreter.o: parse.h stmt.h expr.h

parse.o: parse.h stmt.h expr.h

stmt.o: stmt.h expr.h

expr.o: expr.h

clean:
	rm -f *.o
	rm -f interpreter
