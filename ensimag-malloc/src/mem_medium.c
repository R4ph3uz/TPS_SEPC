/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}

void decoupe_TZL(unsigned int nombre){
    if(nombre >=(FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant)){ // tout est utilisé il faut malloc
        mem_realloc_medium(); // réalloue
    }
    else if(arena.TZL[nombre]==0){
        // fprintf(stderr,"nombre %u arena %lu\n",nombre,(unsigned long)arena.TZL[nombre]);
        decoupe_TZL(nombre+1);
    }
    
    //decoupe le bloc arene.TZL[nombre] -> reinsere les 2 blocs dans la liste chainée de arene.tzl(nombre-1)
    void* compagnon1 = arena.TZL[nombre];
    arena.TZL[nombre] = *((void**) arena.TZL[nombre]);
    unsigned long compagnon2 = (unsigned long) compagnon1^(1<<(nombre-1));
    unsigned long minCompagnon = ((unsigned long) compagnon1 <compagnon2) ? (unsigned long)compagnon1 : compagnon2;
    unsigned long maxCompagnon = ((unsigned long) compagnon1 <compagnon2) ? compagnon2 : (unsigned long)compagnon1;
    *((void**) minCompagnon) =(void*)maxCompagnon;
    arena.TZL[nombre-1] = (void*) minCompagnon;
}

void* emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    unsigned int indice = puiss2(size+32-(size%32)); // +32 pour compter les marqueurs
    if(arena.TZL[indice] ==0)// si la bonne taille est PAS disponible
    {
        decoupe_TZL(indice+1);
    }
    void* addresse = arena.TZL[indice];
    // fprintf(stderr, "adresse%lu\n",(unsigned long) addresse);
    arena.TZL[indice] = *((void**) arena.TZL[indice]);
    // fprintf(stderr, "prochain adresse %lu\n",(unsigned long) arena.TZL[7]);
    void* pointeur = mark_memarea_and_get_user_ptr(addresse,1<<indice,MEDIUM_KIND);
    return pointeur;
}

void recolle_TZL(unsigned int nombre,void* pointeur){
    unsigned long compagnon_calc = (unsigned long) pointeur ^(unsigned long)(1<<nombre);
    unsigned long actuel = (unsigned long) arena.TZL[nombre];
    unsigned long ancien = actuel;

    while ((void*)actuel != NULL && actuel != compagnon_calc){
        //avance tant que c'est pas le buddy ou que c'est fini
        ancien=actuel;
        actuel=*((unsigned long *) actuel);
    }
    if(actuel== compagnon_calc){// on a le buddy
        *((void**)ancien) = *((void**)actuel); //enleve l'actuel
        void* ptr = ((unsigned long)pointeur<compagnon_calc) ? pointeur:(void*)compagnon_calc;
        recolle_TZL(nombre+1,ptr);
    }
    else{//vide
        *((void**)pointeur) = arena.TZL[nombre];
        arena.TZL[nombre]=pointeur;
    }

}


void efree_medium(Alloc a) {
    assert(a.kind==MEDIUM_KIND);
    unsigned long indice= puiss2(a.size);
    recolle_TZL(indice,a.ptr);

}


