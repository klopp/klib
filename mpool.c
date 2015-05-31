/*
 * mpool.c, part of "klib" project.
 *
 *  Created on: 31.05.2015, 01:47
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "mpool.h"
#include <string.h>

/*
 * mpool.pool structure:
 *
 * +---- struct _mblk ---+  allocated +---- struct _mblk ---+  allocated +---
 * |                     |   memory   |                     |    memory  |
 * +--------+-----+------+------------+--------+-----+------+------------+---
 * | 0x1515 | 0/1 | size | size_bytes | 0x1515 | 0/1 | size | size_bytes |
 * +--------+-----+------+------------+--------+-----+------+------------+---
 *      ^      ^     ^         ^
 *      |      |     |         +---- memory returned by mp_alloc()
 *      |      |     +---- block size
 *      |      +---- is_busy
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
    return (mblk)((char *)(mb) + sizeof(struct _mblk) + mb->size);
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

#define _mp_malloc( size ) malloc( (size) )

#endif

#define MS_ALIGN( size, min ) \
    if( (size) < (min) ) (size) = (min); \
    (size) += (sizeof(size_t) - 1); \
    (size) &= ~(sizeof(size_t) - 1)

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

    mp->id = 0;
    mp->size = size;
    mp->next = NULL;
    mp->flags = flags;
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
    if( mp->next ) mp_destroy( mp->next );
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
        walker( mp, mb, data );
        mb = MB_NEXT( mb );
    }
    if( mp->next ) mp_walk( mp->next, walker, data );
}

/*
 * Internal. Defragment mpool. Return free blocks junction count.
 */
static size_t _mp_defragment_pool( const mpool mp )
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
    if( junctions ) mp->flags &= (~MP_DIRTY);
    return junctions;
}

/*
 * Internal. Try to allocate requested block.
 */
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
        mp->flags |= MP_DIRTY;
        return best + 1;
    }

    return NULL;
}

void * mp_alloc( const mpool mp, size_t size )
{
    void * ptr;
    mpool current = mp;
    size_t largest = 0;
    size_t id = mp->id; // 0?

    MS_ALIGN( size, MBLK_MIN );
    do
    {
        id++;
        if( current->size > largest ) largest = current->size;
        ptr = _mp_alloc( current, size );
        if( !ptr && _mp_defragment_pool( current ) ) ptr = _mp_alloc( current,
                size );
        current = current->next;
    } while( !ptr && current );

    if( !ptr && (mp->flags & MP_EXPAND) == MP_EXPAND )
    {
        mpool newpool = mp_create( (largest + size) * MP_EXPAND_FOR,
                mp->flags );
        if( !newpool ) return NULL;
        newpool->id = id;

        // insert as base mpool->next
        if( mp->next ) newpool->next = mp->next;
        mp->next = newpool;
        ptr = _mp_alloc( newpool, size );
    }
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
    if( !ptr || !mp ) return 0;

    if( !MP_VALID( ptr, mp ) ) return mp_free( mp->next, ptr );

    if( (((struct _mblk *)ptr) - 1)->is_busy )
    {
        (((struct _mblk *)ptr) - 1)->is_busy = 0;
        mp->flags |= MP_DIRTY;
        return 1;
    }
    return 0;
}
