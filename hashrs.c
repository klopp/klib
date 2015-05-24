/*
 * hashrs.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:30
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

unsigned hash_rs( unsigned startval, const char * buf, size_t size )
{
    //static const unsigned int b = 378551;
    static unsigned a = 0;
    unsigned hash;

    if( !startval ) a = 63689;

    for( hash = startval; size; size--, buf++ )
    {
        hash = hash * a + (unsigned char)(*buf);
        a *= 378551;
    }
    return hash;
}

