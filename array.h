/*
 * array.h, part of "klib" project.
 *
 *  Created on: 24.05.2015, 21:38
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef ARRAY_H_
#define ARRAY_H_

#include <stdlib.h>
#include <string.h>

#define ARR_K_EXPAND    1.5

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*A_destructor)( void * data );
typedef void (*A_walk)( size_t idx, void * data );

typedef struct _Array
{
    void ** data;
    A_destructor destructor;
    size_t size;
}*Array;

Array acreate( size_t size, A_destructor destructor );
void adestroy( Array array );

void * aset( Array array, size_t idx, void * data  );
void * aget( Array array, size_t idx );
int adel( Array array, size_t idx );
void awalk( Array array, A_walk walker );

#ifdef __cplusplus
extern "C"
}
#endif

#endif /* ARRAY_H_ */
