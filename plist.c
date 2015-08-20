/*
 * plist.c, part of "klib" project.
 *
 *  Created on: 20.08.2015, 16:05
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "plist.h"

void * delPair( void * ptr )
{
    if( ptr )
    {
        Pair pair = (Pair)ptr;
        Free( pair->first );
        Free( pair->second );
        Free( pair );
    }
    return NULL;
}

PList plcreate( void )
{
    return lcreate( delPair );
}

Pair pladd( List list, const char * first, const char * second )
{
    Pair pair = Calloc( sizeof(struct _Pair), 1 );
    if( pair )
    {
        if( first )
        {
            pair->first = Strdup( first );
            if( !pair->first )
            {
                return delPair( pair );
            }
        }
        if( second )
        {
            pair->second = Strdup( second );
            if( !pair->second )
            {
                return delPair( pair );
            }
        }
    }
    return pair;
}

