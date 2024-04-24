all: main

main.o: lab03.shared.main.c
	gcc -c lab03.shared.main.c -o main.o

lib.o: lab03.shared.lib.c
	gcc -fPIC -c lab03.shared.lib.c -o lib.o

lib.so: lib.o
	gcc -shared -o lib.so lib.o

main: main.o lib.so
	gcc -o main main.o -L ~/Lab3b -l:lib.so

clean: 
	rm -f main main.o lib.o lib.so