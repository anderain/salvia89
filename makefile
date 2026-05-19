CC 		= gcc
CFLAGS 	= -c -ansi -Werror -pedantic 
LDFLAGS =
EXE 	= salvia_test.exe

test: salvia.o salvia-test.o
	$(CC) $(LDFLAGS) salvia.o salvia-test.o -o $(EXE)

salvia.o: salvia.c salvia.h
	$(CC) $(CFLAGS) salvia.c -o salvia.o

salvia-test.o: salvia-test.c salvia.h 
	$(CC) $(CFLAGS) salvia-test.c

.PHONY: clean
clean:
	rm *.o $(EXE)
