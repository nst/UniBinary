CC=gcc
CFLAGS=-I. -Wall

unibinary: unibinary.o main.o
	$(CC) -o unibinary main.o unibinary.o $(CFLAGS)

tests: unibinary.o tests.o
	$(CC) -o tests unibinary.o tests.o $(CFLAGS)

clean:
	rm -rf *o unibinary tests
