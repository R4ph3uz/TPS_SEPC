/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

void * emalloc_small(unsigned long size){
    if (arena.chunkpool==0){
        // fprintf(stderr,"On passe ici\n\n\n");
        unsigned long size = mem_realloc_small();
        unsigned long nbBlocs = size / CHUNKSIZE;
        unsigned long * position = arena.chunkpool;
        // fprintf(stderr,"position : %lu\n",(unsigned long) position);
        unsigned long * suivant = position + CHUNKSIZE/sizeof(unsigned long);
        // fprintf(stderr,"suivant : %lu\n",(unsigned long) suivant);
        // fprintf(stderr,"nbBlocs : %lu\n",nbBlocs);
        for (int i = 1; i < nbBlocs-1; i++){
            *position = (unsigned long) suivant;
            position=  (unsigned long*) *position;
            suivant = suivant + CHUNKSIZE/sizeof(unsigned long);
        }
        *position=0;
    }
    //fprintf(stderr,"arena chunkpool %lu\n",(unsigned long)arena.chunkpool);
    
    void *next = (void *) *((unsigned long*) arena.chunkpool);
    void *current = arena.chunkpool;

    arena.chunkpool = next;
    void *ptr = mark_memarea_and_get_user_ptr(current, CHUNKSIZE, SMALL_KIND);

    return (void *) ptr;
}

void efree_small(Alloc a) {
    void *current = arena.chunkpool;
    *((unsigned long *)a.ptr)=(unsigned long) current;
    arena.chunkpool=a.ptr;
}
