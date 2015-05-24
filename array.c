/*
 * array.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 21:43
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "array.h"

Array acreate( size_t size, A_destructor destructor )
{
    Array array = malloc( sizeof(struct _Array) );
    if( !array ) return NULL;

    array->data = calloc( sizeof(void *), size );
    if( !array->data )
    {
        free( array );
        return NULL;
    }
    array->destructor = destructor;
    array->size = size;
    return array;
}

void adestroy( Array array )
{
    if( array )
    {
        if( array->destructor )
        {
            size_t i;
            for( i = 0; i < array->size; i++ )
            {
                if( array->data[i] ) array->destructor( array->data[i] );
            }
        }
        free( array->data );
        free( array );
    }
}

void * aset( Array array, size_t idx, void * data )
{
    if( idx >= array->size )
    {
        size_t newsize = array->size * ARR_K_EXPAND;
        void * ptr = realloc( array->data, newsize );
        if( !ptr ) return NULL;
        array->data = ptr;
        array->size = newsize;
    }
    array->data[idx] = data;
    return NULL;
}

void * aget( Array array, size_t idx )
{
    return idx >= array->size ? NULL : array->data[idx];
}

int adel( Array array, size_t idx )
{
    if( idx < array->size )
    {
        if( array->data[idx] && array->destructor )
        {
            array->destructor( array->data[idx] );
        }
        array->data[idx] = NULL;
    }
    return 0;
}

void awalk( Array array, A_walk walker )
{
    if( array && walker )
    {
        size_t i;
        for( i = 0; i < array->size; i++ )
        {
            walker( i, array->data[i] );
        }
    }
}

