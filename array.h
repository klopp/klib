/*
 * array.h, part of "klib" project.
 *
 *  Created on: 24.05.2015, 21:38
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef ARRAY_H_
#define ARRAY_H_

#include "config.h"
#include "_lock.h"

#define ARR_IDX_EXPAND(idx) (idx) + ((idx)/2)

#ifdef __cplusplus
extern "C"
{
#endif

typedef void ( *A_destructor )( void *data );
typedef void ( *A_walk )( size_t idx, void *data );

typedef struct _Array {
    void **data;
    A_destructor destructor;
    size_t size;
    __lock_t( lock );
} *Array;

Array acreate( size_t size, A_destructor destructor );
void adestroy( Array array );

void *aset( Array array, size_t idx, void *data );
void *aget( Array array, size_t idx );
int adel( Array array, size_t idx );
void awalk( Array array, A_walk walker );

typedef Array   SArray;
#define sacreate( size )    acreate( (size), free )

SArray sasort( SArray array );
void *saset( SArray array, size_t idx, const char *data );

#ifdef __cplusplus
    extern "C"
}
#endif

#endif /* ARRAY_H_ */
