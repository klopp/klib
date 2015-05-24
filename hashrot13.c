/*
 * hashrot13.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:27
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "hash.h"

unsigned hash_rot13( unsigned startval, const char * buf, size_t size )
{
    unsigned hash;

    for( hash = startval; size; size--, buf++ )
    {
        hash += (unsigned char)(*buf);
        hash -= (hash << 13) | (hash >> 19);
    }
    return hash;
}
