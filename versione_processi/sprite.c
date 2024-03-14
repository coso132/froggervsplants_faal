#include "sprite.h"

// void printSprite(WINDOW * win, Sprite sprit)
// {
//     wint_t c;
//  
//     for (int i = 0; i < sprit.h; i++)
//     {
//         // mvwprintw(win,sprit.y,sprit.x)
//        
//         for (int j = 0; j < sprit.w; j++)
//         {
//             c = sprit.txt[(sprit.w*i+j)];
//             if(c != ' ')
//             {
//                 wattr_on(win,COLOR_PAIR(sprit.clr[sprit.w*i+j]), NULL);
//                 //mvwaddch(win,sprit.y+i,sprit.x+j,c);
//                 mvwprintw(win,sprit.y+i,sprit.x+j,"%lc",c);                
//             }
//     
// 
//         }
//     }
//     //wattr_off(win,COLOR_PAIR(sprit.clr), NULL);
// }
// 
// void printSpriteColor(WINDOW * win, Sprite sprit, int cl)
// {
//     wint_t c;
//     wattr_on(win,COLOR_PAIR(c), NULL);
//     for (int i = 0; i < sprit.h; i++)
//     {
//         // mvwprintw(win,sprit.y,sprit.x)
// 
//         for (int j = 0; j < sprit.w; j++)
//         {
//             c = sprit.txt[(sprit.w*i+j)];
//             if(c != ' ')
//             {
//                
//                 //mvwaddch(win,sprit.y+i,sprit.x+j,c);
//                 mvwprintw(win,sprit.y+i,sprit.x+j,"%lc",c);                
//             }
//                
//            
//         }
//     }
// }

/*
Funzione che dato due linee (attraverso punto iniziale x e lunghezza l) 
fornisce in output se intersecano o no
*/
int lineIntersect(int x1, int l1, int x2, int l2)
{
    if(x1 > x2 && x1 < x2+l2)
    {
        return 1;
    }
    if (x1+l1 > x2 && x1+l1 < x2+l2)
    {
        return 1;
    }
    if (x2 > x1 && x2 < x1+l1)
    {
        return 1;
    }
    if (x2+l2 > x1 && x2+l2 < x1+l1)
    {
        return 1;
    }
    return 0;
}
int lineIntersect2(int x1, int l1, int x2, int l2)
{
    l1--;
    l2--;
    if(x1 >= x2 && x1 <= x2+l2)
    {
        return 1;
    }
    if (x1+l1 >= x2 && x1+l1 <= x2+l2)
    {
        return 1;
    }
    if (x2 >= x1 && x2 <= x1+l1)
    {
        return 1;
    }
    if (x2+l2 >= x1 && x2+l2 <= x1+l1)
    {
        return 1;
    }
    return 0;
}

/*
Funzione che, dato in input due sprite, fornsice in output se intersecano 
*/
int spriteIntersects(Sprite s1, Sprite s2)
{
    //Controlla se entrambe le dimensioni in altezza e larghezza della sprite si intersecano
    if(lineIntersect(s1.x,s1.w,s2.x,s2.w) && lineIntersect(s1.y,s1.h,s2.y,s2.h))
    {
        return 1;
    }
    return 0;
}
int spriteIntersects2(Sprite s1, Sprite s2)
{
    //Controlla se entrambe le dimensioni in altezza e larghezza della sprite si intersecano
    if(lineIntersect2(s1.x,s1.w,s2.x,s2.w) && lineIntersect2(s1.y,s1.h,s2.y,s2.h))
    {
        return 1;
    }
    return 0;
}




Sprite * allocateSprite()
{
    Sprite * returnVal= malloc(sizeof(Sprite));
    returnVal->x=returnVal->y=returnVal->h=returnVal->w=0;
    returnVal->clr=malloc(1);
    returnVal->txt=malloc(1);
}

void flipSprite(Sprite *s)
{
    // // short arraytemp[s->w*s->h];
    // short * arraytemp= malloc(sizeof(short)*s->w*s->h);
    // wint_t * arraytemp2= malloc(sizeof(wint_t)*s->w*s->h);
    short temps; wint_t tempc;
    // wint_t arraytemp2[s->w*s->h];
    for (size_t i = 0; i < s->h; i++)
    {
        for (size_t j = 0; j < s->w; j++)
        {
            temps = s->clr[i*s->h+(s->w-j-1)];
            s->clr[i*s->h+(s->w-j-1)]= s->clr[i*s->h+j];
            s->clr[i*s->h+j]=temps;

            tempc = s->txt[i*s->h+(s->w-j-1)];
            s->txt[i*s->h+(s->w-j-1)]= s->txt[i*s->h+j];
            s->txt[i*s->h+j]=tempc;

        }
    }
    // for (size_t i = 0; i < s->h; i++)
    // {
    //     for (size_t j = 0; j < s->w; j++)
    //     {
            
    //     }
    // }
    // free(arraytemp2);
    // free(arraytemp);
}


short * flipArray(short *a, int w, int h)
{
    short t;
    for (size_t i = 0; i < h; i++)
    {
        for (size_t j = 0; j < w/2; j++)
        {
            t = a[i*h + j];
            a[i*h + j]= a[i*h + w-1-j];
            a[i*h + w-1-j]=t;
        }
        
    }
    return a;
}

void copySprite(Sprite* s1, Sprite* s2)
{
    // *s1=*s2;

    s1->clr= realloc(s1->clr,sizeof(short)*s2->w*s2->h);
    s1->txt= realloc(s1->txt,sizeof(wint_t)*s2->w*s2->h);
    s1->x=s2->x;
    s1->y=s2->y;
    s1->type=s2->type;
    // s1->active=s2->active;
    s1->h=s2->h;
    s1->w=s2->w;
    s1->priority=s2->priority;
    s1->id=s2->id;
    
    // short * temp1 = malloc(sizeof(short)*s1->w*s1->h);

    // short * temp1 = (short *)calloc(s1->w*s1->h,sizeof(short));

    for (int i = 0; i < s1->h; i++)
    {
        for (int j = 0; j < s1->w; j++)
        {
            s1->clr[i*s1->w+j]= s2->clr[i*s1->w+j];
        }
    }
    // s1->clr=temp1;

    // wint_t * temp2 = (wint_t *)calloc(s1->w*s1->h,sizeof(wint_t));    
    for (int i = 0; i < s1->h; i++)
    {
        for (int j = 0; j < s1->w; j++)
        {
            s1->txt[i*s1->w+j]= s2->txt[i*s1->w+j];
        }
    }
    // s1->txt=temp2;
}


