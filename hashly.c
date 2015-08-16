/*
 * hashly.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:28
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "hash.h"

#define A   1664525
#define B   1013904223

static unsigned hash = 0;

unsigned hash_ly( unsigned startval, const void * buf, size_t size )
{
    for( hash = startval; size; size--, (unsigned char *)buf++ )
        hash = (hash * A) + *((unsigned char *)buf) + B;
    return hash;
}

unsigned shash_ly( unsigned startval, const char * buf )
{
    for( hash = startval; *buf; buf++ )
        hash = (hash * A) + *((unsigned char *)buf) + B;
    return hash;
}

