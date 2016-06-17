/*
 * slist.c, part of "lists" project.
 *
 *  Created on: 21.05.2015, 04:50
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "slist.h"

static void _sl_destroy( void *data ) {
    Free( data );
}

SList slcreate( void ) {
    return lcreate( _sl_destroy );
}

char *sladd( List list, const char *data ) {
    if( list && data ) {
        char *dup = Strdup( data );
        if( dup ) {
            char *rc = ladd( list, dup );
            if( rc ) {
                return rc;
            }
            Free( dup );
        }
    }
    return NULL;
}

char *slpoke( List list, const char *data ) {
    if( list && data ) {
        char *dup = Strdup( data );
        if( dup ) {
            char *rc = lpoke( list, dup );
            if( rc ) {
                return rc;
            }
            Free( dup );
        }
    }
    return NULL;
}

