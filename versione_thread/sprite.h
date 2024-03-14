#include <ncurses.h>
#include <locale.h>
#include <wchar.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

/*
Struct Sprite che contiene tutte le informazioni necessarie per la memorizzazione
di una sprite nel gioco.
*/
typedef struct sprite
{
    

    //Identificativo univico per sprite, usata nell array di Screen
    int id;
    //Tipo della sprite, puo' essere FROG, CROC, PLANT, BKGD, HOLE
    int type;
    //Posizione nell asse x relativa alla screen associata
    int x;
    //Posizione nell asse y relativa alla screen associata
    int y;
    //Larghezza in caratteri della sprite
    int w;
    //Altezza in caratteri della sprite
    int h; 
    //Ordine di stampa (Valore piu' basso == stampato prima)
    int priority;
    //Puntatore a array contenente la pair di colori di ogni carattere
    short * clr;

    //Puntatore a array di caratteri unicode di ogni "pixel" della sprite
    wint_t* txt;

} Sprite;

// void printSprite(WINDOW * win, Sprite sprit);
// void printSpriteColor(WINDOW * win, Sprite sprit, int cl);
Sprite copySpriteFromPointer(Sprite * sprit);
Sprite * allocateSprite();
short * flipArray(short *a, int w, int h);
void copySprite(Sprite* s1, Sprite* s2);
int lineIntersect(int x1, int l1, int x2, int l2);
int spriteIntersects(Sprite s1, Sprite s2);
int spriteIntersects2(Sprite s1, Sprite s2);
int lineIntersect2(int x1, int l1, int x2, int l2);
void flipSprite(Sprite *s);