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
#if defined(DEBUG)
# define MPOOL_MIN       32
#else
# define MPOOL_MIN       1024
#endif
#define MP_EXPAND_FOR   1.5

#pragma pack(1)

typedef struct _mblk
{
    short signature;
    short is_busy;
    size_t size;
}*mblk;

/*
 * Warning! Do not change flags manually after mp_create() call!
 */
typedef enum _mp_flags
{
    MP_DIRTY = 0x01,        // internal defrag flag
    MP_EXPAND = 0x02,       // expand mpool memory if needed
    MP_DEFAULT = (0x00)
} mp_flags;

typedef struct _mpool
{
    size_t size;
    mp_flags flags;
    char * min;
    char * max;
    char * pool;
}*mpool;

typedef void (*mp_walker)( const mblk mb, void * data );

mpool mp_create( size_t size, mp_flags flags );
void mp_destroy( mpool mp );

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
