/*
 * hashrs.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:30
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "hash.h"

#define A   63689
#define B   378551

static unsigned a = 0;
static unsigned hash = 0;

unsigned hash_rs( const void *buf, size_t size )
{
    return hash_rs_update( 0, buf, size );
}

unsigned shash_rs( const char *buf )
{
    return shash_rs_update( 0, buf );
}

unsigned hash_rs_update( unsigned startval, const void *buf, size_t size )
{
    if( !startval ) {
        a = A;
    }

    for( hash = startval; size; size-- ) {
        hash = hash * a + *( ( unsigned char * ) buf );
        a *= B;
        buf = ( unsigned char * ) buf + 1;
    }

    return hash;
}

unsigned shash_rs_update( unsigned startval, const char *buf )
{
    if( !startval ) {
        a = A;
    }

    for( hash = startval; *buf; buf++ ) {
        hash = hash * a + *( ( unsigned char * ) buf );
        a *= B;
    }

    return hash;
}

