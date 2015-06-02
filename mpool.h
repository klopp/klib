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
# define MPOOL_MIN      32
#else
# define MPOOL_MIN      1024
#endif

/*
 * If mp_alloc() failed, new mpool will be added to chain with new
 * size = ([old mpool size] + [requested size]) * MP_EXPAND_FOR
 */
#define MP_EXPAND_FOR   1.5

#define MP_USE_LOCKING  0

#pragma pack(1)

typedef enum _mb_flags
{
    MB_BUSY = 0x01, MB_LOCKED = 0x02
} mb_flags;

typedef struct _mblk
{
    unsigned short signature;
    unsigned short flags;
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
    size_t id;
    size_t size;
    mp_flags flags;
    char * min;
    char * max;
    char * pool;
    struct _mpool * next;
}*mpool;

typedef void (*mp_walker)( const mpool mp, const mblk mb, void * data );

mpool mp_create( size_t size, mp_flags flags );
void mp_destroy( mpool mp );

void * mp_alloc( const mpool mp, size_t size );
void * mp_calloc( const mpool mp, size_t size, size_t n );

#if MP_USE_LOCKING
int mp_lock( const mpool mp, void * ptr );
int mp_locked( const mpool mp, void * ptr );
int mp_unlock( const mpool mp, void * ptr );
#endif

void * mp_realloc( const mpool mp, void * src, size_t size );

int mp_free( const mpool mp, void * ptr );

void mp_walk( const mpool mp, mp_walker walker, void * data );

#pragma pack()

#if defined(__cplusplus)
}
#endif

#endif /* MPOOL_H_ */
