/*
 * hashfaq6.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:25
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

unsigned hash_faq6( unsigned startval, const char * buf, size_t size )
{
    unsigned hash;

    for( hash = startval; size; buf++, size-- )
    {
        hash += (unsigned)(*buf);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

