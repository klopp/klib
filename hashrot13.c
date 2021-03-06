/*
 * hashrot13.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:27
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "hash.h"

static unsigned hash = 0;

unsigned hash_rot13( const void *buf, size_t size )
{
    return hash_rot13_update( 0, buf, size );
}

unsigned shash_rot13( const char *buf )
{
    return shash_rot13_update( 0, buf );
}

unsigned hash_rot13_update( unsigned startval, const void *buf, size_t size )
{
    for( hash = startval; size; size-- ) {
        hash += *( ( unsigned char * ) buf );
        hash -= ( hash << 13 ) | ( hash >> 19 );
        buf = ( unsigned char * ) buf + 1;
    }

    return hash;
}

unsigned shash_rot13_update( unsigned startval, const char *buf )
{
    for( hash = startval; *buf; buf++ ) {
        hash += *( ( unsigned char * ) buf );
        hash -= ( hash << 13 ) | ( hash >> 19 );
    }

    return hash;
}
