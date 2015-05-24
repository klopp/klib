/*
 * hashly.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:28
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

unsigned hash_ly( unsigned startval, const char * buf, size_t size )
{
    unsigned hash;

    for( hash = startval; size; size--, buf++ )
        hash = (hash * 1664525) + (unsigned char)(*buf) + 1013904223;

    return hash;

}

