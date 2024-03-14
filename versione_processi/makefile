a.out: t.o sprite.o screen.o clear
	gcc t.o sprite.o screen.o -lncursesw -o a.out

sprite.o: sprite.h sprite.c
	gcc -c sprite.c -o sprite.o

screen.o: screen.h screen.c
	gcc -c screen.c -o screen.o

t.o: t.c
	gcc -c t.c -o t.o

clear: 
	clear

