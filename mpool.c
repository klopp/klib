/*
 * mpool.c, part of "klib" project.
 *
 *  Created on: 31.05.2015, 01:47
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "mpool.h"
#include <string.h>

/* ---------------------------------------------------------------------------*/

/*
 * mpool.pool structure:
 *
 * +---- struct _mblk ---+  allocated +---- struct _mblk ---+  allocated +---
 * |                     |   memory   |                     |    memory  |
 * +--------+-----+------+------------+--------+-----+------+------------+---
 * | 0x1515 | 0x0 | size | size_bytes | 0x1515 | 0x0 | size | size_bytes |
 * +--------+-----+------+------------+--------+-----+------+------------+---
 *      ^      ^     ^         ^
 *      |      |     |         +---- memory returned by mp_alloc()
 *      |      |     +---- block size
 *      |      +---- flags
 *      +---- block signature
 */

#if defined(DEBUG)

static void *_mp_malloc( size_t size )
{
    void *ptr = malloc( size );

    if( ptr ) {
        memset( ptr, '#', size );
    }

    return ptr;
}

static int MP_VALID( void *ptr, mpool mp )
{
    if( ( char * )( ptr ) < mp->min || ( char * )( ptr ) > mp->max ) {
        return 0;
    }

    return ( ( ( ( struct _mblk * )( ptr ) ) - 1 )->signature == MBLK_SIGNATURE );
}

static int MB_VALID( mblk mb, mpool mp )
{
    if( ( char * )( mb + 1 ) < mp->min || ( char * )( mb + 1 ) > mp->max ) {
        return 0;
    }

    return ( mb->signature == MBLK_SIGNATURE );
}

static mblk MB_NEXT( mblk mb )
{
    return ( mblk )( ( char * )( mb ) + sizeof( struct _mblk ) + mb->size );
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

#define MP_SWAP( tmp, a, b ) \
    (tmp) = (a); \
    (a) = (b); \
    (b) = (tmp)

static mpool _mp = NULL;
static int _mp_atexit = 0;
static void _mp_destroy( void )
{
    mp_destroy( _mp );
}

#define MP_SET( mp ) \
    if( !mp ) { \
    if( !_mp ) _mp = mp_create( 0, MPF_EXPAND ); \
    mp = _mp; }
/*
 * Recursive check for all mpools in chain:
 */
static int _mp_valid_ptr( void *ptr, const mpool mp )
{
    if( !mp || !ptr ) {
        return 0;
    }

    return MP_VALID( ptr, mp ) ? 1 : _mp_valid_ptr( ptr, mp->next );
}

mpool mp_create( size_t size, mp_flags flags )
{
    mpool mp;
    size = ( size ? size : MPOOL_MIN );
    MS_ALIGN( size, MPOOL_MIN );

    if( ( flags & MPF_EXPAND ) != MPF_EXPAND ) {
        mp = _mp_malloc( sizeof( struct _mpool ) + size + sizeof( struct _mblk ) );

        if( !mp ) {
            return NULL;
        }

        mp->pool = ( char * )( mp + 1 );
    }
    else {
        mp = _mp_malloc( sizeof( struct _mpool ) );

        if( !mp ) {
            return NULL;
        }

        mp->pool = _mp_malloc( size + sizeof( struct _mblk ) );

        if( !mp->pool ) {
            free( mp );
            return NULL;
        }
    }

    if( !_mp_atexit ) {
        _mp_atexit = 1;
        atexit( _mp_destroy );
    }

#ifndef __WINDOWS__
    pthread_spin_init( &mp->lock, 0 );
#else
    mp->lock = 0;
#endif
    mp->id = 0;
    mp->size = size;
    mp->next = NULL;
    mp->flags = flags;
    mp->min = mp->pool + sizeof( struct _mblk );
    mp->max = mp->pool + size - MBLK_MIN;
    mp->last = ( struct _mblk * ) mp->pool;
    mp_clear( mp );
    return mp;
}

void mp_clear( mpool mp )
{
    MP_SET( mp );

    if( mp->next ) {
        mp_clear( mp->next );
    }

    memset( ( ( mblk ) mp->pool ), 0, mp->size );
    ( ( mblk ) mp->pool )->flags = 0;
    ( ( mblk ) mp->pool )->size = mp->size;
    ( ( mblk ) mp->pool )->signature = MBLK_SIGNATURE;
}

void mp_destroy( mpool mp )
{
    if( mp ) {
        /*mp_clear( mp );*/
        if( ( mp->flags & MPF_EXPAND ) == MPF_EXPAND ) {
            free( mp->pool );
        }

        if( mp->next ) {
            mp_destroy( mp->next );
        }

#ifndef __WINDOWS__
        //pthread_mutex_destroy( &mp->lock );
#else
        mp->lock = 0;
#endif
        free( mp );
    }
}

void *mp_calloc( mpool mp, size_t size, size_t n )
{
    void *ptr;
    MP_SET( mp );
    size *= n;
    MS_ALIGN( size, MBLK_MIN );
    ptr = mp_alloc( mp, size );

    if( ptr ) {
        memset( ptr, 0, size );
    }

    return ptr;
}

char *mp_strdup( mpool mp, const char *src )
{
    size_t size = strlen( src ) + 1;
    char *ptr = mp_alloc( mp, size );

    if( ptr ) {
        memcpy( ptr, src, size );
    }

    return ptr;
}

void mp_walk( mpool mp, mp_walker walker, void *data )
{
    MP_SET( mp );

    if( mp ) {
        mblk mb = ( mblk ) mp->pool;

        while( MB_VALID( mb, mp ) ) {
            walker( mp, mb, data );
            mb = MB_NEXT( mb );
        }

        if( mp->next ) {
            mp_walk( mp->next, walker, data );
        }
    }
}

/*
 * Internal. Defragment mpool. Return free blocks junction count.
 */
static size_t _mp_defragment_pool( const mpool mp )
{
    mblk mb;
    mblk next;
    size_t junctions;

    if( !mp || ( mp->flags & MPF_DIRTY ) != MPF_DIRTY ) {
        return 0;
    }

    junctions = 0;
    mb = ( mblk ) mp->pool;

    while( MB_VALID( mb, mp ) ) {
        next = MB_NEXT( mb );

        if( !MB_VALID( next, mp ) ) {
            break;
        }

        if( ( mb->flags & MBF_BUSY ) || ( next->flags & MBF_BUSY ) ) {
            mb = next;
            continue;
        }

        mb->size += next->size + sizeof( struct _mblk );
        junctions++;
    }

    if( junctions ) {
        mp->flags &= ( ~MPF_DIRTY );
    }

    return junctions;
}

/*
 * Internal. Try to allocate requested block.
 */
static void *_mp_alloc( const mpool mp, size_t size )
{
    mblk best = NULL;
    mblk mb;
    size_t min;
    mb = ( mblk ) mp->pool;
    min = mp->size;

    if( ( mp->flags & MPF_FAST ) && MB_VALID( mp->last, mp ) ) {
        if( !( mp->last->flags & MBF_BUSY ) && mp->last->size >= size ) {
            best = mp->last;
        }
    }

    if( !best ) {
        while( MB_VALID( mb, mp ) ) {
            if( !( mb->flags & MBF_BUSY ) && mb->size >= size ) {
                if( ( mb->size == size ) || ( mp->flags & MPF_FAST ) ) {
                    best = mb;
                    break;
                }

                if( !best || mb->size < min ) {
                    best = mb;
                    min = mb->size;
                }
            }

            mb = MB_NEXT( mb );
        }
    }

    if( best ) {
        if( best->size > size + MBLK_MIN + sizeof( struct _mblk ) ) {
            /* split block */
            mb = ( mblk )( ( char * ) best + sizeof( struct _mblk ) + size );
            mb->signature = MBLK_SIGNATURE;
            mb->flags = 0;
            mb->size = best->size - size - sizeof( struct _mblk );
            mp->last = mb;
            best->size = size;
        }

        best->flags = MBF_BUSY;
        mp->flags |= MPF_DIRTY;
        return best + 1;
    }

    return NULL;
}

void *mp_alloc( mpool mp, size_t size )
{
    void *ptr;
    mpool current_pool;
    size_t largest_pool_size;
    size_t workhorse;
    MP_SET( mp );
    _mp_lock( mp );
    largest_pool_size = mp->size;
    workhorse = 0;
    MS_ALIGN( size, MBLK_MIN );
    current_pool = mp;

    do {
        workhorse++;
        ptr = _mp_alloc( current_pool, size );

        if( !ptr ) {
            if( ( !( mp->flags & MPF_FAST ) && _mp_defragment_pool( current_pool ) ) ) {
                ptr = _mp_alloc( current_pool, size );
            }
        }

        current_pool = current_pool->next;
    }
    while( !ptr && current_pool );

    if( !ptr && ( mp->flags & MPF_EXPAND ) == MPF_EXPAND ) {
        mpool newpool = mp_create( MP_EXPAND_FOR( ( largest_pool_size + size ) ),
                                   mp->flags );

        if( !newpool ) {
            _mp_unlock( mp );
            return NULL;
        }

        if( mp->next ) {
            newpool->next = mp->next;
        }

        mp->next = newpool;

        if( mp->flags & MPF_FAST ) {
            _mp_defragment_pool( mp );
        }

        /*
         * swap base pool and newpool (set new pool first)
         */
        newpool->id = mp->id;
        mp->id = workhorse;
        MP_SWAP( workhorse, mp->size, newpool->size );
        MP_SWAP( ptr, mp->min, newpool->min );
        MP_SWAP( ptr, mp->max, newpool->max );
        MP_SWAP( ptr, mp->pool, newpool->pool );
        ptr = _mp_alloc( mp, size );
    }

    _mp_unlock( mp );
    return ptr;
}

int mp_lock( mpool mp, void *ptr )
{
    MP_SET( mp );

    if( _mp_valid_ptr( ptr, mp ) ) {
        if( ( ( ( struct _mblk * ) ptr ) - 1 )->flags & MBF_BUSY ) {
            ( ( ( struct _mblk * ) ptr ) - 1 )->flags |= MBF_LOCKED;
            return 1;
        }
    }

    return 0;
}

int mp_unlock( mpool mp, void *ptr )
{
    MP_SET( mp );

    if( _mp_valid_ptr( ptr, mp ) ) {
        if( ( ( ( struct _mblk * ) ptr ) - 1 )->flags & MBF_LOCKED ) {
            ( ( ( struct _mblk * ) ptr ) - 1 )->flags &= ~( MBF_LOCKED );
            return 1;
        }
    }

    return 0;
}

int mp_locked( mpool mp, void *ptr )
{
    MP_SET( mp );
    return _mp_valid_ptr( ptr, mp ) ?
           ( ( ( ( struct _mblk * ) ptr ) - 1 )->flags & MBF_LOCKED ) : 0;
}

void *mp_realloc( mpool mp, void *src, size_t size )
{
    void *dest;
    MP_SET( mp );
    dest = NULL;

    if( _mp_valid_ptr( src, mp ) ) {
        if( !( ( ( ( struct _mblk * ) src ) - 1 )->flags & MBF_LOCKED ) ) {
            dest = mp_alloc( mp, size );

            if( dest ) {
                size_t tomove = ( ( ( struct _mblk * ) src ) - 1 )->size;

                if( tomove > size ) {
                    tomove = size;
                }

                memcpy( dest, src, tomove );
            }
        }
    }

    _mp_unlock( mp );

    if( dest ) {
        mp_free( mp, src );
    }

    return dest;
}

int mp_free( mpool mp, void *ptr )
{
    mpool locked;
    MP_SET( mp );
    _mp_lock( mp );
    locked = mp;

    if( !ptr ) {
        _mp_unlock( locked );
        return 0;
    }

    while( mp ) {
        if( !MP_VALID( ptr, mp ) ) {
            break;
        }

        mp = mp->next;
    }

    if( !mp ) {
        _mp_unlock( locked );
        return 0;
    }

    if( ( ( ( struct _mblk * ) ptr ) - 1 )->flags & MBF_LOCKED ) {
        _mp_unlock( locked );
        return 0;
    }

    if( ( ( ( struct _mblk * ) ptr ) - 1 )->flags & MBF_BUSY ) {
        mp->flags |= MPF_DIRTY;
    }

    ( ( ( struct _mblk * ) ptr ) - 1 )->flags = 0;
    _mp_unlock( locked );
    return 1;
}

static char *_mp_format_size( unsigned long size, char *bsz )
{
    if( size < 1024 ) {
        sprintf( bsz, "%luB", size );
    }
    else if( size < 1024 * 1024 ) {
        if( size % 1024 ) {
            sprintf( bsz, "%lu.%luKb", size / 1024, size % 1024 );
        }
        else {
            sprintf( bsz, "%luKb", size / 1024 );
        }
    }
    else {
        if( size % ( 1024 * 1024 ) ) {
            char *ptr;
            int  i = 4;
            sprintf( bsz, "%lu.%lu", size / ( 1024 * 1024 ),
                     size % ( 1024 * 1024 ) );
            ptr = strchr( bsz, '.' );

            while( *ptr++ && --i ) {
            }

            strcpy( ptr, "Mb" );
        }
        else {
            sprintf( bsz, "%luMb", size / ( 1024 * 1024 ) );
        }
    }

    return bsz;
}

void mp_dump( mpool mp, FILE *fout, size_t maxw )
{
    unsigned long mp_total = 0;
    unsigned long mp_blocks = 0;
    unsigned long mp_largest = 0;
    size_t mp_total_free = 0;
    size_t mp_largest_free = 0;
    size_t mp_pools = 0;
    mpool current;
    char bsz[32];
    char *outbuf = malloc( maxw + 32 );

    if( !outbuf ) {
        return;
    }

    MP_SET( mp );
    current = mp;

    while( current ) {
        size_t largest = 0;
        size_t mb_total = 0;
        size_t onew;
        mblk mb;
        mp_pools++;
        mp_total += current->size;

        if( mp_largest < current->size ) {
            mp_largest = current->size;
        }

        mb = ( mblk ) current->pool;

        while( MB_VALID( mb, current ) ) {
            mp_blocks++;
            mb_total++;

            if( largest < mb->size ) {
                largest = mb->size;
            }

            mb = MB_NEXT( mb );
        }

        onew = largest / maxw;
#ifndef __WINDOWS__
        fprintf( fout, "ID: %zu, size: %s, ", current->id,
                 _mp_format_size( current->size, bsz ) );
        fprintf( fout, "blocks: %zu, internal: %s\n", mb_total,
                 _mp_format_size( mb_total * sizeof( struct _mblk ) , bsz ) );
#else
        fprintf( fout, "ID: %u, size: %s, ", current->id,
                 _mp_format_size( current->size, bsz ) );
        fprintf( fout, "blocks: %u, internal: %s\n", mb_total,
                 _mp_format_size( mb_total * sizeof( struct _mblk ) ), bsz );
#endif
        mb = ( mblk ) current->pool;

        while( MB_VALID( mb, current ) ) {
            size_t w;
            char c = '.';

            if( mb->flags & MBF_BUSY ) {
                c = '*';

                if( mb->flags & MBF_LOCKED ) {
                    c = '#';
                }
            }
            else {
                mp_total_free += mb->size;

                if( mp_largest_free < mb->size ) {
                    mp_largest_free = mb->size;
                }
            }

            memset( outbuf, 0, maxw + 32 );
            sprintf( outbuf, "%10s [", _mp_format_size( mb->size, bsz ) );
            w = mb->size / onew;

            if( w > maxw ) {
                w = maxw;
            }

            if( !w ) {
                w = 1;
            }

            memset( outbuf + strlen( outbuf ), c, w );
            fprintf( fout, "%s]\n", outbuf );
            mb = MB_NEXT( mb );
        }

        fprintf( fout, "\n" );
        current = current->next;
    }

    fprintf( fout, "%-16s: [.] - free, [*] - busy, [#] - locked\n", "Legend" );

    if( mp_pools > 1 ) {
#ifndef __WINDOWS__
        fprintf( fout, "%-16s: %zu\n", "Total pools", mp_pools );
#else
        fprintf( fout, "%-16s: %u\n", "Total pools", mp_pools );
#endif
        fprintf( fout, "%-16s: %s\n", "Largest pool",
                 _mp_format_size( mp_largest, bsz ) );
    }

    fprintf( fout, "%-16s: %s\n", "Total allocated", _mp_format_size( mp_total,
             bsz ) );
    fprintf( fout, "%-16s: %s\n", "Total free", _mp_format_size( mp_total_free,
             bsz ) );
    fprintf( fout, "%-16s: %lu\n", "Total blocks", mp_blocks );
    fprintf( fout, "%-16s: %s\n", "Largest free",
             _mp_format_size( mp_largest_free, bsz ) );
    fprintf( fout, "%-16s: %s\n", "Internal memory",
             _mp_format_size(
                 ( mp_blocks * sizeof( struct _mblk ) )
                 + ( mp_pools * sizeof( struct _mpool ) ), bsz ) );
    free( outbuf );
}
