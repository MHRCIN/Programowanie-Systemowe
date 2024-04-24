all: main

main.o: lab02.main.c
	gcc -c lab02.main.c 
lib.o: lib.c
	gcc -c lab02.lib.c
main: main.o lib.o
	gcc -o main lab02.main.c lab02.lib.c

clean: 
	rm -f main main.o lib.o