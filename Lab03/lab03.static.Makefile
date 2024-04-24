
all: main

main.o: lab03.static.main.c
	gcc -c lab03.static.main.c -o main.o

lib.o: lab03.static.lib.c
	gcc -c lab03.static.lib.c -o lib.o

lib.a: lib.o
	ar rcs lib.a lib.o

main: main.o lib.a
	gcc -o main main.o lib.a

clean: 
	rm -f main main.o lib.o lib.a