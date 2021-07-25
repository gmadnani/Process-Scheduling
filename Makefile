allocate: process.o allocate.o cpu.o list.o processor.o
	gcc -g -Wall allocate.o process.o cpu.o list.o processor.o -o allocate -lm

allocate.o: allocate.c
	gcc -g -c -Wall -o allocate.o allocate.c

process.o: process.c
	gcc -g -c -Wall -o process.o process.c

cpu.o: cpu.c
	gcc -g -c -Wall -o cpu.o cpu.c 

processor.o: processor.c
	gcc -g -c -Wall -o processor.o processor.c

list.o:list.c
	gcc -g -c -Wall -o list.o list.c

clean:
	rm -f *.o allocate