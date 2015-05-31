/*
 * mpool.c, part of "klib" project.
 *
 *  Created on: 31.05.2015, 01:47
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "mpool.h"
#include <string.h>

#define MP_VALID( ptr, mp ) \
    ((char *)(ptr) >= (mp)->min && \
    (char *)(ptr) <= (mp)->max && \
    (((struct _mblk *)(ptr)) - 1)->signature == MBLK_SIGNATURE)

#define MB_VALID( mb, mp ) \
    ((char *)((mb)+1) >= (mp)->min && \
    (char *)((mb)+1) <= (mp)->max && \
    (mb)->signature == MBLK_SIGNATURE)

mpool mp_create( size_t size )
{
    size_t add_size = sizeof(struct _mblk);
    mpool mp = malloc( sizeof(struct _mpool) + size + add_size );
    if( !mp ) return NULL;

    mp->flags = 0;
    mp->size = size;
    mp->min = mp->pool + sizeof(struct _mblk);
    mp->max = mp->pool + size - MBLK_MIN;
    ((mblk)mp->pool)->status = 0;
    ((mblk)mp->pool)->size = size;
    ((mblk)mp->pool)->signature = MBLK_SIGNATURE;

    return mp;
}

#if defined(DEBUG)

void mp_destroy( mpool mp )
{
    free( mp );
}

#endif

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
        mb = (mblk)((char *)mb + sizeof(struct _mblk) + mb->size);
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
        next = (mblk)((char *)mb + sizeof(struct _mblk) + mb->size);
        if( !MB_VALID( next, mp ) ) break;
        if( mb->status || next->status )
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
        if( !mb->status && mb->size >= size )
        {
            if( mb->size == size )
            {
                best = mb;
                break;
            }
            if( !best )
            {
                best = mb;
                min = mb->size;
            }
            else if( mb->size < min )
            {
                best = mb;
                min = mb->size;
            }
        }
        mb = (mblk)((char *)mb + sizeof(struct _mblk) + mb->size);
    }

    if( best )
    {
        if( best->size == size )
        {
            best->status = 1;
            return best + 1;
        }
        if( best->size < size + MBLK_MIN + sizeof(struct _mblk) )
        {
            best->status = 1;
            return best + 1;
        }
        mb = (mblk)((char *)best + sizeof(struct _mblk) + size);
        mb->status = 0;
        mb->signature = MBLK_SIGNATURE;
        mb->size = best->size - size - sizeof(struct _mblk);
        best->size = size;
        best->status = 1;
        return best + 1;
    }

    return NULL;
}

void * mp_alloc( const mpool mp, size_t size )
{
    void * ptr;

    if( size < MBLK_MIN ) size = MBLK_MIN;
    size += (sizeof(int) - 1);
    size &= ~(sizeof(int) - 1);

    ptr = _mp_alloc( mp, size );
    if( !ptr && _mp_defragment( mp ) ) ptr = _mp_alloc( mp, size );
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
    if( (((struct _mblk *)ptr) - 1)->status )
    {
        (((struct _mblk *)ptr) - 1)->status = 0;
        mp->flags |= MP_DIRTY;
    }
    return 1;
}
