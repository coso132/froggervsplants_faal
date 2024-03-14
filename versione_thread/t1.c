#include "screen.h"
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

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
#define FORGPAUSE 16

//      MISC

#define DIM_BUFFER 30
#define NVITE 3                           // Numero di vite
#define SWIDTH 66                         // Larghezza dello schermo di gioco
#define SHEIGHT 56                        // Altezza dello schermo di gioco
#define NFLOWS 9                          // Numero di flussi nel gioco
#define ELEMENTHEIGHT 4                   // Altezza massima di un singolo elemento di gioco statico
#define NOFSPEEDS 3                       // Numero di velocita' possibili assumibili dai coccodrilli
#define MAXNOFCHARS SWIDTH *ELEMENTHEIGHT // Grandezza massima di una sprite
#define NGRASS 4                          // Numero singole sprite erba che compongono l'argine superiore
#define DURATAMANCHE 90                   // Durata di una manche in secondi
#define NCROCS 15                         // Numero massimo di coccodrilli nel gioco
#define NPLANTS 5                         // Numero massimo di piante nel gioco
#define NHOLES 5                          // Numero massimo di buche nel gioco
#define NFROGPROJS 3                      // Numero di proiettili massimi della rana
#define NPLANTPROJS 12                    // Numero di proiettili massimi delle piante
#define NAPOLI -200                       // Luogo molto strano e lontano...
#define NAPOLIPROJFROG -20                // Luogo molto strano e lontano... per i proiettili delle rane
#define NAPOLIPROJPLANT -40               // Luogo molto strano e lontano... per i proiettili delle piante
#define PERCENTREDCROC 12                 // 1/x coccodrilli saranno creati rossi
#define SECONDIGIU 1                      // Quanto tempo sara' sott'acqua un coccodrillo

// Thread arguments del thread della rana
typedef struct frogArgs
{
    Screen scr;    // Schermo dove opera la rana
    int nflussi;   // Numero di flussi
    int id;        // Id della rana
    int *flussi;   // Puntatore(Array) che contieni le direzioni e velocita' di ogni flusso
} FrogArgs;
void *frog(void *);

// Thread arguments del thread delle piante
typedef struct plantArgs
{
    Screen scr; // Schermo
    int id;
    int x;
    int type; // Tipo che prendera' la pianta, viva o morta
} PlantArgs;
void *plant(void *);

// Thread arguments del thread dei coccodrilli
typedef struct crocArgs
{
    Screen scr;
    int id;
    int dir; // Direzione (e velocita') del coccodrillo, positivo = destra, alto = lento
    int y;
    int x;
    int type; // Tipo del coccodrillo, verde o rosso
} CrocArgs;
void *croc(void *);

// Thread arguments del thread dei proiettili
typedef struct projArgs
{
    Screen scr;
    int id;
    int y;
    int x;
    int diry; // Direzione nella y del proiettile, 1 o -1
} ProjArgs;
void *projectile(void *);

// Thread arguments del thread della barra del tempo
typedef struct timeBarArgs
{
    int secondi; // Durata in secondi
} TimeBarArgs;
void *timeBar(void *);

// Funzioni creazione sprite piu usate
Sprite createCroc()
{
    Sprite coccodrillo;
    coccodrillo.type = CROC;
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
    coccodrillo.clr = malloc(sizeof(short) * 72);
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
    if (type == 0)
    {

        plantSprite.type = PLANTDEAD;
    }
    plantSprite.x = 0;
    plantSprite.w = 6;
    plantSprite.h = 8;
    plantSprite.y = 0;
    plantSprite.priority = 6;
    plantSprite.txt = L"  ▟▙  "
                      " ▄▀▀▄ "
                      "▟▖▘█▗▙"
                      "▌▚▀▀▞▐"
                      "▙▌▟▙▐▟"
                      " ▌▜▛▐ "
                      "▛▞▄▄▚▟"
                      "▜████▛";

    if (type == 0)
    {
        plantSprite.txt = L"      "
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

    plantSprite.clr = malloc(sizeof(short) * 48);
    for (size_t i = 0; i < 48; i++)
    {
        if (type)
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
    // frog1.active=1;
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

    frogSprite.clr = malloc(sizeof(short) * 30);
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

    short arrayclrpro[6] = {06, 06, 06, 06, 06, 06};

    proiettile.clr = malloc(sizeof(short) * 6);
    for (size_t i = 0; i < 6; i++)
    {
        proiettile.clr[i] = arrayclrpro[i];
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

    short arrayclrpro[4] = {02, 02, 02, 06};

    coin.clr = malloc(sizeof(short) * 4);
    for (size_t i = 0; i < 4; i++)
    {
        coin.clr[i] = arrayclrpro[i];
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
    short *colorifinale;
    short arrayclrpro[24] = {56, 66, 66, 66, 66, 56,
                             66, 66, 66, 66, 66, 66,
                             66, 66, 66, 66, 66, 66,
                             56, 66, 66, 66, 66, 56};
    colorifinale = arrayclrpro;
    if (filled)
    {

        hole.txt = L"▟▗▞▗▖▙"
                   "▐▟▘▝▙▌"
                   "▐▜▚█▛▌"
                   "▜ ▀▀ ▛";
        // DARKG,5,YELLOW,2,GREEN,3,BROWN,6;
        short arrayclrpro2[24] = {63, 67, 23, 23, 67, 63,
                                  63, 23, 23, 23, 23, 63,
                                  65, 65, 23, 62, 65, 65,
                                  65, 65, 65, 65, 65, 65};
        colorifinale = arrayclrpro2;

        hole.type = HOLEFILLED;
    }

    hole.clr = malloc(sizeof(short) * 24);
    for (size_t i = 0; i < 24; i++)
    {
        hole.clr[i] = colorifinale[i];
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

int controllo(Screen game, Screen scrbot, Screen scrtop, int *punteggio);

int main()
{
    // Crea seed random
    srand(time(NULL));
    // Serve per inizializzare lo unicode?
    setlocale(LC_ALL, "");
    // Impostazioni stdscr
    initializeScreenFunctions();

    // Check grandezza shcermo
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
    curs_set(0);
    //
    // INIZIALIZZAZIONE DELLE SPRITE DI BACKGROUND

    //      FLUSSI DEL FIUME
    Sprite flussi[NFLOWS];
    for (size_t i = 0; i < NFLOWS; i++)
    {
        flussi[i] = createFlow(i % 2);
        flussi[i].y = game.sizey - (ELEMENTHEIGHT * (i + 2));
        screenAddSprite(&game, &flussi[i]);
    }

    //      MARCIAPIEDE IN BASSO
    Sprite marciapiede = createSidewalk();
    marciapiede.y = game.sizey - ELEMENTHEIGHT;
    screenAddSprite(&game, &marciapiede);

    //      ERBA IN ALTO
    Sprite erba[NGRASS];
    for (size_t i = 0; i < NGRASS; i++)
    {
        erba[i] = createGrass(i == NGRASS - 1);
        erba[i].y = ELEMENTHEIGHT * (i);
        screenAddSprite(&game, &erba[i]);
    }

    //      MONETE (VITA)
    Sprite coins[NVITE];
    wclear(scrbot.win);
    for (size_t i = 0; i < NVITE; i++)
    {
        coins[i] = createCoin();
        coins[i].x = 1 + ((1 + coins[i].w) * i);

        screenAddSprite(&scrbot, &coins[i]);
        printSprite(scrbot, coins[i]);
    }
    wrefresh(scrbot.win);

    //      TANE
    int holesId[NHOLES];
    Sprite holes[NHOLES];
    int startIdHoles = game.nsprites;
    for (size_t i = 0; i < NHOLES; i++)
    {
        holes[i] = createHole(0);
        holes[i].x = (6 + (i * 2 * 6));
        holesId[i] = game.nsprites;
        screenAddSprite(&game, &holes[i]);
    }

    //
    // FINE INIZIALIZZAZIONE DELLE SPRITE DI BACKGROUND

    screenRefreshSprites(game);
    int nsprites = game.nsprites; // Mi salvo il numero di sprite presenti all inizio del gioco, uguale al numero di sprite di background
    int giocare = 1;              // Se il gioco e' attivo o meno
    int vite;                     // Numero di vite
    while (giocare)
    {
        // Crea le tane
        for (size_t i = 0; i < NHOLES; i++)
        {
            holes[i] = createHole(0);
            holes[i].x = (6 + (i * 2 * 6)); // Formula per la posizione delle tane
        }
        vite = NVITE; // Imposto le vite
        // Imposto la sprite delle vite
        for (size_t i = 0; i < NVITE; i++)
        {
            scrbot.sprites[i]->txt = L"▟▙"
                                     "▜▛";
        }
        int punteggio = 0; // Inizializzo il punteggio a 0
        while (vite > 0)   // Fino a quando la rana e' in vita
        {
            // Stampa il punteggio
            wclear(scrtop.win);
            mvwprintw(scrtop.win, 0, 0, "PUNTEGGIO ATTUALE : %d", punteggio);
            wrefresh(scrtop.win);

            flushinp();               // Pulisco l'input di ncurses per evitare input non intesi dal giocatore
            game.nsprites = nsprites; // Resetto il numore di sprite all interno della schermata game

            // Tratto il controllo come un controllo booleano, se da 0 vuol dire che il giocatore ha persouna vita
            if (controllo(game, scrbot, scrtop, &punteggio) == 0)
            {
                vite--;
                scrbot.sprites[vite]->txt = L"    ";
                // sleep(1);
            }
            else // Senno ha vinto la manche
            {
                size_t i;
                // Vedi se ci sono tane vuote
                for (i = 0; i < NHOLES; i++)
                {
                    if (holes[i].type == HOLE)
                    {
                        // Se ci sono tane vuote allore fai un break mettendi i a NHOLES + 1, ripeti il ciclo
                        i = NHOLES + 1;
                    }
                }
                // Se i ha raggiunto il numero massimo di tane senza fare il break, vuol dire che tutte le tane sono chiuse, la partita e' vinta
                if (i == NHOLES)
                {
                    // Vite a -1 e' codice per aver vinto la partita, 0 per aver perso
                    vite = -1;
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

    // Uscita, il giocatore ha scelto di uscire dal gioco
    endwin();

    exit(3);
}

/*
    Mutex utilizzato per assicurare l'atomicita' delle operazioni con ncurses,
    verra effettuato il lock all inizio del ciclo di ogni thread concorrente e
    l'unlock appena finisce, quando e' presente lo sleep di sincronizzazione.

    Nel thread di controllo invece il refresh delle sprite nello schermo verra'
    contenuto da il lock e unlock di questo mutex, per fare in modo che le operazioni
    di ncurses (che non supporta il multithreading) non verranno interrote dallo scheduler SW
    creando vari problemi e glitch grafici
*/
pthread_mutex_t mutexNCurses = PTHREAD_MUTEX_INITIALIZER;

/// Buffer e variabili globali a tutti i thread
int contBuffer = 0; //[MAXSPRITES];
int scriviCont = 0;
int leggiCont = 0;
pthread_mutex_t spriteMutex = PTHREAD_MUTEX_INITIALIZER;
Sprite *spriteBuffer[DIM_BUFFER];

int controllo(Screen game, Screen scrbot, Screen scrtop, int *punteggio)
{
    for (size_t i = 0; i < DIM_BUFFER; i++)
    {
        spriteBuffer[i] = allocateSprite();
    }

    contBuffer = 0;
    scriviCont = 0;
    leggiCont = 0;
    // Variabili varie utilizzate per la logica di gioco
    int flussi[NFLOWS], direzioni[NFLOWS]; // Array di interi che rappresenta la velocita' e direzione di ogni flusso, valore negativo==sinistra e viceversa, valore assoluto alto==velocita' bassa e viceversa
    int velocita;                          // Variabile di appoggio che rappresenta la velocita' in valore assoluto di un determinato flusso alla sua creazione
    int flussoscelto;                      // Rappresenta l'indice del flusso che e' stato scelto dall'algoritmo che associa un coccodrillo a un flusso
    int flussoleggero = 0;                 // Flusso con numero minore di coccodrilli
    int arrayflussiminori[NFLOWS];         // Array che contiene gli indici di tutti i flussi che contendono la posizione come flusso con il minor numero di coccodrilli
    int contFlussoMinore = 0;              // Numero di flussi che condividono il minor numero di coccodrilli
    int crocsaflusso[NFLOWS];              // Array di interi che contengono il numero di coccodrilli attualmente presenti nel flusso corrispondente
    int contCrocs = 0;                     // Coccodrilli attivi sullo schermo al momento
    int continua = 1;                      // Determina se continuare il gioco o no (morte rana o vincita manche o fine tempo)
    clock_t start, end, t, t2 = 0;         // Variabili utilizzate per il conteggio di cicli di clock di alcune parti del codice, utile per sincronizzazione

    //      Generazione casuale dei flussi
    for (size_t i = 0; i < NFLOWS; i++)
    {
        // Imposta casualmente la direzione
        if (rand() % 2)
            direzioni[i] = 1;
        else
            direzioni[i] = -1;

        if (i > 1 && direzioni[i] == direzioni[i - 1] && direzioni[i] == direzioni[i - 2])
        {
            direzioni[i] = -1 * direzioni[i];
        }

        velocita = (rand() % NOFSPEEDS) + 1;   // Imposta casualmente la velocita', da 1 a NOFSPEEDS
        flussi[i] = direzioni[i] * (velocita); // Moltiplica la direzione per la velocita, assegnala al flusso corrispondente
        crocsaflusso[i] = 0;                   // Imposta coccodrilli presenti in quel flusso a 0
    }
    int activeThreads[MAXSPRITES];
    for (size_t i = 0; i < MAXSPRITES; i++)
    {
        activeThreads[i] = 0;
    }
    //      BARRA DEL TEMPO
    //

    Sprite *barratempo = allocateSprite();
    pthread_t barraTempoThread;

    TimeBarArgs timebarargs;

    // timebarargs.buffer = spriteBuffer;
    timebarargs.secondi = DURATAMANCHE;

    pthread_create(&barraTempoThread, NULL, &timeBar, &timebarargs);

    //
    //      RANA
    //
    Sprite *frog1 = allocateSprite();
    screenAddSprite(&game, frog1);
    pthread_t frogThread;

    FrogArgs frogargs;
    frogargs.flussi = flussi;
    // frogargs.frog = spriteBuffer;
    frogargs.id = frog1->id;
    frogargs.nflussi = NFLOWS;
    frogargs.scr = game;

    pthread_create(&frogThread, NULL, &frog, &frogargs);
    //
    //      PIANTE
    //

    pthread_t plantThreads[NPLANTS];
    PlantArgs plantargs[NPLANTS];

    int startIdPlants = game.nsprites; // Primo id delle piante nell array di sprite all interno di screen
    Sprite **plants = malloc((NPLANTS) * sizeof(Sprite *));
    // Inizializzo ogni piante
    for (size_t i = 0; i < NPLANTS; i++)
    {
        plants[i] = allocateSprite();
        screenAddSprite(&game, plants[i]);
        plantargs[i].id = plants[i]->id;
        plantargs[i].scr = game;
        plantargs[i].x = 6 + (i * 2 * 6);
        plantargs[i].type = PLANT;

        pthread_create(&(plantThreads[i]), NULL, &plant, &(plantargs[i]));
    }

    //
    //      COCCODRILLI
    //

    pthread_t crocThreads[NCROCS];
    CrocArgs crocargs[NCROCS];
    int startIdCrocs = game.nsprites;
    Sprite **crocs = malloc((NCROCS) * sizeof(Sprite *));

    for (size_t i = 0; i < NCROCS; i++)
    {
        crocs[i] = allocateSprite();
        screenAddSprite(&game, crocs[i]);
    }

    //
    //      PROIETTILI RANA
    //

    pthread_t frogProjsThreads[NFROGPROJS];
    ProjArgs argsfrogproj[NFROGPROJS];
    int startIdProjFrogs = game.nsprites;
    Sprite **frogProjs = malloc((NFROGPROJS) * sizeof(Sprite *));

    for (size_t i = 0; i < NFROGPROJS; i++)
    {
        frogProjs[i] = allocateSprite();
        screenAddSprite(&game, frogProjs[i]);
    }

    //
    //      PROIETTILI PIANTE
    //

    pthread_t plantProjsThreads[NPLANTPROJS];
    ProjArgs argsplantproj[NPLANTPROJS];
    int startIdProjplant = game.nsprites;
    Sprite **plantProjs = malloc((NPLANTPROJS) * sizeof(Sprite *));

    for (size_t i = 0; i < NPLANTPROJS; i++)
    {
        plantProjs[i] = allocateSprite();
        screenAddSprite(&game, plantProjs[i]);
    }

    //      ULTIME VARIABILI

    Sprite *spriteLetta = allocateSprite();
    int bomboclaat = 1;   // e' un mistero
    int projid;           // Variabile che verra usata nello switch per i proiettili
    int scadutotempo = 0; // Se il tempo e' scaduto
    int larghezzaBarra;   // Larghezza della barra del tempo(di conseguenza il tempo passato)
    int nprojs = 0;       // Numero proiettili della rana attivo

    //      INIZIO LOOP DI GIOCO
    while (continua == 1)
    {
        // Inizia conto clock, serve per sapere quando spawnare i coccodrilli (outdated, meglio usare i frame per questa versione)
        start = clock();

        while (contBuffer > 0 && continua)
        {

            pthread_mutex_lock(&spriteMutex);
            if (contBuffer > 0)
            {
                // read = 1;
                copySprite(spriteLetta, spriteBuffer[leggiCont]);
                leggiCont = (leggiCont + 1) % DIM_BUFFER;

                contBuffer--;
            }
            pthread_mutex_unlock(&spriteMutex);

            // Gestisci questa sprite in base al suo tipo

            switch (spriteLetta->type)
            {
            case FORGPAUSE:
            {
                pthread_mutex_lock(&mutexNCurses);
                // screenRefreshSprites(game);
                wattr_on(game.win, COLOR_PAIR(01), NULL);
                // wclear(game.win);
                mvwprintw(game.win, SHEIGHT / 2, SWIDTH / 2 - 20, "PREMI q (minuscola) PER TOGLIERE LA PAUSA");
                wrefresh(game.win);
                while (wgetch(game.win) != 'q')
                {
                    usleep(FRAMETIME);
                }
                wclear(game.win);
                wrefresh(game.win);
                pthread_mutex_unlock(&mutexNCurses);
            }

            break;
            case PLANTSHOT:
                for (size_t i = 0; i < NPLANTPROJS; i++)
                {
                    // Se il thread relativo non e' attivo
                    if (activeThreads[plantProjs[i]->id] == 0)
                    {
                        // Genera nuovo proiettile
                        // argsplantproj[i].buffer = spriteBuffer;

                        argsplantproj[i].id = plantProjs[i]->id;

                        // argsplantproj[i].mutex = &spriteMutex;

                        argsplantproj[i].scr = game;

                        argsplantproj[i].x = (spriteLetta->x) + (spriteLetta->w / 2) - 1;

                        argsplantproj[i].y = spriteLetta->y + 3;

                        argsplantproj[i].diry = 1;

                        activeThreads[plantProjs[i]->id] = 1;

                        plantProjs[i]->x = argsplantproj[i].x;
                        plantProjs[i]->y = argsplantproj[i].y;

                        pthread_create(&(plantProjsThreads[i]), NULL, &projectile, &(argsplantproj[i]));
                        // break
                        i = NPLANTPROJS;
                    }
                }

            case PLANTDEAD:
            case PLANT:
                // Copia sprite letta alla sua posizione nella lista di sprite dello schermo, cosi da poter essere visualizzata
                copySprite(game.sprites[spriteLetta->id], spriteLetta);
                break;

            case PROJPLANT:
                // Caso proiettile della pianta
                copySprite(game.sprites[spriteLetta->id], spriteLetta);
                projid = spriteLetta->id - startIdProjplant;

                // Se tocca il muro in basso
                if (screenSpriteIntersectsWall(game, *plantProjs[projid], BOTTOMWALL))
                {
                    // Uccidi il thread
                    pthread_cancel(plantProjsThreads[projid]);
                    // Mandalo a napoli, cosi che non verra' stampato
                    plantProjs[projid]->x = NAPOLIPROJPLANT;
                    plantProjs[projid]->y = NAPOLIPROJPLANT;
                    // Imposta il mutex come non attivo
                    activeThreads[plantProjs[projid]->id] = 0;
                }
                else
                {
                    // Se tocca la rana uccidila
                    if (spriteIntersects2(*frog1, *spriteLetta))
                    {
                        continua = 0;
                    }
                    else
                    {
                        // Per ogni proiettile della rana
                        for (size_t i = 0; i < NFROGPROJS; i++)
                        {
                            // Se tocca un proiettile della rana
                            if (spriteIntersects2(*frogProjs[i], *plantProjs[projid]))
                            {
                                // Aumenta punteggio partita
                                *punteggio += 10;
                                // Se i thread sono attivi, uccidili entrambi
                                if (activeThreads[plantProjs[projid]->id] == 1)
                                {
                                    pthread_cancel(plantProjsThreads[projid]);

                                    plantProjs[projid]->x = NAPOLIPROJPLANT;
                                    plantProjs[projid]->y = NAPOLIPROJPLANT;

                                    activeThreads[plantProjs[projid]->id] = 0;
                                }

                                if (activeThreads[frogProjs[i]->id] == 1)
                                {
                                    pthread_cancel(frogProjsThreads[i]);

                                    frogProjs[i]->x = NAPOLIPROJFROG;
                                    frogProjs[i]->y = NAPOLIPROJFROG;

                                    activeThreads[frogProjs[i]->id] = 0;
                                }
                                // Diminiuisci numero proiettili rana attivi
                                nprojs--;
                                // Break
                                i = NFROGPROJS;
                            }
                        }
                    }
                }
                break;
            case PROJ:
                // Caso proiettile rana
                copySprite(game.sprites[spriteLetta->id], spriteLetta);
                projid = spriteLetta->id - startIdProjFrogs;
                // Se tocca il muro superiore
                if (screenSpriteIntersectsWall(game, *frogProjs[projid], TOPWALL))
                {
                    // Uccidi proiettile
                    pthread_cancel(frogProjsThreads[projid]);

                    game.sprites[frogProjs[projid]->id]->x = NAPOLIPROJFROG;
                    game.sprites[frogProjs[projid]->id]->y = NAPOLIPROJFROG;

                    activeThreads[frogProjs[projid]->id] = 0;
                    nprojs--;
                }
                else
                {
                    // Per ogni planta
                    for (size_t i = 0; i < NPLANTS; i++)
                    {
                        // Se interseca una pianta non morta
                        if (spriteIntersects2(*frogProjs[projid], *plants[i]) && plants[i]->type != PLANTDEAD)
                        {
                            // Ed e' attiva
                            if (activeThreads[frogProjs[projid]->id] == 1)
                            {
                                // Uccidi proiettile a pianta
                                pthread_cancel(frogProjsThreads[projid]);

                                game.sprites[frogProjs[projid]->id]->x = NAPOLIPROJFROG;
                                game.sprites[frogProjs[projid]->id]->y = NAPOLIPROJFROG;

                                activeThreads[frogProjs[projid]->id] = 0;
                                nprojs--;

                                *punteggio += 100; // Aumenta punteggio
                                pthread_cancel(plantThreads[i]);

                                // Rigenera pianta come morta
                                // plantargs[i].buffer = spriteBuffer;
                                plantargs[i].id = i + startIdPlants;
                                plantargs[i].scr = game;
                                plantargs[i].x = plants[i]->x;
                                plantargs[i].type = PLANTDEAD;
                                pthread_create(&(plantThreads[i]), NULL, &plant, &(plantargs[i]));
                                i = NPLANTS;
                            }
                        }
                    }
                    // Per ogni coccodrillo
                    for (size_t i = 0; i < NCROCS; i++)
                    {
                        // Se interseca un coccodrillo non sott'acqua
                        if (spriteIntersects2(*frogProjs[projid], *crocs[i]) && crocs[i]->type != REDCROCFELL)
                        {
                            // Che e' attivo
                            if (activeThreads[frogProjs[projid]->id] == 1)
                            {
                                // Uccidi proiettile
                                pthread_cancel(frogProjsThreads[projid]);

                                game.sprites[frogProjs[projid]->id]->x = NAPOLIPROJFROG;
                                game.sprites[frogProjs[projid]->id]->y = NAPOLIPROJFROG;

                                activeThreads[frogProjs[projid]->id] = 0;
                                nprojs--;
                                // Se il coccodrillo che tocca e' rosso
                                if (crocs[i]->type == REDCROC && activeThreads[crocs[i]->id] == 1)
                                {
                                    // Termina coccodrillo rosso e ricrealo come verse
                                    *punteggio += 250;
                                    pthread_cancel(crocThreads[i]);

                                    // crocargs[i].buffer = spriteBuffer;
                                    crocargs[i].id = crocs[i]->id;
                                    crocargs[i].scr = game;
                                    crocargs[i].type = CROC;
                                    crocargs[i].dir = flussi[NFLOWS - 2 - (crocs[i]->y - 20) / 4];
                                    crocargs[i].x = crocs[i]->x;
                                    crocargs[i].y = crocs[i]->y;

                                    pthread_create(&(crocThreads[i]), NULL, &croc, &(crocargs[i]));
                                }
                                i = NCROCS; // break
                            }
                        }
                    }
                }

                break;

            case FROGSPARANTE:
                // Caso rana che spara
                copySprite(frog1, spriteLetta);
                bomboclaat = 0; // se tolgo non va nulla, ho paura

                // Se ci sono proiettili in memoria liberi
                if (nprojs < NFROGPROJS)
                {
                    // Spara
                    nprojs++;
                    // Cerca tra i proiettili della rana
                    for (size_t i = 0; i < NFROGPROJS; i++)
                    {
                        // Un proiettile libero
                        if (activeThreads[frogProjs[i]->id] == 0)
                        {
                            // Genera proiettile
                            // argsfrogproj[i].buffer = spriteBuffer;
                            argsfrogproj[i].id = frogProjs[i]->id;
                            argsfrogproj[i].scr = game;
                            argsfrogproj[i].x = frog1->x + (frog1->w / 2) - (i % 2) - 1;
                            argsfrogproj[i].y = frog1->y - 1;
                            argsfrogproj[i].diry = -1;

                            activeThreads[frogProjs[i]->id] = 1;
                            frogProjs[i]->x = frogProjs[i]->y = -2;
                            pthread_create(&frogProjsThreads[i], NULL, &projectile, &argsfrogproj[i]);
                            usleep(1000); // Senza questa sleep. non funziona nulla. niente.
                            i = NFROGPROJS;
                        }
                    }
                }

            case FROG:
                // Caso rana
                // Per evitare di fare la copysprite due volte?
                if (bomboclaat)
                {
                    copySprite(frog1, spriteLetta);
                }
                bomboclaat = 1;
                // Per ogni planta
                for (size_t i = 0; i < NPLANTS; i++)
                {
                    // Se la tocchi
                    if (spriteIntersects2(*frog1, *(plants[i])))
                    {
                        // E non e' morta
                        if (plants[i]->type != PLANTDEAD)
                        {
                            // Muori
                            continua = 0;
                            i = NPLANTS;
                        }
                    }
                }
                // Per ogni proiettile della pianta, se lo tochci muori
                for (size_t i = 0; i < NPLANTPROJS; i++)
                {
                    if (spriteIntersects2(*frog1, *(plantProjs[i])))
                    {
                        continua = 0;
                        i = NPLANTPROJS;
                    }
                }

                // Se non e' gia morto
                if (continua != 0)
                { // temporaneamnete metti lo stato come morte
                    continua = 0;
                    for (int i = 0; i < game.nsprites; i++)
                    {
                        // Se tocca almeno una sprite sicura
                        if (spriteIntersects2(*frog1, *(game.sprites[i])))
                        {
                            if (game.sprites[i]->type == CROC || game.sprites[i]->type == SIDEWALK || game.sprites[i]->type == GRASS || game.sprites[i]->type == REDCROC)
                            {
                                // Torna a vivere
                                continua = 1;
                                i = game.nsprites;
                            }
                        }
                    }
                }
                // Per ogni sprite
                for (size_t i = 0; i < game.nsprites; i++)
                {
                    // Che interseca
                    if (spriteIntersects2(*frog1, *(game.sprites[i])))
                    {
                        // Per ogni tana che tocca (caso vinto manche)
                        if (game.sprites[i]->type == HOLE)
                        {
                            // Aumenta punteggio in base al tempo + 1000
                            *punteggio += 1000 + (2000 * (larghezzaBarra)) / (SWIDTH - 11);
                            // Cambia continua
                            continua = 2;
                            // Salva posizione tana
                            int tempx = game.sprites[i]->x;
                            int tempy = game.sprites[i]->y;
                            // Cambia tana in tana piena
                            *game.sprites[i] = createHole(1);
                            // Ripristina posizione
                            game.sprites[i]->x = tempx;
                            game.sprites[i]->y = tempy;
                            i = game.nsprites; // Break
                        }
                        else if (game.sprites[i]->type == HOLEFILLED)
                        {
                            // Se sei stupido e sei entrato in una tana dove sei gia entrato muori
                            continua = 0;
                            i = game.nsprites; // break
                        }
                    }
                }
                break;
            case REDCROC:
            case REDCROCFELL:
            case CROC:
                // Caso coccodrillo
                copySprite(game.sprites[spriteLetta->id], spriteLetta);
                int crocid = spriteLetta->id - startIdCrocs;
                // Se il coccodrillo si trova a napoli (thread terminato)
                if (spriteLetta->x == NAPOLI)
                {
                    // Join
                    pthread_join(crocThreads[crocid], NULL);

                    // Inizio ricerca flusso con coccodrilli minori
                    flussoleggero = 0;
                    contFlussoMinore = 0;
                    // Diminiuisci il conto di coccdrilli nel flusso dove e' presente il coccodrillo
                    crocsaflusso[NFLOWS - 1 - (spriteLetta->y - 16) / 4]--;

                    // Troppo tempo per spiegare, genera un array con i flussi che hanno il numero minore di coccodrilli
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
                    // Scegli a caso un flusso tra quelli che hanno il numero minore di coccodrilli
                    flussoscelto = arrayflussiminori[rand() % (contFlussoMinore)];
                    // Aumenta i coccodrilli in quel flusso
                    crocsaflusso[flussoscelto]++;
                    // Stabilisci la x del prossimo coccodrillo da spawnare
                    int xscelto;
                    if (flussi[flussoscelto] > 0)
                    {
                        xscelto = 0 - 17 - 18 * crocsaflusso[flussoscelto]; // magic numbers, 18 e' la larghezza del coccodrillo
                    }
                    else
                    {
                        xscelto = game.sizex - 1 + 18 * (crocsaflusso[flussoscelto]);
                    }

                    // Genera coccodrillo nella nuova posizione

                    // crocargs[crocid].buffer = spriteBuffer;
                    crocargs[crocid].id = crocs[crocid]->id;
                    crocargs[crocid].scr = game;
                    crocargs[crocid].type = CROC + (9 * (rand() % 6 == 0));
                    crocargs[crocid].dir = flussi[flussoscelto];
                    crocargs[crocid].x = xscelto;
                    crocargs[crocid].y = game.sizey - (ELEMENTHEIGHT * (flussoscelto + 2));
                    pthread_create(&(crocThreads[crocid]), NULL, &croc, &(crocargs[crocid]));
                }
                break;

            case TIMEBAR:
                // Caso barra di tempo
                copySprite(barratempo, spriteLetta);
                larghezzaBarra = barratempo->w;
                // Se la barra e' uscita dallo schermo
                if (spriteLetta->x >= game.sizex)
                {
                    // scadutotempo = 1; // Il tempo e' scaduto
                    continua = 0; // La manche e' persa
                }
                else
                {
                    // Se e' tutto normale
                    // Locka per ncurses
                    pthread_mutex_lock(&mutexNCurses);
                    pthread_mutex_lock(&spriteMutex);

                    // Aggiorna scrbot con la barra del tempo
                    werase(scrbot.win);
                    printSprite(scrbot, *barratempo);
                    barratempo->y = 1;
                    printSprite(scrbot, *barratempo);
                    for (size_t i = 0; i < NVITE; i++)
                    {
                        printSprite(scrbot, *scrbot.sprites[i]);
                    }

                    wrefresh(scrbot.win);

                    pthread_mutex_unlock(&spriteMutex);
                    pthread_mutex_unlock(&mutexNCurses);
                }

                break;
            default:

                break;
            }
        }

        end = clock();
        t2 += ((end - start)); // Stabilisci clock

        // Contiene tutto il processo di stampa
        pthread_mutex_lock(&mutexNCurses);
        screenRefreshSprites(game);
        pthread_mutex_unlock(&mutexNCurses);

        usleep(FRAMETIME / 8);

        // Se sono indicativamente passati 1000 tick di clock e ancora non si e' superato il numero di coccodrilli massimo
        if (t2 >= 125 && contCrocs < NCROCS)
        {
            t2 = 0; // resetta clock
            // Genera coccodrillo
            // Trova flussi piu piccoli
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
            int xscelto;
            if (flussi[flussoscelto] > 0)
            {
                xscelto = 0 - 17 - 18 * crocsaflusso[flussoscelto];
            }
            else
            {
                xscelto = game.sizex - 1 + 18 * (crocsaflusso[flussoscelto]);
            }

            activeThreads[crocs[contCrocs]->id] = 1;

            // for (size_t i = 0; i < DIM_BUFFER; i++)
            // {
            //     spriteBuffers[crocs[contCrocs]->id][i] = allocateSprite();
            // }

            // crocargs[contCrocs].buffer = spriteBuffer; // s[crocs[contCrocs]->id];
            crocargs[contCrocs].id = crocs[contCrocs]->id;
            // crocargs[contCrocs].mutex = &spriteMutex;//es[crocs[contCrocs]->id];
            crocargs[contCrocs].scr = game;
            crocargs[contCrocs].type = CROC + (9 * (rand() % 6 == 0));
            crocargs[contCrocs].dir = flussi[flussoscelto];
            crocargs[contCrocs].x = xscelto;
            crocargs[contCrocs].y = game.sizey - (ELEMENTHEIGHT * (flussoscelto + 2));
            // crocargs[contCrocs].contBuffer = &contBuffer;//[crocs[contCrocs]->id];
            // contBuffer[crocs[contCrocs]->id]=leggiCont[crocs[contCrocs]->id]=0;
            pthread_create(&(crocThreads[contCrocs]), NULL, &croc, &(crocargs[contCrocs]));

            contCrocs++;
        }
    }

    //      CANCELLA OGNI MUTEX ECC

    for (size_t i = 0; i < NCROCS; i++)
    {
        //
        if (activeThreads[crocs[i]->id] == 1)
        {

            pthread_cancel(crocThreads[i]);
        }
    }
    // sleep(1);

    for (size_t i = 0; i < NPLANTS; i++)
    {
        pthread_cancel(plantThreads[i]);
    }

    for (size_t i = 0; i < NFROGPROJS; i++)
    {
        if (activeThreads[frogProjs[i]->id] == 1)
        {
            pthread_cancel(frogProjsThreads[i]);
        }
    }

    for (size_t i = 0; i < NPLANTPROJS; i++)
    {
        if (activeThreads[plantProjs[i]->id] == 1)
        {
            pthread_cancel(plantProjsThreads[i]);
        }
    }

    pthread_cancel(frogThread);

    pthread_cancel(barraTempoThread);

    return continua;
}

//      RANA
void *frog(void *threadargs)
{
    FrogArgs *args = ((FrogArgs *)threadargs);
    int dentroflusso = 0;

    Sprite frog1;
    frog1 = (createFrog());
    frog1.id = args->id;
    frog1.x = 0;
    frog1.y = args->scr.sizey - frog1.h;
    short arrayclr2[30] = {03, 07, 23, 23, 07, 03,
                           03, 03, 23, 23, 03, 03,
                           05, 05, 23, 2, 05, 05,
                           05, 05, 23, 2, 05, 05,
                           05, 05, 05, 05, 05, 05};
    short clrfinalenormale[30] = {03, 07, 23, 23, 07, 03,
                                  03, 03, 23, 23, 03, 03,
                                  05, 05, 23, 2, 05, 05,
                                  05, 05, 05, 05, 05, 05};

    int continua = 1;
    int frames = FRAMETIME;
    int diry = 0, dirx = 0, movedY = 0, movedHorizontal = 0;
    int c;
    clock_t start, end;
    long t;

    // Loop

    while (1)
    {

        frog1.type = FROG;

        // Determina che tipo di rana deve essere, in baso allo stato comunicatogli dal padre
        // pthread_mutex_lock(&spriteMutex);
        // if (args->frog->type == FROGSPARANTE)
        // {
        // }
        // else
        // {
        //     frog1.type = FROG;
        // }
        // pthread_mutex_unlock(&spriteMutex);

        // Stabilisci valore dello sleep
        frames = FRAMETIME;
        // Se non si sta muovendo, quindi non e' in alcun animazione
        if (movedY == 0 && movedHorizontal == 0)
        {
            // Prendi input
            pthread_mutex_lock(&mutexNCurses);

            c = (int)wgetch(args->scr.win);
            flushinp();
            pthread_mutex_unlock(&mutexNCurses);

            switch (c)
            {
                // Dentro ogni case c'e' la direzione che prende la rana e il frame dell animazione dove che inizia
            case KEY_UP:
                if (!screenSpriteIntersectsWall(args->scr, frog1, TOPWALL))
                {
                    diry = -1;
                    dirx = 0;
                    movedY = 4;
                }
                break;
            case KEY_DOWN:
                if (!screenSpriteIntersectsWall(args->scr, frog1, BOTTOMWALL))
                {
                    movedY = 4;
                    diry = 1;
                    dirx = 0;
                }
                // frog1.type = FROGSPARANTE;
                break;
            case KEY_LEFT:
                if (!screenSpriteIntersectsWall(args->scr, frog1, LEFTWALL))
                {
                    diry = 0;
                    dirx = -1;
                    movedHorizontal = 3;
                }
                break;
            case KEY_RIGHT:
                if (!screenSpriteIntersectsWall(args->scr, frog1, RIGHTWALL))
                {
                    diry = 0;
                    dirx = 1;
                    movedHorizontal = 3;
                }
                break;
            case 32:
                frog1.type = FROGSPARANTE;
                break;
            case 'q':
                frog1.type = FORGPAUSE;
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
            // moveSprite(scr, &frog1, dirx, diry);
            frames = FRAMETIME;
            moveSprite(args->scr, &frog1, dirx, 0);
            dirx = 0;
            movedHorizontal--;
            break;
        default:
            frames = FRAMETIME;
            moveSprite(args->scr, &frog1, dirx, 0);
            // moveSprite(scr, &frog1, dirx, diry);
            movedHorizontal--;
            break;
        }
        // Se e' all interno dell'azione dello spostamento verticale, esegui le azioni associate ad ogni frame
        switch (movedY)
        {
        case 4:
            frames = FRAMETIME;
            // moveSprite(scr, &frog1, dirx, diry);
            if (diry)
            {
                moveSprite(args->scr, &frog1, 0, diry);
                frog1.w = 6;
                frog1.h = 5;

                frog1.txt = L"▟▗▞▗▖▙"
                            "▐▟▘▝▙▌"
                            " ▐▐█▌ "
                            "▐▜▚█▛▌"
                            "▜ ▀▀ ▛";

                frog1.clr = arrayclr2;
            }

            movedY--;
            break;

        case 1:
            frames = FRAMETIME;
            frog1.w = 6;
            frog1.h = 4;
            frog1.txt = L"▟▗▞▗▖▙"
                        "▐▟▘▝▙▌"
                        "▐▜▚█▛▌"
                        "▜ ▀▀ ▛";

            frog1.clr = clrfinalenormale;
            moveSprite(args->scr, &frog1, 0, diry);
            movedY--;
        case 0:
            // frog1.h = 4;

            frames = FRAMETIME;
            diry = 0;
            break;
        default:
            // frog.x+=dirx;
            // frog.y+=diry;
            frames = FRAMETIME;
            moveSprite(args->scr, &frog1, 0, diry);
            // moveSprite(scr, &frog1, dirx, diry);
            movedY--;
            break;
        }
        // Se sei dentro un flusso
        if (dentroflusso)
        {
            // E non stai saltando
            if (movedY == 0)
            {
                // Determina su che flusso e' la rana, e spostala come fosse un coccodrillo
                int flussoattuale = args->nflussi - 2 - (frog1.y - 20) / 4;
                if (frog1.y <= args->scr.sizey - 8 && frog1.y >= (NGRASS)*ELEMENTHEIGHT)
                {

                    if (args->flussi[flussoattuale] > 0)
                    {
                        moveSprite(args->scr, &frog1, 1, 0);
                    }
                    else
                    {
                        moveSprite(args->scr, &frog1, -1, 0);
                    }
                    // Fai aspettare la rana la stessa sleep del coccodrillo, meno un frametime per dare spazio ad input
                    frames = (FRAMETIME * abs(args->flussi[flussoattuale])) - FRAMETIME;
                    dentroflusso = 0;
                }
                else
                {
                    frames = FRAMETIME;
                }
            }
        }
        else
        {
            dentroflusso = 1;
        }
        // pthread_mutex_unlock(&mutexNCurses);

        while (contBuffer >= DIM_BUFFER)
            usleep(frames);

        pthread_mutex_lock(&spriteMutex);

        if (contBuffer < DIM_BUFFER)
        {

            copySprite(spriteBuffer[scriviCont], &frog1);
            scriviCont = (scriviCont + 1) % DIM_BUFFER;

            contBuffer++;
        }
        pthread_mutex_unlock(&spriteMutex);

        usleep(frames);
    }
}

void *croc(void *crocargs)
{
    // Tutte i vari colori e sprite dei vari coccodrilli che ci possono essere
    short colorisottosinistra[72] = {07, 07, 07, 07, 07, 17, 07, 07, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01,
                                     07, 07, 07, 07, 07, 17, 07, 07, 26, 26, 26, 46, 26, 46, 26, 02, 02, 01,
                                     07, 07, 07, 07, 07, 07, 07, 07, 07, 02, 26, 02, 26, 02, 02, 02, 26, 26,
                                     01, 46, 46, 46, 46, 46, 46, 46, 02, 26, 02, 02, 02, 02, 02, 02, 02, 26};

    short colorisottodestra[72] = {02, 02, 02, 02, 02, 12, 02, 02, 07, 07, 07, 17, 07, 07, 07, 07, 07, 07,
                                   02, 02, 02, 02, 02, 12, 02, 02, 26, 27, 07, 07, 17, 07, 07, 07, 07, 07,
                                   07, 07, 07, 07, 07, 07, 07, 07, 07, 07, 07, 07, 07, 07, 07, 07, 07, 07,
                                   01, 46, 46, 46, 46, 46, 46, 46, 02, 26, 02, 02, 02, 02, 02, 02, 02, 26};
    wint_t *txtsinistra = L"█▙▄▄▄▀▙▖          "
                          " ▀▀▀▀███▀▄▀▄▀▄▀▄▄ "
                          "  ▄ ▄▀▄███▀█▀███▀▀"
                          " ▄█▀▀▀███▄███████▄";

    short sinistraVerde[72] = {05L, 05L, 05L, 05L, 05L, 15, 05L, 05L, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                               1, 15, 05L, 15, 05L, 05, 05, 05, 56, 56, 56, 46, 56, 46, 56, 05L, 05L, 1,
                               1, 1, 41, 1, 41, 41, 41, 05, 05, 05, 56, 05, 56, 05, 05, 05, 56, 56,
                               1, 46, 46, 46, 46, 46, 46, 46, 05, 56, 05, 05, 05, 05, 05, 05, 05, 56};

    short sinistraRosso[72] = {07, 07, 07, 07, 07, 17, 07, 07, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01,
                               01, 17, 07, 17, 07, 07, 07, 07, 76, 76, 76, 46, 76, 46, 76, 07, 07, 01,
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

    short gialloDestra[72] = {01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 02, 02, 12, 02, 02, 02, 02, 02,
                              01, 02, 02, 26, 46, 26, 46, 26, 26, 26, 02, 02, 02, 02, 12, 02, 12, 1,
                              26, 26, 02, 02, 02, 26, 02, 26, 02, 02, 02, 41, 41, 41, 01, 41, 01, 01,
                              26, 02, 02, 02, 02, 02, 02, 02, 26, 02, 46, 46, 46, 46, 46, 46, 46, 1};

    short gialloSinistra[72] = {02, 02, 02, 02, 02, 12, 02, 02, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01,
                                01, 12, 02, 12, 02, 02, 02, 02, 26, 26, 26, 46, 26, 46, 26, 02, 02, 01,
                                01, 01, 41, 01, 41, 41, 41, 02, 02, 02, 26, 02, 26, 02, 02, 02, 26, 26,
                                01, 46, 46, 46, 46, 46, 46, 46, 02, 26, 02, 02, 02, 02, 02, 02, 02, 26};
    wint_t *txtdestra = L"          ▗▟▀▄▄▄▟█"
                        " ▄▄▀▄▀▄▀▄▀███▀▀▀▀ "
                        "▀▀███▀█▀███▄▀▄ ▄  "
                        "▄███████▄███▀▀▀█▄ ";

    wint_t *txtsottosinistra = L"                  "
                               "█▙▄▄▄▀▙▖          "
                               " ▀▀▀▀▀▀▀▀         "
                               "                  ";

    wint_t *txtsottodestra = L"                  "
                             "          ▗▟▀▄▄▄▟█"
                             "        ▀▀▀▀▀▀▀▀  "
                             "                  ";
    CrocArgs *args = (CrocArgs *)crocargs;
    int type = args->type;
    int id = args->id;
    int y = args->y;
    int x = args->x;
    int dir = args->dir;
    Screen scr = args->scr;
    Sprite coccodrillo;
    coccodrillo.type = type;
    coccodrillo.id = id;
    coccodrillo.priority = 4;
    coccodrillo.w = 18;
    coccodrillo.h = 4;
    coccodrillo.y = y;

    coccodrillo.x = x;
    unsigned int frames = (FRAMETIME);
    frames = frames * abs(dir);
    int dirx = 0;
    short *giallo;
    short *colorisotto;
    short *clrfinalenormale, *clrosttofinale;
    wint_t *txtfinale, *txtsottofinale;
    // Determina qui quali sono le sprite e i colori da usare, in base a tipo e direzione
    if (dir > 0)
    {
        colorisotto = colorisottodestra;
        txtfinale = txtdestra;
        txtsottofinale = txtsottodestra;

        giallo = gialloDestra;
        dirx = 1;
        if (type == REDCROC)
        {
            clrfinalenormale = destraRosso;
        }
        else
        {
            clrfinalenormale = destraVerde;
        }
    }
    else
    {
        colorisotto = colorisottosinistra;
        txtfinale = txtsinistra;
        txtsottofinale = txtsottosinistra;

        giallo = gialloSinistra;
        dirx = -1;
        if (type == REDCROC)
        {
            clrfinalenormale = sinistraRosso;
        }
        else
        {
            clrfinalenormale = sinistraVerde;
        }
    }

    // Logica coccodrilli
    int raggiuntoMuro = 0;
    int animazioneVaGiu = 0;
    int andatogiu = 0;


    // Fino a quando non raggiungi muro
    while (!raggiuntoMuro)
    {

        // Imposta la sprite come quella determinata prima
        coccodrillo.txt = txtfinale;
        coccodrillo.type = type;
        coccodrillo.clr = clrfinalenormale;

        // Se il coccodrillo e' andato giu (rosso)
        if (andatogiu > 0)
        {
            // Cambia il tipo e sprite a quella giusta
            coccodrillo.type = REDCROCFELL;
            coccodrillo.clr = colorisotto;
            coccodrillo.txt = txtsottofinale;
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
            andatogiu = SECONDIGIU * FPS;
            animazioneVaGiu--;
            break;
        default:
            // Un quarto dei frame cambia colore al coccodrillo giallo, per lampeggiare
            if (animazioneVaGiu % 4)
            {
                coccodrillo.clr = clrfinalenormale;
            }
            else
            {
                coccodrillo.clr = giallo;
            }
            animazioneVaGiu--;
            break;
        }
        // Se sei un coccodrillo rosso che non e' andato giu ed e' sopra
        if (type == REDCROC && andatogiu == 0 && animazioneVaGiu <= 0)
        {
            // Se e' dentro lo schermo
            if (coccodrillo.x < scr.sizex && coccodrillo.x > 0)
            {
                // Determina casualmente se iniziare l'animazione per andare giu
                if (rand() % FPS * 2 == 0)
                {
                    // Inizia animazione in base agli fps e alla velocita'
                    animazioneVaGiu = (FPS * 3) / abs(dir);
                }
            }
        }

        // Sposta coccodrillo
        coccodrillo.x += dirx;

        while (contBuffer >= DIM_BUFFER)
            usleep(frames);

        pthread_mutex_lock(&spriteMutex);
        if (contBuffer < DIM_BUFFER)
        {
            copySprite(spriteBuffer[scriviCont], &coccodrillo);
            scriviCont = (scriviCont + 1) % DIM_BUFFER;

            (contBuffer)++;
        }

        pthread_mutex_unlock(&spriteMutex);

        // Controlla se ha raggiunto la fine del muro opposto
        if (dir > 0)
        {
            if (coccodrillo.x >= scr.sizex - 1)
            {
                raggiuntoMuro = 1;
            }
        }
        else
        {
            if (coccodrillo.x <= 0 - coccodrillo.w + 1)
            {
                raggiuntoMuro = 1;
            }
        }
        usleep(frames);
    }
    // Se ha raggiunto la fine del muro mandalo a napoli
    coccodrillo.x = NAPOLI;

    // Aggiorna i dati nel buffer l'ultima volta
    // while (contBuffer >= DIM_BUFFER)
    //     usleep(frames);

    pthread_mutex_lock(&spriteMutex);
    if (contBuffer < DIM_BUFFER)
    {
        copySprite(spriteBuffer[scriviCont], &coccodrillo);
        scriviCont = (scriviCont + 1) % DIM_BUFFER;

        (contBuffer)++;
    }
    pthread_mutex_unlock(&spriteMutex);

    // Esci
    pthread_exit(NULL);
}

void *plant(void *threadargs)
{

    PlantArgs *args = (PlantArgs *)threadargs;

    Sprite planta = createPlant((args->type == PLANT));

    unsigned int frames = (FRAMETIME);
    // Se il tipo passato alla fuznione e' la pianta morta
    if (args->type == PLANTDEAD)
    {
        // Inizializza la pianta come morta
        planta.x = args->x;
        planta.type = PLANTDEAD;
        planta.id = args->id;
        planta.y = 0;
        while (contBuffer >= DIM_BUFFER)
            usleep(frames);

        pthread_mutex_lock(&spriteMutex);

        if (contBuffer < DIM_BUFFER)
        {

            copySprite(spriteBuffer[scriviCont], &planta);
            scriviCont = (scriviCont + 1) % DIM_BUFFER;

            (contBuffer)++;
        }
        pthread_mutex_unlock(&spriteMutex);

        // Determina per quanto tempo deve stare morta, in secondi
        int duratamorte = 3 + rand() % 5;
        while (duratamorte >= 0)
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
    planta = createPlant(1);

    planta.x = args->x;
    planta.id = args->id;
    planta.y = 0;

    while (contBuffer >= DIM_BUFFER)
        usleep(frames);

    pthread_mutex_lock(&spriteMutex);

    if (contBuffer < DIM_BUFFER)
    {

        copySprite(spriteBuffer[scriviCont], &planta);
        scriviCont = (scriviCont + 1) % DIM_BUFFER;

        (contBuffer)++;
    }
    pthread_mutex_unlock(&spriteMutex);

    while (1)
    {

        planta.type = PLANT;
        // Possibilta' casuale ad ogni frame che spari
        if (rand() % ((FPS) * ((rand() % 4) + 10)) == 0)
        {
            // Cambia il tipo a pianta che spara
            planta.type = PLANTSHOT;

            // Invia al mutex
            while (contBuffer >= DIM_BUFFER)
                usleep(frames);

            pthread_mutex_lock(&spriteMutex);

            if (contBuffer < DIM_BUFFER)
            {

                copySprite(spriteBuffer[scriviCont], &planta);
                scriviCont = (scriviCont + 1) % DIM_BUFFER;

                (contBuffer)++;
            }
            pthread_mutex_unlock(&spriteMutex);
            // Ristabilisci tipo
            planta.type = PLANT;
        }
        usleep(frames);
    }
}

void *projectile(void *threadargs)
{
    ProjArgs *args = (ProjArgs *)threadargs;
    int id = args->id;
    int x = args->x;
    int y = args->y;
    int diry = args->diry;
    Screen scr = args->scr;
    Sprite proiettile = createProj();
    proiettile.id = id;
    proiettile.x = x;
    proiettile.y = y;

    // Se il proiettile e' sparato verso giu allora e' delle piante
    if (diry > 0)
    {
        // Inizializza proiettile pianta
        proiettile.type = PROJPLANT;

        proiettile.txt = L"▟▙▜▛";
        proiettile.w = 2;
        proiettile.h = 2;
        short arrayclr4[4] = {03, 03, 03, 03};
        proiettile.clr = arrayclr4;
    }
    unsigned int frames = (FRAMETIME) * 1.5;

    while (1)
    {
        moveSprite(scr, &proiettile, 0, diry);

        while (contBuffer >= DIM_BUFFER)
            usleep(frames);

        pthread_mutex_lock(&spriteMutex);

        if (contBuffer < DIM_BUFFER)
        {

            copySprite(spriteBuffer[scriviCont], &proiettile);
            scriviCont = (scriviCont + 1) % DIM_BUFFER;

            (contBuffer)++;
        }
        pthread_mutex_unlock(&spriteMutex);

        usleep(frames);
    }
}

void *timeBar(void *threadargs)
{
    TimeBarArgs *args = (TimeBarArgs *)threadargs;

    int secondi = args->secondi;
    Sprite barraTempo;
    barraTempo.x = 11;
    barraTempo.id = 0;
    barraTempo.y = 0;
    barraTempo.priority = 9;
    barraTempo.type = TIMEBAR;
    barraTempo.h = 1;
    barraTempo.w = SWIDTH - 11;
    barraTempo.txt = L"██████████████████████████████████████████████████████████████████";

    // Vari colori che puo avere la barra
    short arrayclr_barra[SWIDTH];
    for (int i = 0; i < SWIDTH; i++)
    {
        arrayclr_barra[i] = 03;
    }
    short arrayclr_barra1[SWIDTH];
    for (int i = 0; i < SWIDTH; i++)
    {
        arrayclr_barra1[i] = 02;
    }
    short arrayclr_barra2[SWIDTH];
    for (int i = 0; i < SWIDTH; i++)
    {
        arrayclr_barra2[i] = 07;
    }
    barraTempo.clr = arrayclr_barra;

    int cicli = 0;
    // Durata della sleep per cambiare di un pixel la barra
    int frames = ((secondi * 1000000) / ((SWIDTH - 11) * 2));
    int width = (SWIDTH - 11) * 2;
    // Per tutta la larghezza della barra
    while (cicli <= width)
    {
        // pthread_mutex_lock(&mutexNCurses);

        cicli++;

        // Meta' delle volte cambia la sprite, l'altra meta' la sposta e ripristina sprite
        if (cicli % 2 == 1)
        {
            barraTempo.txt = L"▐█████████████████████████████████████████████████████████████████";
        }
        else
        {
            barraTempo.txt = L"██████████████████████████████████████████████████████████████████";
            barraTempo.x++;
        }

        barraTempo.clr = arrayclr_barra;

        // Se e' al secondo terzo della barra cambia colore a giallo
        if (cicli >= (width * 1) / 3)
        {
            barraTempo.clr = arrayclr_barra1;
        }
        // Se e' al terzo terzo della barra cambia colore a  rosso
        if (cicli >= (width * 2) / 3)
        {
            barraTempo.clr = arrayclr_barra2;
        }

        while (contBuffer >= DIM_BUFFER)
            usleep(frames);
        pthread_mutex_lock(&spriteMutex);

        if (contBuffer < DIM_BUFFER)
        {
            copySprite(spriteBuffer[scriviCont], &barraTempo);

            scriviCont = (scriviCont + 1) % DIM_BUFFER;

            (contBuffer)++;
        }
        pthread_mutex_unlock(&spriteMutex);
        usleep(frames);
    }
    while (1)
        sleep(1);
}
