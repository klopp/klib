/*
 * array.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 21:43
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "array.h"

Array acreate( size_t size, A_destructor destructor )
{
    Array array = Malloc( sizeof( struct _Array ) );

    if( !array ) {
        return NULL;
    }

    array->data = Calloc( sizeof( void * ), size );

    if( !array->data ) {
        Free( array );
        return NULL;
    }

    array->destructor = destructor;
    array->size = size;
    __initlock( array->lock );
    return array;
}

void adestroy( Array array )
{
    if( array ) {
        __lock( array->lock );

        if( array->destructor ) {
            size_t i;

            for( i = 0; i < array->size; i++ ) {
                if( array->data[i] ) {
                    array->destructor( array->data[i] );
                }
            }
        }

        Free( array->data );
        __unlock( array->lock );
        Free( array );
    }
}

void *aset( Array array, size_t idx, void *data )
{
    __lock( array->lock );

    if( idx >= array->size ) {
        size_t newsize = ARR_IDX_EXPAND( idx );
        void **ptr = Calloc( newsize, sizeof( void ** ) );

        if( !ptr ) {
            __unlock( array->lock );
            return NULL;
        }

        memcpy( ptr, array->data, array->size * sizeof( void ** ) );
        Free( array->data );
        array->data = ptr;
        array->size = newsize;
    }

    array->data[idx] = data;
    __unlock( array->lock );
    return data;
}

void *aget( Array array, size_t idx )
{
    return idx >= array->size ? NULL : array->data[idx];
}

int adel( Array array, size_t idx )
{
    if( idx < array->size ) {
        __lock( array->lock );

        if( array->data[idx] && array->destructor ) {
            array->destructor( array->data[idx] );
        }

        array->data[idx] = NULL;
        __unlock( array->lock );
    }

    return 0;
}

void awalk( Array array, A_walk walker )
{
    if( array && walker ) {
        size_t i;
        __lock( array->lock );

        for( i = 0; i < array->size; i++ ) {
            walker( i, array->data[i] );
        }

        __unlock( array->lock );
    }
}

void *saset( SArray array, size_t idx, const char *data )
{
    char *scopy = Strdup( data );

    if( scopy && aset( array, idx, scopy ) ) {
        return scopy;
    }

    Free( scopy );
    return NULL;
}

static int a_compare_strings( const void *a, const void *b )
{
    const char **pa = ( const char ** )a;
    const char **pb = ( const char ** )b;

    if( !*pa && !*pb ) {
        return 0;
    }

    if( !*pa && *pb ) {
        return -1;
    }

    if( *pa && !*pb ) {
        return 1;
    }

    return strcmp( *pa, *pb );
}

SArray sasort( SArray array )
{
    qsort( array->data, array->size, sizeof( void * ), a_compare_strings );
    return array;
}
