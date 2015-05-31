/*
 * mpool.h, part of "klib" project.
 *
 *  Created on: 31.05.2015, 01:42
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef MPOOL_H_
#define MPOOL_H_

#if defined(__cplusplus)
extern "C"
{
#endif

#include <stdlib.h>

#define MBLK_SIGNATURE  0x1515
#define MBLK_MIN        (sizeof(struct _mblk))

#pragma pack(1)

typedef struct _mblk
{
    short signature;
    short status;
    size_t size;
}*mblk;

typedef enum _mp_flags
{
    MP_DIRTY = 0x01
} mp_flags;

typedef struct _mpool
{
    size_t size;
    mp_flags flags;
    char * min;
    char * max;
    char pool[sizeof(int)];
}*mpool;

typedef void (*mp_walker)( const mblk mb, void * data );

mpool mp_create( size_t size );
#if defined(DEBUG)
void mp_destroy( mpool mp );
#else
#define mp_destroy( mp ) free( (mp) )
#endif

void * mp_alloc( const mpool mp, size_t size );
void * mp_calloc( const mpool mp, size_t size, size_t n );
void * mp_realloc( const mpool mp, void * src, size_t size );
int mp_free( const mpool mp, void * ptr );

void mp_walk( const mpool mp, mp_walker walker, void * data );

#pragma pack()

#if defined(__cplusplus)
}
#endif

#endif /* MPOOL_H_ */
