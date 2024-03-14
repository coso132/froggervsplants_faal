a.out: t1.o sprite.o screen.o 
	gcc t1.o sprite.o screen.o -lncursesw -l pthread -o a.out

sprite.o: sprite.h sprite.c
	gcc -c sprite.c -o sprite.o

screen.o: screen.h screen.c
	gcc -c screen.c -o screen.o

t1.o: t1.c
	gcc -c t1.c -o t1.o

clear: 
	clear

