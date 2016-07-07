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

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MBLK_SIGNATURE  0x1515
#define MBLK_MIN        (sizeof(struct _mblk))
#if defined(DEBUG)
# define MPOOL_MIN      (1024*64)
#else
# define MPOOL_MIN      (1024*1024*16)
#endif

/*
 * If mp_alloc() failed, new mpool will be added to chain with new
 * size = ([old mpool size] + [requested size]) * MP_EXPAND_FOR
 */
#define MP_EXPAND_FOR(sz)   ((sz) + ((sz) / 2))

#pragma pack(1)

typedef enum _mb_flags
{
    MBF_BUSY = 0x01, MBF_LOCKED = 0x02
}
mb_flags;

typedef struct _mblk {
    unsigned short signature;
    unsigned short flags;
    size_t size;
} *mblk;

/*
 * Warning! Do not change flags manually after mp_create() call!
 */
typedef enum _mp_flags {
    MPF_DIRTY = 0x01,        /* internal defrag flag */
    MPF_EXPAND = 0x02,       /* expand mpool memory if needed */
    MPF_FAST = 0x04,         /* do not search best free block */
    MPF_DEFAULT = ( 0x00 )
} mp_flags;

typedef struct _mpool {
    size_t id;
    size_t size;
    mp_flags flags;
    struct _mblk *lastfree;
    char *min;
    char *max;
    char *pool;
    struct _mpool *next;
} *mpool;

typedef void ( *mp_walker )( const mpool mp, const mblk mb, void *data );

mpool mp_create( size_t size, mp_flags flags );
void mp_clear( mpool mp );
void mp_destroy( mpool mp );

void mp_clear( mpool mp );
#define m_release()             mp_release( NULL )

void *mp_alloc( mpool mp, size_t size );
void *mp_calloc( mpool mp, size_t size, size_t n );
char *mp_strdup( mpool mp, const char *src );
void *mp_realloc( mpool mp, void *src, size_t size );
#define m_alloc(size)           mp_alloc( NULL, (size) )
#define m_calloc(size, n)       mp_calloc( NULL, (size), (n) )
#define m_strdup(src)           mp_strdup( NULL, (src) )
#define m_realloc(src, size)    mp_realloc( NULL, (src), (size) )

int mp_lock( mpool mp, void *ptr );
int mp_locked( mpool mp, void *ptr );
int mp_unlock( mpool mp, void *ptr );
#define m_lock(ptr)             mp_lock( NULL, (ptr) )
#define m_locked(ptr)           mp_locked( NULL, (ptr) )
#define m_unlock(ptr)           mp_unlock( NULL, (ptr) )

int mp_free( mpool mp, void *ptr );
#define m_free(ptr)             mp_free( NULL, (ptr) )

void mp_walk( mpool mp, mp_walker walker, void *data );
void mp_dump( mpool, FILE *, size_t );
#define m_walk(walker, data)    mp_walk( NULL, (walker), (data) )
#define m_dump(file, width)     mp_dump( NULL, (file), (width) )

#pragma pack()

#if defined(__cplusplus)
}
#endif

#endif /* MPOOL_H_ */
