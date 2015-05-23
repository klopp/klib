/*
 * slist.c, part of "lists" project.
 *
 *  Created on: 21.05.2015, 04:50
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "slist.h"

#if defined(DEBUG)

SList slcreate( void )
{
    return lcreate( free );
}

#endif

char * sladd( List list, const char * data )
{
    if( list && data )
    {
        char * dup = strdup( data );
        if( dup )
        {
            char * rc = ladd( list, dup );
            if( rc ) return rc;
            free( dup );
        }
    }
    return NULL;
}

char * slpoke( List list, const char * data )
{
    if( list && data )
    {
        char * dup = strdup( data );
        if( dup )
        {
            char * rc = lpoke( list, dup );
            if( rc ) return rc;
            free( dup );
        }
    }
    return NULL;
}

