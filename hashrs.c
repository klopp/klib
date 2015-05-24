/*
 * hashrs.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:30
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "hash.h"

#define A   63689
#define B   378551

unsigned hash_rs( unsigned startval, const void * buf, size_t size )
{
    static unsigned a = 0;
    unsigned hash;

    if( !startval ) a = A;
    for( hash = startval; size; size--, (unsigned char *)buf++ )
    {
        hash = hash * a + *((unsigned char *)buf);
        a *= B;
    }
    return hash;
}

unsigned shash_rs( unsigned startval, const char * buf )
{
    return hash_rs( startval, buf, strlen(buf) );
}

