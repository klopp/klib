/*
 * hashfaq6.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:25
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "hash.h"

unsigned hash_faq6( unsigned startval, const void * buf, size_t size )
{
    unsigned hash;

    for( hash = startval; size; (unsigned char *)buf++, size-- )
    {
        hash += *((unsigned char *)buf);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

unsigned shash_faq6( unsigned startval, const char * buf )
{
    unsigned hash;

    for( hash = startval; *buf; buf++ )
    {
        hash += *((unsigned char *)buf);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

