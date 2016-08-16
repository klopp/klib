/*
 * hashjen.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:25
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "hash.h"

static unsigned hash = 0;

unsigned hash_jen( const void *buf, size_t size )
{
    return hash_jen_update( 0, buf, size );
}

unsigned shash_jen( const char *buf )
{
    return shash_jen_update( 0, buf );
}

unsigned hash_jen_update( unsigned startval, const void *buf, size_t size )
{
    for( hash = startval; size; size-- ) {
        hash += *( ( unsigned char * ) buf );
        hash += ( hash << 10 );
        hash ^= ( hash >> 6 );
        buf = ( unsigned char * ) buf + 1;
    }

    hash += ( hash << 3 );
    hash ^= ( hash >> 11 );
    hash += ( hash << 15 );
    return hash;
}

unsigned shash_jen_update( unsigned startval, const char *buf )
{
    for( hash = startval; *buf; buf++ ) {
        hash += *( ( unsigned char * ) buf );
        hash += ( hash << 10 );
        hash ^= ( hash >> 6 );
    }

    hash += ( hash << 3 );
    hash ^= ( hash >> 11 );
    hash += ( hash << 15 );
    return hash;
}

