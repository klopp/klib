/*
 * mpool.c, part of "klib" project.
 *
 *  Created on: 31.05.2015, 01:47
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "mpool.h"
#include <string.h>
#include <stdio.h>

/*
 * mpool.pool structure:
 *
 * +---- struct _mblk ---+  allocated +---- struct _mblk ---+  allocated +---
 * |                     |   memory   |                     |    memory  |
 * +--------+-----+------+------------+--------+-----+------+------------+---
 * | 0x1515 | 0/1 | size | size_bytes | 0x1515 | 0/1 | size | size_bytes |
 * +--------+-----+------+------------+--------+-----+------+------------+---
 *      |      |     |         |
 *      |      |     |         +---- memory returned by mp_alloc()
 *      |      |     +---- block size
 *      |      +---- free/busy
 *      +---- block signature
 */

#if defined(DEBUG)

static void * _mp_malloc( size_t size )
{
    void * ptr = malloc( size );
    if( ptr ) memset( ptr, 0x99, size );
    return ptr;
}

static int MP_VALID( void * ptr, mpool mp )
{
    if( (char *)(ptr) < mp->min ) return 0;
    if( (char *)(ptr) > mp->max ) return 0;
    return ((((struct _mblk *)(ptr)) - 1)->signature == MBLK_SIGNATURE);
}

static int MB_VALID( mblk mb, mpool mp )
{
    if( (char *)((mb) + 1) < mp->min ) return 0;
    if( (char *)((mb) + 1) > mp->max ) return 0;
    return (mb->signature == MBLK_SIGNATURE);
}

static mblk MB_NEXT( mblk mb )
{
    return (mblk)((char *)(mb) + sizeof(struct _mblk) + (mb)->size);
}

#else
#define MP_VALID( ptr, mp ) \
    ((char *)(ptr) >= (mp)->min && \
    (char *)(ptr) <= (mp)->max && \
    (((struct _mblk *)(ptr)) - 1)->signature == MBLK_SIGNATURE)

#define MB_VALID( mb, mp ) \
    ((char *)((mb)+1) >= (mp)->min && \
    (char *)((mb)+1) <= (mp)->max && \
    (mb)->signature == MBLK_SIGNATURE)

#define MB_NEXT( mb ) \
        (mblk)((char *)(mb) + sizeof(struct _mblk) + (mb)->size)

#define _mp_malloc( size ) malloc( (size) );

#endif

#define MS_ALIGN( size, min ) \
    if( (size) < (min) ) (size) = (min); \
    (size) += (sizeof(size_t) - 1); \
    (size) &= ~(sizeof(size_t) - 1);

mpool mp_create( size_t size, mp_flags flags )
{
    mpool mp;

    MS_ALIGN( size, MPOOL_MIN );

    if( (flags & MP_EXPAND) != MP_EXPAND )
    {
        mp = _mp_malloc( sizeof(struct _mpool) + size + sizeof(struct _mblk) );
        if( !mp ) return NULL;
        mp->pool = (char *)(mp + 1);
    }
    else
    {
        mp = _mp_malloc( sizeof(struct _mpool) );
        if( !mp ) return NULL;
        mp->pool = _mp_malloc( size + sizeof(struct _mblk) );
        if( !mp->pool )
        {
            free( mp );
            return NULL;
        }
    }

    mp->flags = flags;
    mp->size = size;
    mp->min = mp->pool + sizeof(struct _mblk);
    mp->max = mp->pool + size - MBLK_MIN;
    ((mblk)mp->pool)->is_busy = 0;
    ((mblk)mp->pool)->size = size;
    ((mblk)mp->pool)->signature = MBLK_SIGNATURE;

    return mp;
}

void mp_destroy( mpool mp )
{
    if( (mp->flags & MP_EXPAND) == MP_EXPAND )
    {
        free( mp->pool );
    }
    free( mp );
}

void * mp_calloc( const mpool mp, size_t size, size_t n )
{
    void * ptr = mp_alloc( mp, size );
    if( ptr ) memset( ptr, 0, size );
    return ptr;
}

void mp_walk( const mpool mp, mp_walker walker, void * data )
{
    mblk mb = (mblk)mp->pool;

    while( MB_VALID( mb, mp ) )
    {
        walker( mb, data );
        mb = MB_NEXT( mb );
    }
}

static size_t _mp_defragment( const mpool mp )
{
    mblk mb;
    mblk next;
    size_t junctions;

    if( (mp->flags & MP_DIRTY) != MP_DIRTY ) return 0;

    junctions = 0;
    mb = (mblk)mp->pool;

    while( MB_VALID( mb, mp ) )
    {
        next = MB_NEXT( mb );
        if( !MB_VALID( next, mp ) ) break;
        if( mb->is_busy || next->is_busy )
        {
            mb = next;
            continue;
        }
        mb->size += next->size + sizeof(struct _mblk);
        junctions++;
    }
    mp->flags &= (~MP_DIRTY);
    return junctions;
}

static void * _mp_alloc( const mpool mp, size_t size )
{
    mblk best = NULL;
    mblk mb = (mblk)mp->pool;
    size_t min = mp->size;

    while( MB_VALID( mb, mp ) )
    {
        if( !mb->is_busy && mb->size >= size )
        {
            if( mb->size == size )
            {
                best = mb;
                break;
            }
            if( !best || mb->size < min )
            {
                best = mb;
                min = mb->size;
            }
        }
        mb = MB_NEXT( mb );
    }

    if( best )
    {
        if( best->size > size + MBLK_MIN + sizeof(struct _mblk) )
        {
            // split block
            mb = (mblk)((char *)best + sizeof(struct _mblk) + size);
            mb->is_busy = 0;
            mb->signature = MBLK_SIGNATURE;
            mb->size = best->size - size - sizeof(struct _mblk);
            best->size = size;
        }
        best->is_busy = 1;
        return best + 1;
    }

    return NULL;
}

void * mp_alloc( const mpool mp, size_t size )
{
    void * ptr;

    MS_ALIGN( size, MBLK_MIN );
    ptr = _mp_alloc( mp, size );
    if( !ptr && _mp_defragment( mp ) ) ptr = _mp_alloc( mp, size );

    if( !ptr && (mp->flags & MP_EXPAND) == MP_EXPAND )
    {
        mblk mb;
        mblk last = NULL;
        size_t left;
        char * newpool;
        char * oldpool = mp->pool;
        size_t oldsize = mp->size;
        size_t newsize = (mp->size + size) * MP_EXPAND_FOR;

        MS_ALIGN( newsize, MPOOL_MIN );
        left = newsize + sizeof(struct _mblk);
        newpool = _mp_malloc( left );
        if( !newpool ) return NULL;

        memcpy( newpool, mp->pool, mp->size + sizeof(struct _mblk) );
        mp->size = newsize;
        mp->pool = newpool;
        mp->min = mp->pool + sizeof(struct _mblk);
        mp->max = mp->pool + newsize - MBLK_MIN;

        mb = (mblk)mp->pool;
        while( MB_VALID( mb, mp ) )
        {
            left -= mb->size + sizeof(struct _mblk);
            last = mb;
            mb = MB_NEXT( mb );
        }

        if( !last )
        {
            mp->pool = oldpool;
            mp->size = oldsize;
            free( newpool );
            return NULL;
        }

        free( oldpool );
        if( last->is_busy )
        {
            mb = MB_NEXT( last );
            mb->signature = MBLK_SIGNATURE;
            mb->is_busy = 0;
            mb->size = left - sizeof(struct _mblk);
        }
        else
        {
            last->size += left - sizeof(struct _mblk);
        }
        ptr = _mp_alloc( mp, size );
    }

    if( ptr ) mp->flags |= MP_DIRTY;
    return ptr;
}

void * mp_realloc( const mpool mp, void * src, size_t size )
{
    void * dest = mp_alloc( mp, size );

    if( dest )
    {
        memcpy( dest, src, (((struct _mblk *)src) - 1)->size );
        mp_free( mp, src );
    }
    return dest;
}

int mp_free( const mpool mp, void * ptr )
{
    if( !ptr || !mp || !MP_VALID( ptr, mp ) ) return 0;
    if( (((struct _mblk *)ptr) - 1)->is_busy )
    {
        (((struct _mblk *)ptr) - 1)->is_busy = 0;
        mp->flags |= MP_DIRTY;
    }
    return 1;
}
