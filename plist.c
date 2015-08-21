/*
 * plist.c, part of "klib" project.
 *
 *  Created on: 20.08.2015, 16:05
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "plist.h"

void * pair_Delete( void * ptr )
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

Pair pair_Create( const char * first, const char * second )
{
    Pair pair = Calloc( sizeof(struct _Pair), 1 );
    if( pair )
    {
        if( first )
        {
            pair->first = Strdup( first );
            if( !pair->first )
            {
                return pair_Delete( pair );
            }
        }
        if( second )
        {
            pair->second = Strdup( second );
            if( !pair->second )
            {
                return pair_Delete( pair );
            }
        }
    }
    return pair;
}

PList plcreate( void )
{
    return lcreate( pair_Delete );
}

Pair pladd( List list, const char * first, const char * second )
{
    Pair pair = pair_Create( first, second );
    return pair ? ladd( list, pair ) : NULL;
}

