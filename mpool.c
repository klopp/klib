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
 * | 0x1515 | 0x0 | size | size_bytes | 0x1515 | 0x0 | size | size_bytes |
 * +--------+-----+------+------------+--------+-----+------+------------+---
 *      ^      ^     ^         ^
 *      |      |     |         +---- memory returned by mp_alloc()
 *      |      |     +---- block size
 *      |      +---- flags
 *      +---- block signature
 */

#if defined(DEBUG)

static void *_mp_malloc(size_t size) {
    void *ptr = malloc(size);
    if(ptr) {
        memset(ptr, 0x69, size);
    }
    return ptr;
}

static int MP_VALID(void *ptr, mpool mp) {
    if((char *)(ptr) < mp->min || (char *)(ptr) > mp->max) {
        return 0;
    }
    return ((((struct _mblk *)(ptr)) - 1)->signature == MBLK_SIGNATURE);
}

static int MB_VALID(mblk mb, mpool mp) {
    if((char *)(mb + 1) < mp->min || (char *)(mb + 1) > mp->max) {
        return 0;
    }
    return (mb->signature == MBLK_SIGNATURE);
}

static mblk MB_NEXT(mblk mb) {
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

#define SWAP( tmp, a, b ) \
    (tmp) = (a); \
    (a) = (b); \
    (b) = (tmp)

static mpool _mp = NULL;
static int _mp_atexit = 0;
static void _mp_destroy(void) {
    mp_destroy(_mp);
}

#define MP_SET( mp ) \
    if( !mp ) { \
    if( !_mp ) _mp = mp_create( 0, MP_EXPAND ); \
    mp = _mp; }

/*
 * Recursive check for all mpools in chain:
 */
static int _mp_valid_ptr(void *ptr, const mpool mp) {
    if(!mp || !ptr) {
        return 0;
    }
    return MP_VALID(ptr, mp) ? 1 : _mp_valid_ptr(ptr, mp->next);
}

mpool mp_create(size_t size, mp_flags flags) {
    mpool mp;
    size = (size ? size : MPOOL_MIN);
    MS_ALIGN(size, MPOOL_MIN);
    if((flags & MP_EXPAND) != MP_EXPAND) {
        mp = _mp_malloc(sizeof(struct _mpool) + size + sizeof(struct _mblk));
        if(!mp) {
            return NULL;
        }
        mp->pool = (char *)(mp + 1);
    } else {
        mp = _mp_malloc(sizeof(struct _mpool));
        if(!mp) {
            return NULL;
        }
        mp->pool = _mp_malloc(size + sizeof(struct _mblk));
        if(!mp->pool) {
            free(mp);
            return NULL;
        }
    }

    if(!_mp_atexit) {
        _mp_atexit = 1;
        atexit(_mp_destroy);
    }

    mp->id = 0;
    mp->size = size;
    mp->next = NULL;
    mp->flags = flags;
    mp->min = mp->pool + sizeof(struct _mblk);
    mp->max = mp->pool + size - MBLK_MIN;

    mp_release( mp );
    return mp;
}

void mp_release(mpool mp)
{
    memset( ((mblk)mp->pool), 0, mp->size );
    ((mblk)mp->pool)->flags = 0;
    ((mblk)mp->pool)->size = mp->size;
    ((mblk)mp->pool)->signature = MBLK_SIGNATURE;
}

void mp_destroy(mpool mp) {
    if(mp) {
        if((mp->flags & MP_EXPAND) == MP_EXPAND) {
            free(mp->pool);
        }
        if(mp->next) {
            mp_destroy(mp->next);
        }
        free(mp);
    }
}

void *mp_calloc(mpool mp, size_t size, size_t n) {
    void *ptr;
    MP_SET(mp);
    size *= n;
    MS_ALIGN(size, MBLK_MIN);
    ptr = mp_alloc(mp, size);
    if(ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

char *mp_strdup(mpool mp, const char *src) {
    size_t size = strlen(src) + 1;
    char *ptr = mp_alloc(mp, size);
    if(ptr) {
        memcpy(ptr, src, size);
    }
    return ptr;
}

void mp_walk(mpool mp, mp_walker walker, void *data) {
    MP_SET(mp);
    if(mp) {
        mblk mb = (mblk)mp->pool;
        while(MB_VALID(mb, mp)) {
            walker(mp, mb, data);
            mb = MB_NEXT(mb);
        }
        if(mp->next) {
            mp_walk(mp->next, walker, data);
        }
    }
}

/*
 * Internal. Defragment mpool. Return free blocks junction count.
 */
static size_t _mp_defragment_pool(const mpool mp) {
    mblk mb;
    mblk next;
    size_t junctions;
    if(!mp || (mp->flags & MP_DIRTY) != MP_DIRTY) {
        return 0;
    }
    junctions = 0;
    mb = (mblk)mp->pool;
    while(MB_VALID(mb, mp)) {
        next = MB_NEXT(mb);
        if(!MB_VALID(next, mp)) {
            break;
        }
        if((mb->flags & MB_BUSY) || (next->flags & MB_BUSY)) {
            mb = next;
            continue;
        }
        mb->size += next->size + sizeof(struct _mblk);
        junctions++;
    }
    if(junctions) {
        mp->flags &= (~MP_DIRTY);
    }
    return junctions;
}

/*
 * Internal. Try to allocate requested block.
 */
static void *_mp_alloc(const mpool mp, size_t size) {
    mblk best = NULL;
    mblk mb;
    size_t min;
    mb = (mblk)mp->pool;
    min = mp->size;
    while(MB_VALID(mb, mp)) {
        if(!(mb->flags & MB_BUSY) && mb->size >= size) {
            if(mb->size == size) {
                best = mb;
                break;
            }
            if(!best || mb->size < min) {
                best = mb;
                min = mb->size;
            }
        }
        mb = MB_NEXT(mb);
    }
    if(best) {
        if(best->size > size + MBLK_MIN + sizeof(struct _mblk)) {
            // split block
            mb = (mblk)((char *)best + sizeof(struct _mblk) + size);
            mb->signature = MBLK_SIGNATURE;
            mb->flags = 0;
            mb->size = best->size - size - sizeof(struct _mblk);
            best->size = size;
        }
        best->flags = MB_BUSY;
        mp->flags |= MP_DIRTY;
        return best + 1;
    }
    return NULL;
}

void *mp_alloc(mpool mp, size_t size) {
    void *ptr;
    mpool current;
    size_t largest = 0;
    size_t workhorse = 0;
    MP_SET(mp);
    MS_ALIGN(size, MBLK_MIN);
    current = mp;
    do {
        workhorse++;
        if(current->size > largest) {
            largest = current->size;
        }
        ptr = _mp_alloc(current, size);
        if(!ptr && _mp_defragment_pool(current)) ptr = _mp_alloc(current,
                    size);
        current = current->next;
    } while(!ptr && current);
    if(!ptr && (mp->flags & MP_EXPAND) == MP_EXPAND) {
        /*
                mpool newpool = mp_create( (largest + size) * MP_EXPAND_FOR,
                        mp->flags );
        */
        mpool newpool = mp_create(MP_EXPAND_FOR((largest + size)),
                                  mp->flags);
        if(!newpool) {
            return NULL;
        }
        if(mp->next) {
            newpool->next = mp->next;
        }
        mp->next = newpool;
        /*
         * swap base pool and newpool:
         */
        newpool->id = mp->id;
        mp->id = workhorse;
        SWAP(workhorse, mp->size, newpool->size);
        SWAP(ptr, mp->min, newpool->min);
        SWAP(ptr, mp->max, newpool->max);
        SWAP(ptr, mp->pool, newpool->pool);
        ptr = _mp_alloc(mp, size);
    }
    return ptr;
}

#if MP_USE_LOCKING
int mp_lock(mpool mp, void *ptr) {
    MP_SET(mp);
    if(_mp_valid_ptr(ptr, mp)) {
        if((((struct _mblk *)ptr) - 1)->flags & MB_BUSY) {
            (((struct _mblk *)ptr) - 1)->flags |= MB_LOCKED;
            return 1;
        }
    }
    return 0;
}

int mp_unlock(mpool mp, void *ptr) {
    MP_SET(mp);
    if(_mp_valid_ptr(ptr, mp)) {
        if((((struct _mblk *)ptr) - 1)->flags & MB_LOCKED) {
            (((struct _mblk *)ptr) - 1)->flags &= ~(MB_LOCKED);
            return 1;
        }
    }
    return 0;
}

int mp_locked(mpool mp, void *ptr) {
    MP_SET(mp);
    return _mp_valid_ptr(ptr, mp) ?
           ((((struct _mblk *)ptr) - 1)->flags & MB_LOCKED) : 0;
}
#endif

void *mp_realloc(mpool mp, void *src, size_t size) {
    void *dest;
    MP_SET(mp);
    dest = NULL;
    if(_mp_valid_ptr(src, mp)) {
#if MP_USE_LOCKING
        if(!((((struct _mblk *)src) - 1)->flags & MB_LOCKED)) {
#endif
            dest = mp_alloc(mp, size);
            if(dest) {
                size_t tomove = (((struct _mblk *)src) - 1)->size;
                if(tomove > size) {
                    tomove = size;
                }
                memcpy(dest, src, tomove);
                mp_free(mp, src);
            }
#if MP_USE_LOCKING
        }
#endif
    }
    return dest;
}

int mp_free(mpool mp, void *ptr) {
    MP_SET(mp);
    if(!ptr || !mp) {
        return 0;
    }
    if(!MP_VALID(ptr, mp)) {
        return mp_free(mp->next, ptr);
    }
#if MP_USE_LOCKING
    if((((struct _mblk *)ptr) - 1)->flags & MB_LOCKED) {
        return 0;
    }
#endif
    if((((struct _mblk *)ptr) - 1)->flags & MB_BUSY) {
        mp->flags |= MP_DIRTY;
    }
    (((struct _mblk *)ptr) - 1)->flags = 0;
    return 1;
}

static char *_mp_format_size(unsigned long size) {
    static char bsz[32];
    if(size < 1024) {
        sprintf(bsz, "%luB", size);
    } else if(size < 1024 * 1024) {
        if(size % 1024) {
            sprintf(bsz, "%lu.%luKb", size / 1024, size % 1024);
        } else {
            sprintf(bsz, "%luKb", size / 1024);
        }
    } else {
        if(size % (1024 * 1024)) {
            sprintf(bsz, "%lu.%luMb", size / (1024 * 1024),
                    size % (1024 * 1024));
        } else {
            sprintf(bsz, "%luMb", size / (1024 * 1024));
        }
    }
    return bsz;
}

void mp_dump(mpool mp, FILE *fout, size_t maxw) {
    unsigned long mp_total = 0;
    unsigned long mp_blocks = 0;
    unsigned long mp_largest = 0;
    size_t mp_total_free = 0;
    size_t mp_largest_free = 0;
    size_t mp_pools = 0;
    mpool current;
    char *outbuf = malloc(maxw + 32);
    if(!outbuf) {
        return;
    }
    MP_SET(mp);
    current = mp;
    while(current) {
        size_t largest = 0;
        //size_t total = 0;
        size_t mb_total = 0;
        size_t onew;
        mblk mb;
        mp_pools++;
        mp_total += current->size;
        if(mp_largest < current->size) {
            mp_largest = current->size;
        }
        mb = (mblk)current->pool;
        while(MB_VALID(mb, current)) {
            mp_blocks++;
            mb_total++;
            //total += mb->size;
            if(largest < mb->size) {
                largest = mb->size;
            }
            mb = MB_NEXT(mb);
        }
        onew = largest / maxw;
#ifndef __WINDOWS__
        fprintf(fout, "ID: %zu, size: %s, ", current->id,
                _mp_format_size(current->size));
        fprintf(fout, "blocks: %zu, internal: %s\n", mb_total,
                _mp_format_size(mb_total * sizeof(struct _mblk)));
#else
        fprintf(fout, "ID: %u, size: %s, ", current->id,
                _mp_format_size(current->size));
        fprintf(fout, "blocks: %u, internal: %s\n", mb_total,
                _mp_format_size(mb_total * sizeof(struct _mblk)));
#endif
        mb = (mblk)current->pool;
        while(MB_VALID(mb, current)) {
            size_t w;
            char c = '.';
            if(mb->flags & MB_BUSY) {
                c = '*';
                if(mb->flags & MB_LOCKED) {
                    c = '#';
                }
            } else {
                mp_total_free += mb->size;
                if(mp_largest_free < mb->size) {
                    mp_largest_free = mb->size;
                }
            }
            memset(outbuf, 0, maxw + 32);
            sprintf(outbuf, "%10s [", _mp_format_size(mb->size));
            w = mb->size / onew;
            if(w > maxw) {
                w = maxw;
            }
            if(!w) {
                w = 1;
            }
            memset(outbuf + strlen(outbuf), c, w);
            fprintf(fout, "%s]\n", outbuf);
            mb = MB_NEXT(mb);
        }
        fprintf(fout, "\n");
        current = current->next;
    }
#ifndef __WINDOWS__
    fprintf(fout, "%-16s: [.] - free, [*] - busy, [#] - locked\n"
            "%-16s: %zu\n", "Legend", "Total pools", mp_pools);
#else
    fprintf(fout, "%-16s: [.] - free, [*] - busy, [#] - locked\n"
            "%-16s: %u\n", "Legend", "Total pools", mp_pools);
#endif
    fprintf(fout, "%-16s: %s\n", "Largest pool",
            _mp_format_size(mp_largest));
    fprintf(fout, "%-16s: %s\n", "Total allocated",
            _mp_format_size(mp_total));
    fprintf(fout, "%-16s: %s\n", "Total free",
            _mp_format_size(mp_total_free));
    fprintf(fout, "%-16s: %lu\n", "Total blocks", mp_blocks);
    fprintf(fout, "%-16s: %s\n", "Largest free",
            _mp_format_size(mp_largest_free));
    fprintf(fout, "%-16s: %s\n", "Internal memory",
            _mp_format_size(
                (mp_blocks * sizeof(struct _mblk))
                + (mp_pools * sizeof(struct _mpool))));
    free(outbuf);
}
