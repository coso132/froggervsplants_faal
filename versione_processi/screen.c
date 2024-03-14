#include "screen.h"

/*
Funzione che inizializza i colori custom e le relative pair 
creati da me, inoltre alle varie funzioni di ncurses necessarie 
per l'avvio di uno schermo
*/
void initializeScreenFunctions()
{
    initscr();
    noecho();
    start_color();

    //Inizializzo ogni colore in base ai valori rgb 
    init_color(BLACK, 0, 0, 0);
    init_color(WHITE, 1000, 1000, 1000);
    init_color(YELLOW, 882, 882, 0);
    init_color(MAGENTA, 580, 0, 0);
    init_color(BLUE, 0, 301, 619);
    init_color(BROWN, 496, 398, 0);
    init_color(GREEN, 0, 900, 0);
    init_color(DARKG, 0, 600, 0);
    init_color(DARKGREY, 400, 400, 400);
    init_color(GREY, 700, 700, 700);

    // init_color(GRASSGREEN1, 700, 700, 700);

    // init_color(GRASSGREEN2, 700, 700, 700);

    /*
    Metodo della gestione dei colori per permettere il cambio arbitrario
    dello singolo sfondo o background di un colore

    Sfruttando la conoscenza sul numero massimo di colori, e le operazioni
    intere del linguaggio C, si considera una coppia di colori in base
    alla macro dei colori custom

    Considerando la pair '24', il 2 rappresenta la macro YELLOW, di conseguenza
    lo sfondo sara giallo, il 4 la macro BLUE, di conseguenza il testo sara' blue

    Attraverso il modulo di NCOLORS si puo ottenere il colore del testo, 
    e attraverso la divisione di NCOLORS si puo' ottenere il colore dello sfondo,
    e cambiarlo in modo dinamico.

    Questo necessita la creazione di una pair di colori formata da ogni colore come
    sfondo e ogni colore come foreground
    */


    //Per ogni foreground
    for (int i = 0; i < NCOLORS; i++)
    {
        //Per ogni background
        for (int j = 0; j < NCOLORS; j++)
        {
            //Inizializza la pair di numero j*10+i
            init_pair(j * NCOLORS + i, i, j);

            //Attiva il colore e stampalo a schermo insieme al suo numero
            //attron(COLOR_PAIR(j * 10 + i));
            //mvaddch(j,i*2,'X');
            //mvprintw(j,i*3,"%d",j * 10 + i);
        }
    }

    //Getchar per fermare un attimo il programma, permettendo la visualizzazione di ogni colore prima di iniziare il gioco
    //getch();
    // bkgd(COLOR_PAIR(0));
    cbreak();
    curs_set(0);
}

//Funzione che dato uno Screen, una Sprite, e una Macro per indicare il muro, indica se la sprite tocca quel muro
int screenSpriteIntersectsWall(Screen scr, Sprite sprit, int dir)
{

    switch (dir)
    {
    case TOPWALL:
        if ((sprit.y) <= 0)
            return 1;
        break;
    case BOTTOMWALL:
        if ((sprit.y + sprit.h) >= scr.sizey)
            return 1;
        break;
    case LEFTWALL:
        if ((sprit.x) <= 0)
            return 1;
        break;
    case RIGHTWALL:
        if ((sprit.x + sprit.w) >= scr.sizex)
            return 1;
        break;

    default:
        break;
    }
    return 0;
}

//Controlla se una sprite interseca un qualunque muro 
int screenSpriteIntersectsAnyWall(Screen scr, Sprite sprit)
{
    for (size_t i = 0; i < 4; i++)
    {
        screenSpriteIntersectsWall(scr, sprit, i);
    }
}
Sprite eldiablo;
/*
Funzione che refresha lo schermo di una Screen, ristampando ogni singola
sprite che contiene all interno del suo array, seguendo le regole di priorita'
*/
void screenRefreshSprites(Screen scr)
{
    //Clear dello schermo
    werase(scr.win);

    //bomboclaat
   
        
    //Stabilisci il background
    wbkgd(scr.win, COLOR_PAIR(scr.background));
    /*
    Salva temporaneamente il valore del background, che puo' essere sovrascritto
    dalla funzione di print sprite presente dopo
    */
    int background = scr.background;

    //Per ogni valore di priorita' (stampa in ordine)
    for (int i = 0; i < PLEVEL; i++)
    {
        //Per ogni sprite
        for (int j = 0; j < scr.nsprites; j++)
        {
            //Per ogni sprite con la priorita' attuale
            if ((scr.sprites[j]->priority) == i)
            {
                // if (i != 0)
                // {
                //     for (int k = 0; k < scr.nsprites; k++)
                //     {
                //         if (i == 0)
                //         {
                //             if (spriteIntersects(*(scr.sprites[j]), *(scr.sprites[k])))
                //             {
                //                 scr.background = (scr.sprites[k]->clr[0]);
                //             }
                //         }
                //     }
                // }
                // printSprite(scr, *(scr.sprites[j]));
                
                //Stampala
                printSpriteGetBackgroundColor(scr, *(scr.sprites[j]));
            }
        }
    }
    //Ripristina sfondo
    scr.background = background;
    //Refresh window
    wrefresh(scr.win);
}

/*
Funzione che aggiunge una sprite all'array di sprite dello screen,
funziona come una lista
*/
void screenAddSprite(Screen *scr, Sprite *sprite)
{
    if (scr->nsprites <= MAXSPRITES)
    {
        scr->sprites[scr->nsprites] = sprite;
        sprite->id=scr->nsprites;
        scr->nsprites++;
    }
}

/*
Funzione che rimuove una sprite all'array di sprite dello screen,
funziona come una lista
*/
// void screenRemoveSprite(Screen scr, int id)
// {
//     int found = 0;
//     for (int i = 0; i < MAXSPRITES; i++)
//     {
//         if(i==id)
//         {
//             scr.sprites[i]=scr.sprites[i+1];
//             for (int j = i; j < MAXSPRITES-1; j++)
//             {
                
//             }
            
//             i=MAXSPRITES;
//         }
//     }
// }

/*
Funzione che inizializza uno Screen, assegnando ogni valore necessario 
*/
void screenInit(Screen *scr, int startX, int startY, int sizeX, int sizeY, int background)
{
    WINDOW *win2 = newwin(sizeY, sizeX, startY, startX);
    
    scr->sizex = sizeX;
    scr->sizey = sizeY;
    scr->win = win2;
    scr->background = background;
    scr->nsprites = 0;
    // (*scr).sprites = (Sprite **)malloc(MAXSPRITES * sizeof(Sprite *));
}


/*
Funzione che muove una sprite sullo schermo
*/
void moveSprite(Screen scr, Sprite *sprite, int dirx, int diry)
{
    // clearSprite(scr,*sprite);
    // refreshBackground(scr);
    int canmove = 1;
    if (diry != 0)
    {
        if (diry > 0)
        {
            canmove = !(screenSpriteIntersectsWall(scr, *sprite, BOTTOMWALL));
        }
        else
        {
            canmove = !(screenSpriteIntersectsWall(scr, *sprite, TOPWALL));
        }
    }
    if (dirx != 0)
    {
        if (dirx > 0)
        {
            canmove = !(screenSpriteIntersectsWall(scr, *sprite, RIGHTWALL));
        }
        else
        {
            canmove = !(screenSpriteIntersectsWall(scr, *sprite, LEFTWALL));
        }
    }
    if (canmove)
    {
        sprite->x += dirx;
        sprite->y += diry;
    }
    // printSpriteGetBackgroundColor(scr,*sprite);
    // wrefresh(scr.win);
}

void printSprite(Screen scr, Sprite sprit)
{
    wint_t c;

    for (int i = 0; i < sprit.h; i++)
    {
        // mvwprintw(win,sprit.y,sprit.x)

        for (int j = 0; j < sprit.w; j++)
        {
            c = sprit.txt[(sprit.w * i + j)];
            if (c != ' ')
            {
                wattr_on(scr.win, COLOR_PAIR(sprit.clr[(sprit.w * i + j)]), NULL);
                // mvwaddch(win,sprit.y+i,sprit.x+j,c);
                mvwprintw(scr.win, sprit.y + i, sprit.x + j, "%lc", c);
            }
        }
    }
    // wattr_off(win,COLOR_PAIR(sprit.clr), NULL);
}


/*
Funzione finale della stampa di una sprite, dato uno screen e una sprite la stampa
sullo schermo tenendo conto dello sfondo dello screen e delle sprite con priorita'
piu' bassa che interseca
*/
void printSpriteGetBackgroundColor(Screen scr, Sprite sprit)
{
    //Char unicode temporaneo
    wint_t c;
    //Colore temporaneo, posizioni x e y temporanee
    int clr, xt, yt;
    //Sprite temporanea grande 1x1, e sprite temporanea
    Sprite onebyone, temp;
    //Salvo background temporaneamente
    int bkgd = scr.background;

    //Inizializzo sprite 1x1 nella posizione della sprite da stampare
    onebyone.w=onebyone.h=1;
    onebyone.x=sprit.x;
    onebyone.y=sprit.y;
    //Per ogni carattere
    //Per l'altezza
    for (int i = 0; i < sprit.h; i++)
    {
        //Per la largehzza
        for (int j = 0; j < sprit.w; j++)
        {            
            //Estrai carattere da stampare
            c = sprit.txt[(sprit.w * i + j)];
            //Se e' diverso dallo spazio (trasparenza)
            if (c != ' ' || sprit.priority==0)
            {
                //Salvati il colore che deve prendere
                clr = sprit.clr[(sprit.w * i) + j];
                //Se il colore e' inferiore al numero di colori (colore di background nero/trasparente)
                if (clr < NCOLORS)
                {
                    //Cerca ogni sprite che sta sotto di essa
                    //Per ogni sprite
                    for (int k = 0; k < scr.nsprites; k++)
                    {
                        
                        temp = *scr.sprites[k];
                        //Se la priorita' e' 0 (sprite di sfondo)
                        if (temp.priority== 0 )
                        {
                            //Se interseca con il pixel della sprite che sta venendo stampato
                            if (spriteIntersects(onebyone, temp))
                            {
                                //Preleva il colore di quel pixel nella sprite di sfondo
                                yt=onebyone.y- temp.y;
                                xt=onebyone.x-temp.x;
                                //Aggiorna il colore dello sfondo in base a quello
                                scr.background = (temp.clr[yt*temp.w + xt]);
                            }
                        }
                    }
                    //Cambia il colore del carattere per fare in modo che abbia come sfondo lo sfondo del colore appena prelevato
                    clr = (clr % NCOLORS) + ((scr.background / NCOLORS) * NCOLORS);
                    //Aggiorna background
                    scr.background=bkgd;
                }
                //Attiva colore da stampare, incluso background appena trovato (o no)
                wattr_on(scr.win, COLOR_PAIR(clr), NULL);
                //Stampa carattere
                mvwprintw(scr.win, sprit.y + i, sprit.x + j, "%lc", c);            
                wattr_off(scr.win, COLOR_PAIR(clr), NULL);    
            }
            //Aggiorna posizione x 1x1
            onebyone.x++;
        }
        //Resetta posizione x
        onebyone.x=sprit.x;
        //Aggiorna posizione y 1x1
        onebyone.y++;
    }
    // wattr_off(win,COLOR_PAIR(sprit.clr), NULL);
}

