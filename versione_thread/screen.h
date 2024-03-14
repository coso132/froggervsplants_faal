#include "sprite.h"


//Macro per la funzione delle intersecazioni con i muri
#define TOPWALL 0
#define BOTTOMWALL 1
#define LEFTWALL 2
#define RIGHTWALL 3

//Numero di livelli di priorita' diversi
#define PLEVEL 10
//Sprite massime contenibili nell'array di screen
#define MAXSPRITES 70

//Colori creati da me
#define BLACK 0
#define WHITE 1
#define YELLOW 2
#define GREEN 3
#define BLUE 4
#define DARKG 5
#define BROWN 6
#define MAGENTA 7
#define DARKGREY 8
#define GREY 9
//Numero di colori creati
#define NCOLORS 10

/*
Struct rappresentante screen, contiene ogni informazione necessaria
per gestire una WINDOW di gioco, con le proprie sprite
*/
typedef struct screen
{
    //Larghezza in caratteri
    int sizex;
    //Altezza in caratteri
    int sizey;
    //Color pair dello sfondo della window
    int background;
    //Puntatore alla window di questo screen
    WINDOW * win;
    //Numero di sprite contenute nella window
    int nsprites;
    
    //Array di puntatori di sprites, che contiene nsprites sprites
    Sprite * sprites[MAXSPRITES];
    
} Screen;

void initializeScreenFunctions();

int screenSpriteIntersectsWall(Screen scr, Sprite sprit, int dir);
void screenRefreshSprites(Screen scr);
void screenAddSprite(Screen * scr, Sprite* sprite);
void screenRemoveSprite(Screen  scr, Sprite* sprite);
void screenInit(Screen * scr, int startX, int startY,int sizeX, int sizeY , int background );
void moveSprite(Screen scr, Sprite * sprite, int dirx, int diry);
void printSprite(Screen scr, Sprite sprit);
void printSpriteGetBackgroundColor(Screen scr, Sprite sprit);
void clearSprite(Screen scr, Sprite sprite);
void refreshBackground(Screen scr);