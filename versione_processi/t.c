
// #include <ncurses.h>

// #include <locale.h>

// #include <wchar.h>

#include "screen.h"
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

// FPS a cui sono sincronizzati i processi di gioco
#define FPS 18
// Frametime in microsecondi
#define FRAMETIME 1000000 / FPS

//      TIPI SPRITE
#define FROG 0
#define CROC 1
#define SIDEWALK 2
#define GRASS 3
#define FLOW 4
#define TIMEBAR 5
#define PROJ 6
#define FROGSPARANTE 7
#define PLANT 8
#define PLANTSHOT 9
#define REDCROC 10
#define REDCROCFELL 11
#define PLANTDEAD 12
#define PROJPLANT 13
#define HOLE 14
#define HOLEFILLED 15

//      MISC

#define NVITE 3     //Numero di vite
#define SWIDTH 66   //Larghezza dello schermo di gioco
#define SHEIGHT 56  //Altezza dello schermo di gioco
#define NFLOWS 9    //Numero di flussi nel gioco
#define ELEMENTHEIGHT 4 //Altezza massima di un singolo elemento di gioco statico
#define NOFSPEEDS 3 //Numero di velocita' possibili assumibili dai coccodrilli
#define MAXNOFCHARS SWIDTH *ELEMENTHEIGHT   //Grandezza massima di una sprite
#define NGRASS 4    //Numero singole sprite erba che compongono l'argine superiore
#define NPREAD 2    //Divisore arbitrario non usato piu
#define DURATAMANCHE 180
#define NCROCS 15   //Numero massimo di coccodrilli nel gioco
#define NPLANTS 5   //Numero massimo di piante nel gioco
#define NHOLES 5   //Numero massimo di coccodrilli nel gioco
#define NFROGPROJS 3//Numero proiettili massimi della rana
#define NPLANTPROJS 12                    // Numero di proiettili massimi delle piante
#define INACTIVEPROCESS 9999999  //Valore pid che rappresenta processo non ancora inizializzato tramite fork (o terminato forzatamente dal programma)
#define NAPOLI -200                       // Luogo molto strano e lontano...
#define NAPOLIPROJFROG -20                // Luogo molto strano e lontano... per i proiettili delle rane
#define NAPOLIPROJPLANT -40               // Luogo molto strano e lontano... per i proiettili delle piante
#define PERCENTREDCROC 12                 // 1/x coccodrilli saranno creati rossi
#define SECONDIGIU 1                      // Quanto tempo sara' sott'acqua un coccodrillo


//Funzioni creazione sprite piu usate
Sprite createCroc()
{
    Sprite coccodrillo;
    coccodrillo.type = CROC;
    // coccodrillo.active=1;
    coccodrillo.priority = 4;
    coccodrillo.w = 18;
    coccodrillo.h = 4;
    coccodrillo.x = 0;
    coccodrillo.txt = L"█▙▄▄▄▀▙▖          "
                      " ▀▀▀▀███▀▄▀▄▀▄▀▄▄ "
                      "  ▄ ▄▀▄███▀█▀███▀▀"
                      " ▄█▀▀▀███▄███████▄";
    short arrayclr4[72] = {05L, 05L, 05L, 05L, 05L, 15, 05L, 05L, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                           1, 15, 05L, 15, 05L, 05, 05, 05, 56, 56, 56, 46, 56, 46, 56, 05L, 05L, 1,
                           1, 1, 41, 1, 41, 41, 41, 05, 05, 05, 56, 05, 56, 05, 05, 05, 56, 56,
                           1, 46, 46, 46, 46, 46, 46, 46, 05, 56, 05, 05, 05, 05, 05, 05, 05, 56};
    coccodrillo.clr=malloc(sizeof(short)*72);
    for (size_t i = 0; i < 72; i++)
    {
        coccodrillo.clr[i] = arrayclr4[i];
    }
    return coccodrillo;
}


Sprite createPlant(int type)
{
    Sprite plantSprite;
    plantSprite.type = PLANT;
    if(type==0)
    {

        plantSprite.type = PLANTDEAD;
    }
    plantSprite.x = 0;
    // frogPtr.active=1;
    plantSprite.w = 6;
    plantSprite.h = 8;
    plantSprite.y = 0;
    plantSprite.priority = 6;
    plantSprite.txt =  L"  ▟▙  "
                        " ▄▀▀▄ "
                        "▟▖▘█▗▙"
                        "▌▚▀▀▞▐"
                        "▙▌▟▙▐▟"
                        " ▌▜▛▐ "
                        "▛▞▄▄▚▟"
                        "▜████▛";
    
    if(type==0)
    {
        plantSprite.txt =  L"      "
                        "      "
                        "  ▗▖  "
                        "  ▛█  "
                        "  █▞  "
                        "  ▞█  "
                        "▛▞█▞▗▟"
                        "▜████▛";
    }
    
                        
    short clrfinalenormale[48] = {00, 00, 00, 00, 00, 00,
                                  00, 00, 00, 00, 00, 00,
                                  00, 00, 05, 05, 00, 00,
                                  00, 00, 51, 05, 00, 00,
                                  00, 00, 05, 05, 00, 00, 
                                  00, 00, 55, 55, 00, 00, 
                                  51, 05, 35, 35, 05, 15,
                                  05, 55, 55, 55, 55, 05};
             
    short clrfinalenormale2[48] = {00, 00, 05, 05, 00, 00,
                                  00, 05, 35, 35, 05, 00,
                                  05, 35, 31, 33, 35, 05,
                                  35, 35, 35, 35, 35, 35,
                                  35, 35, 35, 35, 35, 35,
                                  00, 35, 35, 35, 35, 00,
                                  51, 05, 35, 35, 05, 15,
                                  05, 55, 55, 55, 55, 05};

    plantSprite.clr=malloc(sizeof(short)*48);
    for (size_t i = 0; i < 48; i++)
    {
        if(type)
        {
            plantSprite.clr[i] = clrfinalenormale2[i];
        }    
        else
        {
            plantSprite.clr[i] = clrfinalenormale[i];
        }
    }
    return plantSprite;
}

Sprite createFrog()
{
    Sprite frogSprite;
    frogSprite.type = FROG;
    frogSprite.x = 0;
    // frogPtr.active=1;
    frogSprite.w = 6;
    frogSprite.h = 4;
    frogSprite.y = 0;
    frogSprite.priority = 9;

    frogSprite.txt = L"▟▗▞▗▖▙"
                "▐▟▘▝▙▌"
                "▐▜▚█▛▌"
                "▜ ▀▀ ▛";
    short clrfinalenormale[30] = {03, 07, 23, 23, 07, 03,
                          03, 03, 23, 23, 03, 03,
                          05, 05, 23, 2, 05, 05,
                          05, 05, 05, 05, 05, 05};

    frogSprite.clr=malloc(sizeof(short)*30);
    for (size_t i = 0; i < 30; i++)
    {
        frogSprite.clr[i] = clrfinalenormale[i];
    }
    return frogSprite;
}

Sprite createSidewalk()
{
    Sprite marciapiede;
    marciapiede.x = 0;
    marciapiede.type = SIDEWALK;
    marciapiede.y = 0;
    marciapiede.priority = 0;
    marciapiede.w = SWIDTH;
    marciapiede.h = ELEMENTHEIGHT;
    // marciapiede.txt = L"█▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌█"
    //                   "▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄▗▄▄"
    //                   "▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀▝▀▀"
    //                   "█▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌██▌█";

    marciapiede.txt = L"▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀"
                      "  ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖  "
                      "▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█"
                      "▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟";

    marciapiede.txt = L"▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀"
                      "  ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖  "
                      "▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█▛▀▀▀▀█"
                      "▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟▖    ▟";
    marciapiede.clr = malloc(sizeof(short) * MAXNOFCHARS);
    for (int i = 0; i < MAXNOFCHARS; i++)
    {
        marciapiede.clr[i] = 89;
    }
    return marciapiede;
}

Sprite createProj()
{
    Sprite proiettile;
    proiettile.type = PROJ;
    proiettile.priority = 8;
    proiettile.w = 3;
    proiettile.h = 2;
    proiettile.y = 0;
    proiettile.x = -1;
    proiettile.txt = L" ▟ "
                      " ▛ ";
    
    short arrayclrpro[6] = {06, 06,06,06,06,06};

    proiettile.clr=malloc(sizeof(short)*6); 
    for (size_t i = 0; i < 6; i++)
    {
        proiettile.clr[i]=arrayclrpro[i];
    }
    
    return proiettile;
}

Sprite createCoin()
{
    Sprite coin;
    coin.type = 20;
    coin.priority = 8;
    coin.w = 2;
    coin.h = 2;
    coin.y = 0;
    coin.x = 0;
    coin.txt = L"▟▙"
                "▜▛";
        
    short arrayclrpro[4] = {02, 02,02,06};

    coin.clr=malloc(sizeof(short)*4); 
    for (size_t i = 0; i < 4; i++)
    {
        coin.clr[i]=arrayclrpro[i];
    }
    
    return coin;
}

Sprite createHole(int filled)
{
    Sprite hole;
    hole.type = HOLE;
    hole.priority = 0;
    hole.w = 6;
    hole.h = 4;
    hole.y = 0;
    hole.x = 0;
    hole.txt = L"▟████▙"
                "██████"
                "██████"
                "▜████▛";
    short * colorifinale;
    short arrayclrpro[24] = {56,66,66,66,66,56,
                             66,66,66,66,66,66,
                             66,66,66,66,66,66,
                             56,66,66,66,66,56};
    colorifinale=arrayclrpro;
    if(filled)
    {

        hole.txt = L"▟▗▞▗▖▙"
                    "▐▟▘▝▙▌"
                    "▐▜▚█▛▌"
                    "▜ ▀▀ ▛";
                    // DARKG,5,YELLOW,2,GREEN,3,BROWN,6;
        short arrayclrpro2[24] =    {63, 67, 23, 23, 67, 63,
                                    63, 23, 23, 23, 23, 63,
                                    65, 65, 23, 62, 65, 65,
                                    65, 65, 65, 65, 65, 65};
        colorifinale=arrayclrpro2;

        hole.type = HOLEFILLED;
    }

    hole.clr=malloc(sizeof(short)*24); 
    for (size_t i = 0; i < 24; i++)
    {
        hole.clr[i]=colorifinale[i];
    }
    
    return hole;
}

Sprite createFlow(int tipo)
{
    Sprite flusso;
    wint_t *flussotxt = L"                                                                  "
                        "  ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗  "
                        "  ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘  "
                        "                                                                  ";
    wint_t *flussotxt2 = L"                                                                  "
                         "▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗▗    ▗"
                         "▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘▘    ▘"
                         "                                                                  ";

    flusso.clr = malloc(sizeof(short) * MAXNOFCHARS);
    // short flussiclr[MAXNOFCHARS] = {41};
    for (int i = 0; i < MAXNOFCHARS; i++)
    {
        flusso.clr[i] = 41;
    }

    flusso.x = 0;
    flusso.y = 0;
    flusso.priority = 0;
    flusso.type = FLOW;
    flusso.w = SWIDTH;
    flusso.h = ELEMENTHEIGHT;

    if (tipo)
        flusso.txt = flussotxt;
    else
        flusso.txt = flussotxt2;
    return flusso;
}

Sprite createGrass(int tipo)
{
    Sprite erba;

    wint_t *erbaSprite = L"▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌"
                         "▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐"
                         "▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌"
                         "▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐";

    wint_t *erbaSprite2 = L"▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌"
                          "▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐"
                          "▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌"
                          "▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟▟";
    if (tipo)
        erba.txt = erbaSprite2;
    else
        erba.txt = erbaSprite;
    
    erba.clr = malloc(sizeof(short) * MAXNOFCHARS);
    for (int i = 0; i < MAXNOFCHARS; i++)
    {
        erba.clr[i] = 65;
    }

    erba.x = 0;
    erba.y = 0;
    erba.priority = 0;
    erba.w = SWIDTH;
    erba.type = GRASS;
    erba.h = ELEMENTHEIGHT;
    return erba;
}

//Funzioni scambio sprite su pipe, permettono di ricevere ed inviare sprites in una pipe in modo atomico
Sprite *copySpriteFromPipe(int pipe[2])
{


    char *chars = malloc(sizeof(Sprite) + ((sizeof(short) + sizeof(wint_t)) * 288));
    Sprite *txtsottosinistra = (Sprite *)chars;
    int returnValue = read(pipe[0], chars, sizeof(Sprite) + ((sizeof(short) + sizeof(wint_t)) * 288));

    int spriteSize = txtsottosinistra->w * txtsottosinistra->h;
    Sprite *returnval = malloc(sizeof(Sprite) + ((sizeof(short) + sizeof(wint_t)) * spriteSize));
    returnval->x = txtsottosinistra->x;
    returnval->y = txtsottosinistra->y;
    returnval->w = txtsottosinistra->w;
    returnval->h = txtsottosinistra->h;
    returnval->id = txtsottosinistra->id;
    returnval->priority = txtsottosinistra->priority;
    returnval->type = txtsottosinistra->type;


    returnval->clr = (short *)(chars + sizeof(Sprite));
    returnval->txt = (wint_t *)(chars + sizeof(Sprite) + ((sizeof(short)) * spriteSize));

    for (int i = 0; i < spriteSize; i++)
    {

        returnval->clr[i] = ((short *)(chars + sizeof(Sprite)))[i];
        returnval->txt[i] = ((wint_t *)(chars + sizeof(Sprite) + (sizeof(short) * spriteSize)))[i];
    }

    free(txtsottosinistra);
    return returnval;
}


void sendSpriteToPipe(int pipe[2], Sprite sprit)
{
    Sprite *returnval;

    int spriteSize = sprit.w * sprit.h;

    char *chars = malloc(sizeof(Sprite) + ((sizeof(short) + sizeof(wint_t)) * 288));
    returnval = (Sprite *)chars;
    returnval->clr = (short *)(chars + sizeof(Sprite));
    returnval->txt = (wint_t *)(chars + sizeof(Sprite) + sizeof(short) * spriteSize);
    returnval->x = sprit.x;
    returnval->y = sprit.y;
    returnval->w = sprit.w;
    returnval->h = sprit.h;
    returnval->type = sprit.type;
    returnval->id = sprit.id;
    returnval->priority = sprit.priority;
    for (int i = 0; i < spriteSize; i++)
    {
        returnval->clr[i] = sprit.clr[i];
        returnval->txt[i] = sprit.txt[i];
    }
    int returnValue = write(pipe[1], returnval, sizeof(Sprite) + ((sizeof(short) + sizeof(wint_t)) * 288));
    free(returnval);
}
int modificaTempo(Sprite *barra, int *time, int );
void frog(Screen, int *, int, int *, int *, int);
int controllo(Screen, Screen, Screen, int[MAXSPRITES][2], int *,int*);
void croc(Screen scr, int id, int dir, int pipesprites[], int *, int y, int x, int type);

void projectile(Screen scr, int id, int pipesprites[], int y, int x, int diry);

void timeBar2(int pipeMain[], int);
void plant(Screen scr, int id, int pipesprites[], int x,int type);
void mainPrint(Screen, Screen, int *);


int main()
{
    // Crea seed random
    srand(time(NULL));
    // Serve per inizializzare lo unicode?
    setlocale(LC_ALL, "");
    // Impostazioni stdscr
    initializeScreenFunctions();

    //Check grandezza shcerm,o
    if (LINES <= 58 || COLS <= 72)
    {
        endwin();
        printf("\nGrandezza schermo insufficiente, ingrandire schermata (anche andando fullscreen con F11), oppure ridurre lo zoom del terminale.\n ");
        exit(1);
    }

    // Inizializzo wins
    Screen scrtop;
    screenInit(&scrtop, 0, 0, SWIDTH, 1, 01);
    

    Screen game;
    screenInit(&game, 0, 1, SWIDTH, SHEIGHT, 41);
    game.background = 44;

    Screen scrbot;
    screenInit(&scrbot, 0, SHEIGHT + 1, SWIDTH, 2, 11);

    nodelay(game.win, 1);
    keypad(game.win, 1);
    // nocbreak();
    //
    // INIZIALIZZAZIONE DELLE SPRITE DI BACKGROUND


    //      FLUSSI DEL FIUME
    Sprite flussi[NFLOWS];
    Sprite *pF[NFLOWS];
    for (size_t i = 0; i < NFLOWS; i++)
    {
        flussi[i] = createFlow(i % 2);
        flussi[i].y = game.sizey - (ELEMENTHEIGHT * (i + 2));
        pF[i] = allocateSprite();
        copySprite(pF[i], &flussi[i]);
        screenAddSprite(&game, pF[i]);
    }


    //      MARCIAPIEDE IN BASSO
    Sprite marciapiede = createSidewalk();
    marciapiede.y = game.sizey - ELEMENTHEIGHT;
    Sprite *pM = allocateSprite();
    copySprite(pM, &marciapiede);
    screenAddSprite(&game, &marciapiede);

    //      ERBA IN ALTO
    Sprite erba[NGRASS];
    Sprite *pE[NGRASS];
    for (size_t i = 0; i < NGRASS; i++)
    {
        erba[i] = createGrass(i == NGRASS - 1);
        erba[i].y = ELEMENTHEIGHT * (i );
        pE[i] = allocateSprite();
        copySprite(pE[i], &erba[i]);
        screenAddSprite(&game, pE[i]);
    }

    //      MONETE (VITA)
    Sprite coins[NVITE];
    Sprite *pC[NVITE];
    wclear(scrbot.win);
    for (size_t i = 0; i < NVITE; i++)
    {
        coins[i] = createCoin();
        coins[i].x = 1+((1+coins[i].w)*i);
        pC[i] = allocateSprite();
        copySprite(pC[i], &coins[i]);
        screenAddSprite(&scrbot, pC[i]);
        printSprite(scrbot,*pC[i]);
    }
    wrefresh(scrbot.win);
    int holesId[NHOLES];    
    Sprite **holes = malloc((NHOLES) * sizeof(Sprite*));
    int startIdHoles = game.nsprites;
    for (size_t i = 0; i < NHOLES; i++)
    {
        holes[i]=allocateSprite();
        *holes[i]=createHole(0);
        holes[i]->x=(6+(i*2*6));
        holesId[i] = game.nsprites;
        screenAddSprite(&game, holes[i]);
    }

    //
    // FINE INIZIALIZZAZIONE DELLE SPRITE DI BACKGROUND

    
    int giocare = 1;    // Se il gioco e' attivo o meno
    int vite;           // Numero di vite
    while (giocare)
    {

        // Crea le tane
        for (size_t i = 0; i < NHOLES; i++)
        {
            
            *holes[i]=createHole(0);
            holes[i]->x=(6+(i*2*6));// Formula per la posizione delle tane
        }
        vite = NVITE;// Imposto le vite
        // Imposto la sprite delle vite
        for (size_t i = 0; i < NVITE; i++)
        {
            scrbot.sprites[i]->txt =   L"▟▙"
                                        "▜▛";

        }
        int punteggio=0;    // Inizializzo il punteggio a 0
        //Commentato questa parte del codice nella versione dei threads
        while (vite > 0)    // Fino a quando la rana e' in vita
        {
            // Stampa il punteggio
            wclear(scrtop.win);
            mvwprintw(scrtop.win,0,0,"PUNTEGGIO ATTUALE : %d",punteggio);
            wrefresh(scrtop.win);

            werase(game.win);
            flushinp();

            if(controllo(game, scrbot, scrtop, NULL, NULL,&punteggio)==0)
            {
                vite--;
                scrbot.sprites[vite]->txt=L"    ";
            }
            else
            {
                size_t i;
                for (i = 0; i < NHOLES; i++)
                {
                    if(holes[i]->type==HOLE)
                    {
                        i=NHOLES+1;
                    }
                }
                if(i==NHOLES)
                {
                    vite=-1;
                }
                
            }
        }
        // Pulisci lo schermo e cambiagli colore di sfondo
        wclear(game.win);
        wbkgd(game.win, COLOR_PAIR(01));
        nodelay(game.win, 0);
        // Se il giocatore ha viinto
        if (vite == -1)
        {

            mvwprintw(game.win, SHEIGHT / 2, 4, "HAI VINTO! PUNTEGGIO TOTALE = %d", punteggio);
        }
        else // Se ha perso
        {
            mvwprintw(game.win, SHEIGHT / 2, 4, "HAI PERSO :( PUNTEGGIO TOTALE = %d", punteggio);
        }
        // Pulisci tutte le altre schermate (punteggio e tempo/vite)
        wclear(scrtop.win);
        wclear(scrbot.win);
        wrefresh(scrtop.win);
        wrefresh(scrbot.win);

        mvwprintw(game.win, SHEIGHT / 2 + 2, 4, "PREMI SPAZIO PER GIOCARE DI NUOVO,\n    PREMI OGNI ALTRO TASTO PER USCIRE");

        wrefresh(game.win);
        sleep(1);
        flushinp();
        // Ottieni input, se e' spazio si continua a giocare, senno termina
        if (wgetch(game.win) == 32)
        {
            giocare = 1;
        }
        else
        {
            giocare = 0;
        }
        wclear(game.win);
        nodelay(game.win, 1);
    }
    endwin();
    exit(3);
}

int controllo(Screen game, Screen scrbot, Screen scrtop, int pipesprites[MAXSPRITES][2], int pipebool[], int *punteggio)
{
    //Variabili utilizzate per la logica di gioco
    int flussi[NFLOWS],direzioni[NFLOWS]; //Array di interi che rappresenta la velocita' e direzione di ogni flusso, valore negativo==sinistra e viceversa, valore assoluto alto==velocita' bassa e viceversa
    int velocita;   //Variabile di appoggio che rappresenta la velocita' in valore assoluto di un determinato flusso alla sua creazione
    int flussoscelto;   //Rappresenta l'indice del flusso che e' stato scelto dall'algoritmo che associa un coccodrillo a un flusso
    int flussoleggero = 0;  //Flusso con numero minore di coccodrilli
    int arrayflussiminori[NFLOWS];  //Array che contiene gli indici di tutti i flussi che contendono la posizione come flusso con il minor numero di coccodrilli
    int contFlussoMinore = 0;   //Numero di flussi che condividono il minor numero di coccodrilli 
    int crocsaflusso[NFLOWS];   //Array di interi che contengono il numero di coccodrilli attualmente presenti nel flusso corrispondente
    int contCrocs = 0;  //Coccodrilli attivi sullo schermo al momento

    int nProcesses = 0;  //Numero processi di sprite attivi ad ogni momento
    int pidPrint=INACTIVEPROCESS,pidFrog=INACTIVEPROCESS;   //Pid processi principali
    
    int spriteSize; //Variabile di appoggio, memorizza la grandezza dell'ultima sprite prelevata
    int continua = 1;   //Bool, determina se continuare il gioco o no (morte rana o vincita manche)
    clock_t start, end, t, t2=0; //Variabili utilizzate per il conteggio di cicli di clock di alcune parti del codice, utile per sincronizzazione
    
    int sburacchio = 0;//BOMBOCLAAT

    //Creazione ed inizializzazione di tutte le pipe del gioco
    int pipePrint[2],pipeMain[2];
    // Creo pipe per comunicazione col processo stampa
    if (pipe(pipePrint) == -1)
    {
        endwin();
        perror("Pipe call (Print)");
        exit(99);
    }

    // Creo pipe per la comunicazione delle pipe
    if (pipe(pipeMain) == -1)
    {
        endwin();
        perror("Pipe call");

        exit(1);
    }

    //Generazione casuale dei flussi
    for (size_t i = 0; i < NFLOWS; i++)
    {
        //Imposta casualmente la direzione
        if (rand() % 2)
            direzioni[i] = 1;
        else
            direzioni[i] = -1;
        
        if(i>1 && direzioni[i]==direzioni[i-1]&&direzioni[i] ==direzioni[i-2])
        {
            direzioni[i]=-1*direzioni[i];
        }
        
        velocita=(rand() % NOFSPEEDS) + 1;  //Imposta casualmente la velocita', da 1 a NOFSPEEDS
        flussi[i] = direzioni[i] * (velocita); //Moltiplica la direzione per la velocita, assegnala al flusso corrispondente
        crocsaflusso[i] = 0;    //Imposta coccodrilli presenti in quel flusso a 0
        game.sprites[i]->txt[0]=direzioni[i] ;
        
    }


    //Creazione Sprite di gioco, inizializzazione e assegnazione allo schermo corretto

    //Da rifare, memory leak
    Sprite *frogPtr = allocateSprite();
    screenAddSprite(&game, frogPtr);
    
    //  PROIETTILI RANE
    Sprite **projs = malloc((NFROGPROJS ) * sizeof(Sprite*));
    for (size_t i = 0; i < NFROGPROJS; i++)
    {
        projs[i]=allocateSprite();
    }
    
    int startIdProj = game.nsprites;
    int projIds[NFROGPROJS ];
    pid_t pidProjs[NFROGPROJS ];
    int contprojfrog = 0;

    for (int i = 0; i < NFROGPROJS ; i++)
    {
        pidProjs[i] = INACTIVEPROCESS;
        projIds[i] = game.nsprites;
        screenAddSprite(&game, projs[i]);
        projIds[i] = projs[i]->id;
    }

    //  COCCODRILLI
    int pidCrocs[NCROCS];
    int crocsid[NCROCS];    
    Sprite **crocs = malloc((NCROCS) * sizeof(Sprite*));
    for (size_t i = 0; i < NCROCS; i++)
    {
        crocs[i]=allocateSprite();
    }

    int startIdCrocs = game.nsprites;
    for (int i = 0; i < NCROCS; i++)
    {
        pidCrocs[i] = INACTIVEPROCESS;
        crocsid[i] = game.nsprites;
        screenAddSprite(&game, crocs[i]);
        crocsid[i] = crocs[i]->id;
    }

    //  PIANTE
    int pidPlants[NPLANTS];
    int plantsId[NPLANTS];    
    Sprite **plants = malloc((NPLANTS) * sizeof(Sprite*));
    for (size_t i = 0; i < NPLANTS; i++)
    {
        plants[i]=allocateSprite();
    }
    clock_t clock1;
    int startIdPlants = game.nsprites;
    for (int i = 0; i < NPLANTS; i++)
    {
        pidPlants[i] = INACTIVEPROCESS;
        plantsId[i] = game.nsprites;
        screenAddSprite(&game, plants[i]);
        plantsId[i] = plants[i]->id;
        usleep(3000);
        if((pidPlants[i]=fork())==0)
        {
            srand(clock1=clock());
            plant(game,plantsId[i],pipeMain,(6+(i*2*6)),PLANT);
        }
    }

    //  PROIETTILI PIANTE

    Sprite **projsPlants = malloc((NPLANTPROJS ) * sizeof(Sprite*));
    for (size_t i = 0; i < NPLANTPROJS; i++)
    {
        projsPlants[i]=allocateSprite();
    }
    

    int startIdProjPlant = game.nsprites;
    int projPlantIds[NPLANTPROJS ];
    pid_t pidProjPlant[NPLANTPROJS ];

    for (int i = 0; i < NPLANTPROJS ; i++)
    {
        pidProjPlant[i] = INACTIVEPROCESS;
        projPlantIds[i] = game.nsprites;
        screenAddSprite(&game, projsPlants[i]);
        projPlantIds[i] = projsPlants[i]->id;
    }
    

    //Creazione processo per la gestione della grafica
    pidPrint = fork();
    if (pidPrint < 0)
    {
        endwin();
        exit(3);
    }
    else if (pidPrint == 0)
    {
        mainPrint(game, scrbot, pipePrint);
        endwin();
        exit(3);
    }

    //Creazione processo per la rana
    pidFrog = fork();
    nProcesses++;
    if (pidFrog < 0)
    {
        endwin();
        exit(3);
    }
    else if (pidFrog == 0)
    {

        frog(game, flussi, NFLOWS, pipeMain, pipeMain, frogPtr->id);
        endwin();
        exit(3);
    }

    int pidTimeBar=fork();
    if(pidTimeBar==0)
    {
        timeBar2(pipeMain,DURATAMANCHE);
        endwin();
        exit(3);
    }


    Sprite *spriteLetta;
    int larghezzaBarra=0;
    //      INIZIO LOOP DI GIOCO
    while (continua==1)
    {
        // Inizia conto clock, serve per sapere quando spawnare i coccodrilli (outdated, meglio usare i frame per questa versione)
        t = 0;

        //  Loop principale, cicla la lettura e gestione di un tot di sprite, in base al numero di processi attivi (aumentando il cont t)
        while (t < (nProcesses / NPREAD) + 2)
        {
            //Inizia il clock che dira' il tempo passato alla porzione di codice che genera coccodrilli
            start = clock();

            //Leggi dalla pipe principale una sprite
            spriteLetta = copySpriteFromPipe(pipeMain);
            spriteSize = spriteLetta->h * spriteLetta->w;

            //Invia la pipe al processo per la stampa delle sprite
            write(pipePrint[1], spriteLetta, sizeof(Sprite));
            write(pipePrint[1], spriteLetta->txt, (sizeof(wint_t) * spriteSize));
            write(pipePrint[1], spriteLetta->clr, (sizeof(short) * spriteSize));

            //Gestisci la sprite in base al tipo
            switch (spriteLetta->type)
            {
            //Casistiche gia commentate sulla versione thread.
            case PLANTSHOT:
                for (size_t i = 0; i < NPLANTPROJS; i++)
                {
                    if (pidProjPlant[i] == INACTIVEPROCESS)
                    {
                        
                        pidProjPlant[i] = fork();
                        if (pidProjPlant[i] < 0)
                        {
                            endwin();
                            exit(3);
                        }
                        else if (pidProjPlant[i] == 0)
                        {
                            // nProcesses++;

                            projectile(game, projPlantIds[i], pipeMain, spriteLetta->y + 3, (spriteLetta->x) + (spriteLetta->w / 2) -1, 1);
                            endwin();
                            exit(3);
                        }
                        i = NPLANTPROJS;
                    }
                }
            case PLANTDEAD:
            case PLANT:
                copySprite(game.sprites[spriteLetta->id], spriteLetta);
                break;
            case PROJPLANT:
                
                copySprite(game.sprites[spriteLetta->id], spriteLetta);
                int projid=spriteLetta->id-startIdProjPlant;
                if (screenSpriteIntersectsWall(game, *projsPlants[projid], BOTTOMWALL))
                {
                    kill(pidProjPlant[projid], SIGUSR1);

                    projsPlants[projid]->x=NAPOLIPROJPLANT;
                    projsPlants[projid]->y=NAPOLIPROJPLANT;
                    write(pipePrint[1], projsPlants[projid], sizeof(Sprite));
                    write(pipePrint[1], projsPlants[projid]->txt, (sizeof(wint_t) * spriteSize));
                    write(pipePrint[1], projsPlants[projid]->clr, (sizeof(short) * spriteSize));
                    
                    pidProjPlant[projid] = INACTIVEPROCESS;
                    contprojfrog--;
                }
                else
                {
                    if (spriteIntersects2(*frogPtr,*spriteLetta))
                    {
                        
                        

                        // projsPlants[projid]->x=-100;
                        // write(pipePrint[1], projsPlants[projid], sizeof(Sprite));
                        // write(pipePrint[1], projsPlants[projid]->txt, (sizeof(wint_t) * spriteSize));
                        // write(pipePrint[1], projsPlants[projid]->clr, (sizeof(short) * spriteSize));
                        
                        
                        //contprojfrog--;
                        continua=0;
                        //kill(pidProjPlant[projid], SIGUSR1);pidProjs[projid] = INACTIVEPROCESS;
                    }
                    else
                    {
                        for (size_t i = 0; i < NFROGPROJS; i++)
                        {
                            if (spriteIntersects2(*projs[i],*projsPlants[projid]))
                            {
                                *punteggio+=10;
                                kill(pidProjs[i], SIGUSR1);
                                pidProjs[i] = INACTIVEPROCESS;
                                contprojfrog--;
                                projs[i]->x=NAPOLIPROJFROG;
                                projs[i]->y=NAPOLIPROJFROG;
                                write(pipePrint[1], projs[i], sizeof(Sprite));
                                write(pipePrint[1], projs[i]->txt, (sizeof(wint_t) * projs[i]->w*projs[i]->h));
                                write(pipePrint[1], projs[i]->clr, (sizeof(short) *  projs[i]->w*projs[i]->h));
                                
                                

                                kill(pidProjPlant[projid], SIGUSR1);
                                pidProjPlant[projid] = INACTIVEPROCESS;
                                projsPlants[projid]->x=NAPOLIPROJPLANT;
                                projsPlants[projid]->y=NAPOLIPROJPLANT;
                                write(pipePrint[1],projsPlants[projid], sizeof(Sprite));
                                write(pipePrint[1], projsPlants[projid]->txt, (sizeof(wint_t) * spriteSize));
                                write(pipePrint[1], projsPlants[projid]->clr, (sizeof(short) * spriteSize));
                                
                                
                                i=NFROGPROJS;
                            }
                        }
                    }
                    
                    
                }
                break;
            case PROJ:
                {
                copySprite(game.sprites[spriteLetta->id], spriteLetta);
                int projid=spriteLetta->id-startIdProj;
                if (screenSpriteIntersectsWall(game, *spriteLetta, TOPWALL))
                {
                    kill(pidProjs[projid], SIGUSR1);

                    game.sprites[spriteLetta->id]->x=NAPOLIPROJFROG;
                    game.sprites[spriteLetta->id]->y=NAPOLIPROJFROG;
                    write(pipePrint[1], game.sprites[spriteLetta->id], sizeof(Sprite));
                    write(pipePrint[1], game.sprites[spriteLetta->id]->txt, (sizeof(wint_t) * spriteSize));
                    write(pipePrint[1], game.sprites[spriteLetta->id]->clr, (sizeof(short) * spriteSize));
                    
                    pidProjs[projid] = INACTIVEPROCESS;
                    contprojfrog--;
                }
                else
                {
                    for (size_t i = 0; i < NPLANTS; i++)
                    {
                        if(spriteIntersects2(*spriteLetta,*plants[i]) && plants[i]->type!=PLANTDEAD)
                        {
                            if(pidProjs[projid]==INACTIVEPROCESS)
                            {

                            }
                            else
                            {
                                kill(pidProjs[projid], SIGUSR1);
                                projs[projid]->x=NAPOLIPROJFROG;
                                projs[projid]->y=NAPOLIPROJFROG;
                                write(pipePrint[1], projs[projid], sizeof(Sprite));
                                write(pipePrint[1], projs[projid]->txt, (sizeof(wint_t) * spriteSize));
                                write(pipePrint[1], projs[projid]->clr, (sizeof(short) * spriteSize));

                                pidProjs[projid]=INACTIVEPROCESS;
                                contprojfrog--;
                            }
                            
                            *punteggio+=100;
                            kill(pidPlants[i], SIGUSR1);
                            if((pidPlants[i]=fork())==0)
                            {
                                plant(game,i+startIdPlants,pipeMain,plants[i]->x,PLANTDEAD);
                            }
                            
                        }
                    }
                    for (size_t i = 0; i < NCROCS; i++)
                    {
                        
                        if(spriteIntersects2(*spriteLetta,*crocs[i]) && crocs[i]->type!=REDCROCFELL)
                        {
                            kill(pidProjs[projid], SIGUSR1);
                            game.sprites[spriteLetta->id]->x=NAPOLI;
                            game.sprites[spriteLetta->id]->y=NAPOLI;
                            write(pipePrint[1], game.sprites[spriteLetta->id], sizeof(Sprite));
                            write(pipePrint[1], game.sprites[spriteLetta->id]->txt, (sizeof(wint_t) * spriteSize));
                            write(pipePrint[1], game.sprites[spriteLetta->id]->clr, (sizeof(short) * spriteSize));
                            
                            pidProjs[projid] = INACTIVEPROCESS;
                            contprojfrog--;

                            if(crocs[i]->type==REDCROC && pidCrocs[i]!=INACTIVEPROCESS )
                            {
                                *punteggio+=250;
                                kill(pidCrocs[i],SIGUSR1);
                                pidCrocs[i] = fork();
                                if (pidCrocs[i] < 0)
                                {
                                    endwin();
                                    exit(3);
                                }
                                else if (pidCrocs[i] == 0)
                                {
                                    // nProcesses++;
                                    croc(game, crocs[i]->id, flussi[NFLOWS-2-(crocs[i]->y-20)/4], pipeMain, pipeMain, crocs[i]->y, crocs[i]->x, CROC);
                                    endwin();
                                    exit(3);
                                }
                            }
                        }
                    }
                }
                }
                break;
            case FROGSPARANTE:
                if (contprojfrog < NFROGPROJS)
                {
                    contprojfrog++;
                    // nProcesses++;
                    for (size_t i = 0; i < NFROGPROJS; i++)
                    {
                        if (pidProjs[i] == INACTIVEPROCESS)
                        {

                            pidProjs[i] = fork();
                            if (pidProjs[i] < 0)
                            {
                                endwin();
                                exit(3);
                            }
                            else if (pidProjs[i] == 0)
                            {
                                // nProcesses++;

                                projectile(game, projIds[i], pipeMain, frogPtr->y - 1, (frogPtr->x) + (frogPtr->w / 2) - (i % 2)-1, -1);
                                endwin();
                                exit(3);
                            }
                            i = NFROGPROJS;
                        }
                    }
                }
            case FROG:
                copySprite(frogPtr, spriteLetta);
                for (size_t i = 0; i < NPLANTS; i++)
                {
                    if (spriteIntersects2(*frogPtr, *(plants[i])))
                    {
                        if(plants[i]->type!=PLANTDEAD)
                        {
                            continua =0;
                            i = NPLANTS;
                        }
                    }
                }
                for (size_t i = 0; i < NPLANTPROJS; i++)
                {
                    if (spriteIntersects2(*frogPtr, *(projsPlants[i])))
                    {
                        continua =0;
                        i = NPLANTPROJS;

                    }
                }
                if (frogPtr->h == 4 && continua!=0)
                {
                    continua = 0;
                    for (int i = 0; i < game.nsprites; i++)
                    {

                        if (spriteIntersects2(*frogPtr, *(game.sprites[i])))
                        {
                            if (game.sprites[i]->type == CROC || game.sprites[i]->type == SIDEWALK || game.sprites[i]->type == GRASS|| game.sprites[i]->type == REDCROC)
                            {
                                continua = 1;
                                i = game.nsprites;
                            }
                            
                        }
                    }
                }
                for (size_t i = 0; i < game.nsprites; i++)
                {
                    if (spriteIntersects2(*frogPtr, *(game.sprites[i])))
                    {
                        if (game.sprites[i]->type == HOLE)
                        {
                            *punteggio+=1000+(2000*(larghezzaBarra))/(SWIDTH-11);
                            continua =2;
                            int tempx=game.sprites[i]->x;
                            int tempy=game.sprites[i]->y;
                            *game.sprites[i]=createHole(1);
                            game.sprites[i]->x=tempx;
                            game.sprites[i]->y=tempy;
                            i = game.nsprites;

                        }
                        else if (game.sprites[i]->type == HOLEFILLED)
                        {
                            continua=0;
                            i = game.nsprites;
                        }
                    }
                }
                break;
            case REDCROC:
            case REDCROCFELL:
            case CROC:

                copySprite(game.sprites[spriteLetta->id], spriteLetta);
                // sleep(1);
                int status = 0;
                int crocid = spriteLetta->id - startIdCrocs;

                // if(flussi[(game.sprites[spriteLetta->id]->y - (20)) / 4]>0)
                if (spriteLetta->x == NAPOLI )
                {
                    waitpid(pidCrocs[crocid], &status, 0);

                    flussoleggero = 0;
                    contFlussoMinore = 0;
                    sburacchio = 0;
                    
                    crocsaflusso[NFLOWS-1-(spriteLetta->y-16)/4]--;
                    
                    for (size_t i = 0; i < NFLOWS; i++)
                    {
                        // mvwprintw(scrtop.win,0,i,"%d",crocsaflusso[i]);
                        // wrefresh(scrtop.win);
                        if (crocsaflusso[i] < crocsaflusso[flussoleggero])
                        {
                            flussoleggero = i;
                            contFlussoMinore = 0;
                            arrayflussiminori[contFlussoMinore] = i;
                            contFlussoMinore++;
                        }
                        else if (crocsaflusso[i] == crocsaflusso[flussoleggero])
                        {
                            arrayflussiminori[contFlussoMinore] = i;
                            contFlussoMinore++;
                        }
                    }
                    
                    flussoscelto = arrayflussiminori[rand() % (contFlussoMinore)];

                    crocsaflusso[flussoscelto]++;
                    pidCrocs[crocid] = fork();
                    if (pidCrocs[crocid] < 0)
                    {
                        endwin();
                        exit(3);
                    }
                    else if (pidCrocs[crocid] == 0)
                    {
                        // nProcesses++;
                        int xscelto;
                        if(flussi[flussoscelto]>0)
                        {
                            xscelto=-17-(6*crocsaflusso[flussoscelto]);
                        }
                        else
                        {
                            xscelto=game.sizex+(6*(crocsaflusso[flussoscelto]));
                        }
                        croc(game, crocs[crocid]->id, flussi[flussoscelto], pipeMain, pipeMain, game.sizey - (ELEMENTHEIGHT * (flussoscelto + 2)), xscelto, CROC+(9*(rand()%PERCENTREDCROC==0)));
                        endwin();
                        exit(3);
                    }
                }
                break;
            case TIMEBAR:  
                larghezzaBarra=spriteLetta->w; 
                if (spriteLetta->x>=game.sizex)
                {
                    continua=0;
                }
                
                break;
            default:
                break;
            }

            end = clock();
            t2 += ((end - start));
            t++;
            free(spriteLetta);
        }

        //Gia commentato in versione thread
        if (t2 >= 1000 && contCrocs < NCROCS)
        {
            t2 = 0;
            flussoleggero = 0;
            contFlussoMinore = 0;
            for (size_t i = 0; i < NFLOWS; i++)
            {
                if (crocsaflusso[i] < crocsaflusso[flussoleggero])
                {
                    flussoleggero = i;
                    contFlussoMinore = 0;
                    arrayflussiminori[contFlussoMinore] = i;
                    contFlussoMinore++;
                }
                else if (crocsaflusso[i] == crocsaflusso[flussoleggero])
                {

                    arrayflussiminori[contFlussoMinore] = i;
                    contFlussoMinore++;
                }
            }
            flussoscelto = arrayflussiminori[rand() % (contFlussoMinore)];

            crocsaflusso[flussoscelto]++;

            crocs[contCrocs]->y = game.sizey - (ELEMENTHEIGHT * (flussoscelto + 2));

            crocs[contCrocs]->x = game.sizex;
            // screenAddSprite(&game, crocs[cont]);
            pidCrocs[contCrocs] = fork();
            nProcesses++;
            if (pidCrocs[contCrocs] < 0)
            {
                endwin();
                exit(3);
            }
            else if (pidCrocs[contCrocs] == 0)
            {
                int xscelto;
                if(flussi[flussoscelto]>0)
                {
                    xscelto=0-17-8*crocsaflusso[flussoscelto];
                }
                else
                {
                    xscelto=game.sizex+8*(crocsaflusso[flussoscelto]);
                }
                croc(game, crocsid[contCrocs], flussi[flussoscelto], pipeMain, pipeMain, game.sizey - (ELEMENTHEIGHT * (flussoscelto + 2)), xscelto, CROC+(9*(rand()%6==0)));

                exit(3);
            }

            contCrocs++;
        
        }
    }

    //Uccidi i processi 
    for (size_t i = 0; i < NCROCS; i++)
    {
        if (pidCrocs[i] != INACTIVEPROCESS)
        {
            kill(pidCrocs[i], SIGUSR1);
        }
    }

    for (size_t i = 0; i < NPLANTS; i++)
    {
        if (pidPlants[i] != INACTIVEPROCESS)
        {
            kill(pidPlants[i], SIGUSR1);
        }
    }

    for (size_t i = 0; i < NFROGPROJS; i++)
    {
        if (pidProjs[i] != INACTIVEPROCESS)
        {

            kill(pidProjs[i], SIGUSR1);
        }
    }

    for (size_t i = 0; i < NPLANTPROJS; i++)
    {
        if (pidProjPlant[i] != INACTIVEPROCESS)
        {

            kill(pidProjPlant[i], SIGUSR1);
        }
    }
    
    kill(pidPrint, SIGUSR1);
    kill(pidFrog, SIGUSR1);
    kill(pidTimeBar, SIGUSR1);
    return continua;
}

void croc(Screen scr, int id, int dir, int pipesprites[], int pipebool[], int y, int x, int type)
{

        // Tutte i vari colori e sprite dei vari coccodrilli che ci possono essere
        short colorisottosinistra[72]= {07, 07, 07, 07, 07, 17, 07, 07, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01,
                            07, 07, 07, 07, 07, 17, 07, 07, 26, 26, 26, 46, 26, 46, 26, 02 ,02, 01,
                            07, 07, 07, 07, 07, 07, 07, 07, 07, 02, 26, 02, 26, 02, 02, 02, 26, 26,
                            01, 46, 46, 46, 46, 46, 46, 46, 02, 26, 02, 02, 02, 02, 02, 02, 02, 26};

        short colorisottodestra[72]= {02, 02, 02, 02, 02, 12, 02, 02, 07, 07, 07, 17, 07, 07, 07, 07, 07, 07,
                            02, 02, 02, 02, 02, 12, 02, 02, 26, 27, 07, 07, 17, 07, 07, 07 ,07, 07,
                            07, 07, 07, 07, 07, 07, 07, 07, 07, 07,  07, 07, 07, 07, 07, 07 ,07, 07,
                            01, 46, 46, 46, 46, 46, 46, 46, 02, 26, 02, 02, 02, 02, 02, 02, 02, 26};
        wint_t* txtsinistra= L"█▙▄▄▄▀▙▖          "
                            " ▀▀▀▀███▀▄▀▄▀▄▀▄▄ "
                            "  ▄ ▄▀▄███▀█▀███▀▀"
                            " ▄█▀▀▀███▄███████▄";

        short sinistraVerde[72] = {05L, 05L, 05L, 05L, 05L, 15, 05L, 05L, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 15, 05L, 15, 05L, 05, 05, 05, 56, 56, 56, 46, 56, 46, 56, 05L, 05L, 1,
                            1, 1, 41, 1, 41, 41, 41, 05, 05, 05, 56, 05, 56, 05, 05, 05, 56, 56,
                            1, 46, 46, 46, 46, 46, 46, 46, 05, 56, 05, 05, 05, 05, 05, 05, 05, 56};

        short sinistraRosso[72] = {07, 07, 07, 07, 07, 17, 07, 07, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01,
                                01, 17, 07, 17, 07, 07, 07, 07, 76, 76, 76, 46, 76, 46, 76, 07 ,07, 01,
                                01, 01, 41, 01, 41, 41, 41, 07, 07, 07, 76, 07, 76, 07, 07, 07, 76, 76,
                                01, 46, 46, 46, 46, 46, 46, 46, 07, 76, 07, 07, 07, 07, 07, 07, 07, 76};

        short destraRosso[72] = {01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 07, 07, 17, 07, 07, 07, 07, 07,
                                    01, 07, 07, 76, 46, 76, 46, 76, 76, 76, 07, 07, 07, 07, 17, 07, 17, 1,
                                    76, 76, 07, 07, 07, 76, 07, 76, 07, 07, 07, 41, 41, 41, 01, 41, 01, 01,
                                    76, 07, 07, 07, 07, 07, 07, 07, 76, 07, 46, 46, 46, 46, 46, 46, 46, 1};

        short destraVerde[72] = {01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 05, 05, 15, 05, 05, 05, 05, 05,
                                01, 05, 05, 56, 46, 56, 46, 56, 56, 56, 05, 05, 05, 05, 15, 05, 15, 1,
                                56, 56, 05, 05, 05, 56, 05, 56, 05, 05, 05, 41, 41, 41, 01, 41, 01, 01,
                                56, 05, 05, 05, 05, 05, 05, 05, 56, 05, 46, 46, 46, 46, 46, 46, 46, 1};

        
        short gialloDestra[72] =   {01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 02, 02, 12, 02, 02, 02, 02, 02,
                                        01, 02, 02, 26, 46, 26, 46, 26, 26, 26, 02, 02, 02, 02, 12, 02, 12, 1,
                                        26, 26, 02, 02, 02, 26, 02, 26, 02, 02, 02, 41, 41, 41, 01, 41, 01, 01,
                                        26, 02, 02, 02, 02, 02, 02, 02, 26, 02, 46, 46, 46, 46, 46, 46, 46, 1};

        short gialloSinistra[72] ={02, 02, 02, 02, 02, 12, 02, 02, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01,
                                    01, 12, 02, 12, 02, 02, 02, 02, 26, 26, 26, 46, 26, 46, 26, 02 ,02, 01,
                                    01, 01, 41, 01, 41, 41, 41, 02, 02, 02, 26, 02, 26, 02, 02, 02, 26, 26,
                                    01, 46, 46, 46, 46, 46, 46, 46, 02, 26, 02, 02, 02, 02, 02, 02, 02, 26};
        wint_t* txtdestra=    L"          ▗▟▀▄▄▄▟█"
                                " ▄▄▀▄▀▄▀▄▀███▀▀▀▀ "
                                "▀▀███▀█▀███▄▀▄ ▄  "
                                "▄███████▄███▀▀▀█▄ ";

        wint_t * txtsottosinistra=   L"                  "
                        "█▙▄▄▄▀▙▖          "
                        " ▀▀▀▀▀▀▀▀         "
                        "                  ";

        wint_t * txtsottodestra=        L"                  "
                                "          ▗▟▀▄▄▄▟█"
                                "        ▀▀▀▀▀▀▀▀  "
                                "                  ";                         

    Sprite coccodrillo;
    coccodrillo.type = type;
    coccodrillo.id = id;
    coccodrillo.priority = 4;
    coccodrillo.w = 18;
    coccodrillo.h = 4;
    coccodrillo.y = y;

    coccodrillo.x = x;
    unsigned int frames = (FRAMETIME);
    frames = frames * (abs(dir));
    int dirx=0;
    short *giallo; short * colorisotto;
    short *clrfinalenormale, *clrosttofinale;
    wint_t * txtfinale,* txtsottofinale;
    // Determina qui quali sono le sprite e i colori da usare, in base a tipo e direzione
    if(dir>0)
    {
        colorisotto=colorisottodestra;
        txtfinale=txtdestra;
        txtsottofinale=txtsottodestra;

        giallo=gialloDestra;
        dirx=1;
        if(type==REDCROC)
        {
            clrfinalenormale=destraRosso;
        }
        else
        {
            clrfinalenormale = destraVerde;
        }
    } 
    else
    {
        colorisotto=colorisottosinistra;
        txtfinale = txtsinistra;
        txtsottofinale=txtsottosinistra;


        giallo=gialloSinistra;
        dirx=-1;
        if(type==REDCROC)
        {
            clrfinalenormale=sinistraRosso;
        }
        else
        {
            clrfinalenormale = sinistraVerde;
        }
    }
    
    // Logica coccodrilli
    int raggiuntoMuro=0;
    int animazioneVaGiu=0;
    int andatogiu=0;
    // Fino a quando non raggiungi muro
    while (!raggiuntoMuro)
    {
        // Imposta la sprite come quella determinata prima
        coccodrillo.txt=txtfinale;
        coccodrillo.type=type;
        coccodrillo.clr=clrfinalenormale;
        
        // Se il coccodrillo e' andato giu (rosso)
        if(andatogiu>0)
        {
            coccodrillo.type=REDCROCFELL;
            coccodrillo.clr=colorisotto;
            coccodrillo.txt=txtsottofinale;
            // Abbassa contatore dell animazione
            andatogiu--;
        }
        
        // Casi dei vari frame dell animazione dove sta per andare giu
        switch (animazioneVaGiu)
        {
        
        case 0:
            /* code */
            break;
        case 1:
            // Determina quanto deve stare giu
            andatogiu=SECONDIGIU*FPS;
            animazioneVaGiu--;
            break;
        default:
            // Un quarto dei frame cambia colore al coccodrillo giallo, per lampeggiare
            if(animazioneVaGiu%4)
            {
                coccodrillo.clr=clrfinalenormale;
            }
            else
            {
                coccodrillo.clr=giallo;
            }
            animazioneVaGiu--;
            break;
        }

        // Se sei un coccodrillo rosso che non e' andato giu ed e' sopra
        if(type==REDCROC && andatogiu==0 && animazioneVaGiu<=0)
        {
            // Se e' dentro lo schermo
            if(coccodrillo.x<scr.sizex&&coccodrillo.x>0)
            {
                // Determina casualmente se iniziare l'animazione per andare giu
                if(rand()%FPS*2==0)
                {   
                    // Inizia animazione in base agli fps e alla velocita'
                    animazioneVaGiu=(FPS*3)/abs(dir);
                }
            }

        }

        // Sposta coccodrillo
        coccodrillo.x+=dirx;
        sendSpriteToPipe(pipesprites, coccodrillo);
        usleep(frames);
        // Controlla se ha raggiunto la fine del muro opposto
        if (dir > 0)
        {
            if (coccodrillo.x >= scr.sizex -1)
            {
                raggiuntoMuro=1;
            }
        }
        else
        {
            // coccodrillo.x = scr.sizex;
            if (coccodrillo.x <= 0 - coccodrillo.w+1)
            {
                raggiuntoMuro=1;   
            }
        }
    }
    // Se ha raggiunto la fine del muro mandalo a napoli
    coccodrillo.x = NAPOLI;
    sendSpriteToPipe(pipesprites, coccodrillo);
    exit(0);
}

void frog(Screen scr, int *flussi, int nflussi, int pipesprites[], int pipebool[], int id)
{
    // sleep(1);
    int txtsottosinistra;
    txtsottosinistra = 0;
    Sprite frogPtr;
    frogPtr.type = FROG;
    frogPtr.id = id;
    frogPtr.x = 0;
    frogPtr.y = scr.sizey - 4;
    frogPtr.priority = 9;
    frogPtr.w = 6;
    frogPtr.h = 4;
    frogPtr.txt = L"▟▗▞▗▖▙"
                "▐▟▘▝▙▌"
                "▐▜▚█▛▌"
                "▜ ▀▀ ▛";
    short clrfinalenormale[30] = {03, 07, 23, 23, 07, 03,
                          03, 03, 23, 23, 03, 03,
                          05, 05, 23, 2, 05, 05,
                          05, 05, 05, 05, 05, 05};
    frogPtr.clr = clrfinalenormale;
    short arrayclr2[30] = {03, 07, 23, 23, 07, 03,
                           03, 03, 23, 23, 03, 03,
                           05, 05, 23, 2, 05, 05,
                           05, 05, 23, 2, 05, 05,
                           05, 05, 05, 05, 05, 05};
    int continua = 1;
    int frames = FRAMETIME;
    close(pipesprites[0]);
    int diry = 0, dirx = 0, movedY = 0, movedHorizontal = 0;
    int c;
    clock_t start, end;
    long t;
    // frogPtr.id = 0;
    while (1)
    {
        frogPtr.type = FROG;
        frames = FRAMETIME;
        start = clock();
        // Se non si sta muovendo, quindi non e' in alcun animazione
        if (movedY == 0 && movedHorizontal == 0)
        {
            // Prendi input
            c = (int)wgetch(scr.win);
            flushinp();
            switch (c)
            {
                // Dentro ogni case c'e' la direzione che prende la rana e il frame dell animazione dove che inizia
            case KEY_UP:
                if (!screenSpriteIntersectsWall(scr, frogPtr, TOPWALL))
                {
                    diry = -1;
                    dirx = 0;
                    movedY = 4;
                }
                break;
            case KEY_DOWN:
                if (!screenSpriteIntersectsWall(scr, frogPtr, BOTTOMWALL))
                {
                    movedY = 4;
                    diry = 1;
                    dirx = 0;
                }
                // frogPtr.type = FROGSPARANTE;
                break;
            case KEY_LEFT:
                if (!screenSpriteIntersectsWall(scr, frogPtr, LEFTWALL))
                {
                    diry = 0;
                    dirx = -1;
                    movedHorizontal = 3;
                }
                break;
            case KEY_RIGHT:
                if (!screenSpriteIntersectsWall(scr, frogPtr, RIGHTWALL))
                {
                    diry = 0;
                    dirx = 1;
                    movedHorizontal = 3;
                }
                break;
            case 32:
                frogPtr.type = FROGSPARANTE;
                break;
            case 'q':
                continua = 0;
                break;
            default:
                break;
            }
        }

        // Se e' all interno dell'azione dello spostamento orizzontale, esegui le azioni associate ad ogni frame
        switch (movedHorizontal)
        {
        case 0:

            // dirx = diry = 0;
            break;
        case 1:
            // moveSprite(scr, &frogPtr, dirx, diry);
            frames = FRAMETIME;
            moveSprite(scr, &frogPtr, dirx, 0);
            dirx = 0;
            movedHorizontal--;
            break;
        default:
            frames = FRAMETIME;
            moveSprite(scr, &frogPtr, dirx, 0);
            // moveSprite(scr, &frogPtr, dirx, diry);
            movedHorizontal--;
            break;
        }

        // Se e' all interno dell'azione dello spostamento verticale, esegui le azioni associate ad ogni frame
        switch (movedY)
        {
        case 4:
            frames = FRAMETIME;
            // moveSprite(scr, &frogPtr, dirx, diry);
            if (diry)
            {
                moveSprite(scr, &frogPtr, 0, diry);
                frogPtr.w = 6;
                frogPtr.h = 5;

                frogPtr.txt = L"▟▗▞▗▖▙"
                            "▐▟▘▝▙▌"
                            " ▐▐█▌ "
                            "▐▜▚█▛▌"
                            "▜ ▀▀ ▛";

                frogPtr.clr = arrayclr2;
            }

            movedY--;
            break;

        case 1:
            frames = FRAMETIME;
            frogPtr.w = 6;
            frogPtr.h = 4;
            frogPtr.txt = L"▟▗▞▗▖▙"
                        "▐▟▘▝▙▌"
                        "▐▜▚█▛▌"
                        "▜ ▀▀ ▛";

            frogPtr.clr = clrfinalenormale;
            moveSprite(scr, &frogPtr, 0, diry);
            movedY--;
        case 0:
            // frogPtr.h = 4;

            frames = FRAMETIME;
            diry = 0;
            break;
        default:
            // frog.x+=dirx;
            // frog.y+=diry;
            frames = FRAMETIME;
            moveSprite(scr, &frogPtr, 0, diry);
            // moveSprite(scr, &frogPtr, dirx, diry);
            movedY--;
            break;
        }
        if (txtsottosinistra)
        {
            if (movedY == 0)
            {
                // Determina su che flusso e' la rana, e spostala come fosse un coccodrillo
                int flussoattuale=nflussi-2-(frogPtr.y-20)/4;
                if (frogPtr.y <= scr.sizey - 8 && frogPtr.y >= (NGRASS) * ELEMENTHEIGHT)
                {

                    if (flussi[flussoattuale] > 0)
                    {
                        moveSprite(scr, &frogPtr, 1, 0);
                    }
                    else
                    {
                        moveSprite(scr, &frogPtr, -1, 0);
                    }
                    // Fai aspettare la rana la stessa sleep del coccodrillo, meno un frametime per dare spazio ad input
                    frames = (FRAMETIME * abs(flussi[flussoattuale])) - FRAMETIME;
                    txtsottosinistra = 0;
                }
                else
                {
                    frames = FRAMETIME;
                }
            }
        }
        else
        {
            txtsottosinistra = 1;
        }

        sendSpriteToPipe(pipesprites, frogPtr);

        end = clock();
        // t = (((end - start) * 1000000) / CLOCKS_PER_SEC);
        // usleep(FRAMETIME - t);
        usleep(frames);
        // usleep(100000);
    }
}

//Funzione che verra usata da un processo, questo processo e' responsabile della stampa delle sprites sulle diverse screen
void mainPrint(Screen game, Screen scrbot, int pipePrint[])
{
    wclear(game.win);
    screenRefreshSprites(game);
    close(pipePrint[1]); // 0 = Lettura, 1 = Scrittura
    int time = 0;
    Sprite *txtsottosinistra;
    int spriteSize;
    Sprite *spritePrelevata;
    Sprite *tempSprite;
    int culo = 0;
{
    // Sprite eldiablo;
    // eldiablo.x=eldiablo.y=0;
    // eldiablo.priority=4;
    // eldiablo.w=eldiablo.h=1;
    // eldiablo.txt= L"................";
    // short retard[16]= {00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00};
    // eldiablo.clr=retard;
    // // wclear(game.win);
    // for (size_t diocane = 0; diocane < 1; diocane++)
    // {
    //     for (size_t i = 0; i < game.sizey; i++)
    //     {
    //         for (size_t j = 0; j < game.sizex; j++)
    //         {

    //             eldiablo.x=j;
    //             eldiablo.y=i;
    //             usleep(1);
    //             printSprite(game,eldiablo);

    //         }
    //     }
    // }
}
    wclear(game.win);
    screenRefreshSprites(game);
    Sprite * timebar= allocateSprite();
    Sprite * timebar2= allocateSprite();
    // sleep(1);
    while (1)
    {
        // wclear(game.win);

        screenRefreshSprites(game);
        // wrefresh(game.win);
        culo = 0;
        while (culo < 1)
        {
            culo++;

            //Serie di istruzioni per leggere dalla pipe una sprite
            spritePrelevata = malloc(sizeof(Sprite));
            tempSprite = malloc(sizeof(Sprite));
            read(pipePrint[0], tempSprite, sizeof(Sprite));
            spriteSize = tempSprite->w * tempSprite->h;
            tempSprite->txt = malloc(sizeof(wint_t) * spriteSize);
            tempSprite->clr = malloc(sizeof(short) * spriteSize);
            read(pipePrint[0], tempSprite->txt, (sizeof(wint_t) * spriteSize));
            read(pipePrint[0], tempSprite->clr, (sizeof(short) * spriteSize));
            *spritePrelevata = *tempSprite;
            spritePrelevata->clr = malloc((sizeof(short) * spriteSize));
            spritePrelevata->txt = malloc((sizeof(wint_t) * spriteSize));
            for (int i = 0; i < spriteSize; i++)
            {
                spritePrelevata->clr[i] = tempSprite->clr[i];
                spritePrelevata->txt[i] = tempSprite->txt[i];
            }
            if(spritePrelevata->type!=TIMEBAR)
            {
                free(game.sprites[spritePrelevata->id]->clr);
                free(game.sprites[spritePrelevata->id]->txt);
                game.sprites[spritePrelevata->id] = spritePrelevata;
            }
            else
            {
                if(scrbot.sprites[0]->type!=TIMEBAR)
                {
                    // screenAddSprite(&scrbot,timebar);
                }
                timebar=spritePrelevata;
                *timebar2=*timebar;
                timebar2->y=1;
                werase(scrbot.win);
                //wbkgd(scrbot.win,scrbot.background);
                printSprite(scrbot,*timebar);
                printSprite(scrbot,*timebar2);
                for (size_t i = 0; i < NVITE; i++)
                {
                    printSprite(scrbot,*scrbot.sprites[i]);
                }
                
                // screenRefreshSprites(scrbot);
                wrefresh(scrbot.win);
            }
            //Libero le varibili temporanee che sono state allocate manualmente
            free(tempSprite->clr);
            free(tempSprite->txt);
            free(tempSprite);
        }
    }
}


void projectile(Screen scr, int id, int pipesprites[], int y, int x, int diry)
{
    // sleep(1);
    // endwin();
    // delwin(scr.win);
    // free(scr.win);
    Sprite proiettile= createProj();
    proiettile.id=id;
    proiettile.x=x;
    proiettile.y=y;
    // if(coccodrillo.y<=scr.sizey-8)s

    // Se il proiettile e' sparato verso giu allora e' delle piante
    if(diry>0)
    {
        
        proiettile.type = PROJPLANT;

        proiettile.txt = L"▟▙▜▛";
        proiettile.w = 2;
        proiettile.h = 2;
        short arrayclr4[4] = {03,03,03,03};
        proiettile.clr = arrayclr4;
    }
    unsigned int frames = (FRAMETIME);
    while (1)
    {
        // sleep(1);
        moveSprite(scr, &proiettile, 0, diry);
        sendSpriteToPipe(pipesprites, proiettile);
        usleep(frames);
    }
}

void plant(Screen scr, int id, int pipesprites[], int x,int type)
{
    // sleep(1);
    // endwin();
    // delwin(scr.win);
    // free(scr.win);
    Sprite planta =createPlant((type==PLANT));
    
    unsigned int frames = (FRAMETIME);
    // if(coccodrillo.y<=scr.sizey-8)s

    // Se il tipo passato alla fuznione e' la pianta morta
    if(type==PLANTDEAD)
    {
        planta.x=x;
        planta.type=PLANTDEAD;
        planta.id=id;
        planta.y=0;
        sendSpriteToPipe(pipesprites, planta);
        
        // Determina per quanto tempo deve stare morta, in secondi
        int duratamorte = 3 + rand() % 5;
        while (duratamorte>=0)
        {
            for (size_t i = 0; i < FPS; i++)
            {
                usleep(FRAMETIME);
            }
            duratamorte--;
        }
        
    }
    // Una volta che e' viva (o se non e' stata generata viva)
    // Crea pianta viva
    planta =createPlant(1);


    planta.x=x;
    planta.id=id;
    planta.y=0;
    sendSpriteToPipe(pipesprites, planta);
    while (1)
    {
        // sleep(1);

        // Possibilta' casuale ad ogni frame che spari
        if(rand()%((FPS)*((rand()%4)+10))==0)
        {
            // Cambia il tipo a pianta che spara
            planta.type=PLANTSHOT;

            //Invio alla pipe
            sendSpriteToPipe(pipesprites, planta);

            // Ristabilisci tipo
            planta.type=PLANT;
        }
        
        usleep(frames);
    }
}

void timeBar2(int pipeMain[], int secondi)
{

    Sprite barraTempo;
    barraTempo.x = 11;
    barraTempo.id = 0;
    barraTempo.y = 0;
    barraTempo.priority = 9;
    barraTempo.type = TIMEBAR;
    barraTempo.h = 1;
    barraTempo.w = SWIDTH-11;
    barraTempo.txt = L"██████████████████████████████████████████████████████████████████";
    
    short arrayclr_barra[SWIDTH];
    for (int i = 0; i < SWIDTH; i++)
    {
        arrayclr_barra[i] = 03;
    }
    barraTempo.clr = arrayclr_barra;

    
    int cicli = 0;
    // Durata della sleep per cambiare di un pixel la barra
    int frames= ((secondi*1000000)/((SWIDTH-11)*2));
    while (barraTempo.x >= 0)
    {
        
        
        usleep(frames);
        
        modificaTempo(&barraTempo,&cicli,SWIDTH-11);
        cicli++;
        sendSpriteToPipe(pipeMain,barraTempo);
    }
    exit(0);
}


int modificaTempo(Sprite *barra, int *time, int width)
{

        
    int color;
    int delta = 1;
    bool change = false;

    // Meta' delle volte cambia la sprite, l'altra meta' la sposta e ripristina sprite
    if (*time % 2 == 1)
    {
        barra->txt = L"▐█████████████████████████████████████████████████████████████████";
        delta = 0;
    }
    else
    {
        barra->txt = L"██████████████████████████████████████████████████████████████████";
    }

    barra->x += delta;

    // In base al progresso della barra, essa puo assumere colori diversi, giallo per la 1/3 e rosso per 2/3

    if (*time == ((width*2)/3)*2 || *time == ((width*2)/3))
    {
        if (*time == ((width*2)/3))
        {
            color = 02;
        }
        else
        {
            color = 07;
        }

        for (int i = 0; i < width*2; i++)
        {
            barra->clr[i] = color;
            // barra2->clr[i] = color;
        }
    }
}