/*
 *  Created on: 16 авг. 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "htable.h"
#include "crc.h"
#include "hash.h"

#define HT_HASH_MASK(ht)    (ht->size-1)
#define QSORT_STACK_SIZE    128
#define ISORT_LIMIT         128

static struct {
    HT_Hash_Functions idx;
    HT_Hash_Function hf;

} _hf[] = { { HF_HASH_JEN, hash_jen }, { HF_HASH_LY, hash_ly }, { HF_HASH_ROT13, hash_rot13 }, {
        HF_HASH_RS,
        hash_rs
    }, { HF_HASH_CRC32, crc32 }
};

/*
 * Create hash table with given initial size. Return created table or NULL.
 */
HTable HT_create( HT_Hash_Functions hf, size_t size, HT_Destructor destructor )
{
    size_t i;
    const HTable ht = Malloc( sizeof( struct _HTable ) );

    if( !ht ) {
        return NULL;
    }

    ht->size = size;

    if( ht->size < HT_MIN_SIZE ) {
        ht->size = HT_MIN_SIZE;
    }

    if( ht->size & ( ht->size - 1 ) ) {
        /*
         * Round up to the next highest power of 2:
         */
        i = 1;

        while( i < ht->size ) {
            i *= 2;
        }

        ht->size = i;
    }

    ht->items = Calloc( ht->size, sizeof( HTItem ) );

    if( !ht->items ) {
        Free( ht );
        return NULL;
    }

    ht->nitems = 0;
    ht->order = 0;
    ht->hf = NULL;
    ht->destructor = destructor;
    ht->error = 0;
    ht->flags = 0;
    __initlock( ht->lock );

    for( i = 0; i < sizeof( _hf ) / sizeof( _hf[0] ); i++ ) {
        if( hf == _hf[i].idx ) {
            ht->hf = _hf[i].hf;
        }
    }

    if( !ht->hf ) {
        ht->hf = _hf[0].hf;
    }

    return ht;
}

/*
 * Internal, destroy item:
 */
static void _HT_Destroy_Item( HTItem e, const HTable ht )
{
    if( e->next ) {
        _HT_Destroy_Item( e->next, ht );
    }

    if( ht->destructor ) {
        ht->destructor( e->data );
    }

    Free( e->key.key );
    Free( e );
}

/*
 * Delete all hash table items:
 */
void HT_clear( const HTable ht )
{
    size_t i;
    __lock( ht->lock );

    for( i = 0; i < ht->size; i++ ) {
        if( ht->items[i] ) {
            _HT_Destroy_Item( ht->items[i], ht );
        }
    }

    ht->nitems = 0;
    ht->error = 0;
    ht->order = 0;
    __unlock( ht->lock );
}

/*
 * Destroy hash table:
 */
void HT_destroy( const HTable ht )
{
    HT_clear( ht );
    Free( ht->items );
    Free( ht );
}

static void _HT_ForEach( HTItem item, HT_Foreach foreach, void *data )
{
    if( item->next ) {
        _HT_ForEach( item->next, foreach, data );
    }

    foreach( item, data );
}

/*
 * Foreach iterator:
 */
void HT_foreach( const HTable ht, HT_Foreach foreach, void *data )
{
    size_t i;
    __lock( ht->lock );

    for( i = 0; i < ht->size; i++ ) {
        if( ht->items[i] ) {
            _HT_ForEach( ht->items[i], foreach, data );
        }
    }

    __unlock( ht->lock );
}

static void _HT_items( const HTItemConst item, void *data )
{
    struct {
        HTItemConst *items;
        size_t idx;
        size_t nitems;
    }*ptr = data;

    if( !ptr->items ) {
        ptr->items = Malloc( sizeof( HTItem ) * ptr->nitems );
    }

    if( ptr->items ) {
        ptr->items[ptr->idx++] = item;
    }
}

/*
 * Get hash table items (unordered):
 */
HTItemConst *HT_items( const HTable ht )
{
    struct {
        HTItemConst *items;
        size_t idx;
        size_t nitems;
    } data = { NULL, 0, ht->nitems };

    if( ht->nitems ) {
        HT_foreach( ht, _HT_items, &data );
    }

    return data.items;
}

/*
 * Internal, compare items insertion order:
 */
static int _HTItem_compare_order( const HTItemConst a, const HTItemConst b )
{
    if( a->order != b->order ) {
        return a->order > b->order ? 1 : -1;
    }

    return 0;
}

/*
 * Internal, insertion sort implementation:
 */
static HTItemConst *_HT_ISort( HTItemConst *items, size_t nitems,
                               HT_Compare compare )
{
    size_t i, j;
    HTItemConst tmp;

    for( i = 1; i < nitems; i++ ) {
        tmp = items[i];

        for( j = i - 1; j >= 0; j-- ) {
            if( compare( items[j], tmp ) < 0 ) {
                break;
            }

            items[j + 1] = items[j];
            items[j] = tmp;
        }
    }

    return items;
}

/*
 * Internal, quick sort implementation:
 */
static HTItemConst *_HT_QSort( HTItemConst *items, size_t nitems,
                               HT_Compare compare )
{
    long i, j;
    long lb, ub;
    long lbstack[QSORT_STACK_SIZE], ubstack[QSORT_STACK_SIZE];
    size_t stackpos = 1, ppos;
    HTItemConst pivot;
    lbstack[1] = 0;
    ubstack[1] = nitems - 1;

    do {
        lb = lbstack[stackpos];
        ub = ubstack[stackpos];
        stackpos--;

        do {
            ppos = ( lb + ub ) / 2;
            i = lb;
            j = ub;
            pivot = items[ppos];

            do {
                while( compare( items[i], pivot ) < 0 ) {
                    i++;
                }

                while( compare( pivot, items[j] ) < 0 ) {
                    j--;
                }

                if( i <= j ) {
                    if( i != j ) {
                        HTItemConst tmp = items[i];
                        items[i] = items[j];
                        items[j] = tmp;
                    }

                    i++;
                    j--;
                }
            } while( i <= j );

            if( i < ppos ) {
                if( i < ub ) {
                    stackpos++;
                    lbstack[stackpos] = i;
                    ubstack[stackpos] = ub;
                }

                ub = j;
            }
            else {
                if( j > lb ) {
                    stackpos++;
                    lbstack[stackpos] = lb;
                    ubstack[stackpos] = j;
                }

                lb = i;
            }
        } while( lb < ub );
    } while( stackpos != 0 );

    return items;
}

/*
 * Internal, convert items array to keys array:
 */
static HTIKeyConst *_HT_Items2Keys( HTItemConst *items, size_t max )
{
    HTIKeyConst *keys = Malloc( max * sizeof( HTIKeyConst ) + 1 );

    if( keys ) {
        size_t i;

        for( i = 0; i < max; i++ ) {
            keys[i] = &items[i]->key;
        }

        keys[i] = NULL;
    }

    Free( items );
    return keys;
}

HTIKeyConst *HT_keys( const HTable ht )
{
    HTItemConst *items = HT_items( ht );

    if( items ) {
        return _HT_Items2Keys( items, ht->nitems );
    }

    return NULL;
}

/*
 * Get keys sorted by insertion order:
 */
HTIKeyConst *HT_ordered_keys( const HTable ht )
{
    HTItemConst *items = HT_ordered_items( ht );

    if( items ) {
        return _HT_Items2Keys( items, ht->nitems );
    }

    return NULL;
}

/*
 * Get keys sorted by user function:
 */
HTIKeyConst *HT_sorted_keys( const HTable ht, HT_Compare compare )
{
    HTItemConst *items = HT_sorted_items( ht, compare );

    if( items ) {
        return _HT_Items2Keys( items, ht->nitems );
    }

    return NULL;
}

/*
 * Internal, convert items array to values array:
 */
static void const **_HT_Items2Values( HTItemConst *items, size_t max )
{
    void const **values = Malloc( max * sizeof( void const * ) + 1 );

    if( values ) {
        size_t i;

        for( i = 0; i < max; i++ ) {
            values[i] = items[i]->data;
        }

        values[i] = NULL;
    }

    Free( items );
    return values;
}

void const **HT_values( const HTable ht )
{
    HTItemConst *items = HT_items( ht );

    if( items ) {
        return _HT_Items2Values( items, ht->nitems );
    }

    return NULL;
}

/*
 * Get values sorted by insertion order:
 */
void const **HT_ordered_values( const HTable ht )
{
    HTItemConst *items = HT_ordered_items( ht );

    if( items ) {
        return _HT_Items2Values( items, ht->nitems );
    }

    return NULL;
}

/*
 * Get values sorted by user function:
 */
void const **HT_sorted_values( const HTable ht, HT_Compare compare )
{
    HTItemConst *items = HT_sorted_items( ht, compare );

    if( items ) {
        return _HT_Items2Values( items, ht->nitems );
    }

    return NULL;
}

/*
 * Get items sorted by insertion order:
 */
HTItemConst *HT_ordered_items( const HTable ht )
{
    return HT_sorted_items( ht, _HTItem_compare_order );
}

/*
 * Sort items by user function:
 */
HTItemConst *HT_sort_items( HTItemConst *items, size_t nitems,
                            HT_Compare compare )
{
    return nitems > ISORT_LIMIT ? _HT_QSort( items, nitems,
            compare ) : _HT_ISort( items, nitems, compare );
}

/*
 * Get items sorted by user function:
 */
HTItemConst *HT_sorted_items( const HTable ht, HT_Compare compare )
{
    HTItemConst *items = HT_items( ht );

    if( items ) {
        ht->nitems > ISORT_LIMIT ? _HT_QSort( items, ht->nitems,
                                              compare ) : _HT_ISort( items, ht->nitems, compare );
    }

    return items;
}

/*
 * Returm max items bucket length:
 */
size_t HT_max_bucket( const HTable ht )
{
    size_t i;
    size_t max_bucket;
    __lock( ht->lock );
    ht->error = 0;
    max_bucket = 0;

    for( i = 0; i < ht->size; i++ ) {
        if( ht->items[i] && ht->items[i]->next ) {
            size_t bucket = 0;
            HTItem e = ht->items[i];

            while( e ) {
                bucket++;
                e = e->next;
            }

            if( bucket > max_bucket ) {
                max_bucket = bucket;
            }
        }
    }

    __unlock( ht->lock );
    return max_bucket;
}

HTable HT_disable_expand( const HTable ht )
{
    ht->flags |= HTF_DISABLE_EXPAND;
    return ht;
}

HTable HT_disable_reduce( const HTable ht )
{
    ht->flags |= HTF_DISABLE_REDUCE;
    return ht;
}

HTable HT_enable_expand( const HTable ht )
{
    ht->flags &= ( ~HTF_DISABLE_EXPAND );
    return ht;
}

HTable HT_enable_reduce( const HTable ht )
{
    ht->flags &= ( ~HTF_DISABLE_REDUCE );
    return ht;
}

/*
 * Reduce storage. Return 1 (success) or 0 (failed). Do not change internal
 * error code.
 */
static int _HT_Reduce( const HTable ht )
{
    size_t newsize;
    HTItem *items;
    size_t i;

    if( ht->size < HT_MIN_SIZE ) {
        return 1;
    }

    newsize = ht->size / 2;
    items = Calloc( newsize, sizeof( HTItem ) );

    if( !items ) {
        return 0;
    }

    memcpy( items, ht->items, newsize * sizeof( HTItem ) );

    for( i = newsize; i < ht->size; i++ ) {
        HTItem cur;

        if( !ht->items[i] ) {
            continue;
        }

        if( !items[i - newsize] ) {
            items[i - newsize] = ht->items[i];
        }
        else {
            cur = items[i - newsize];

            while( cur->next ) {
                cur = cur->next;
            }

            cur->next = ht->items[i];
        }
    }

    Free( ht->items );
    ht->items = items;
    ht->size = newsize;
    return 1;
}

/*
 * Expand storage. Return 1 (success) or 0 (failed). Do not change internal
 * error code.
 */
static int _HT_Expand( const HTable ht )
{
    size_t newsize = ht->size * 2;
    size_t newmask;
    size_t mask;
    size_t i;
    HTItem *items = Calloc( newsize, sizeof( HTItem ) );

    if( !items ) {
        return 0;
    }

    mask = HT_HASH_MASK( ht );
    newmask = newsize - 1;
    memcpy( items, ht->items, ht->size * sizeof( HTItem ) );

    for( i = 0; i < ht->size; i++ ) {
        HTItem cur = items[i], prev = NULL, temp;

        while( cur ) {
            if( ( cur->hash & mask ) != ( cur->hash & newmask ) ) {
                if( !prev ) {
                    items[i] = cur->next;
                }
                else {
                    prev->next = cur->next;
                }

                temp = cur->next;
                cur->next = items[cur->hash & newmask];
                items[cur->hash & newmask] = cur;
                cur = temp;
            }
            else {
                prev = cur;
                cur = cur->next;
            }
        }
    }

    Free( ht->items );
    ht->items = items;
    ht->size = newsize;
    return 1;
}

/*
 * Get hash table item data. Return data found or NULL. Set internal
 * error code.
 */
HTItemConst HT_get( const HTable ht, const void *key, size_t key_size )
{
    unsigned int hash;
    HTItem e;
    __lock( ht->lock );
    hash = ht->hf( key, key_size );
    e = ht->items[hash & HT_HASH_MASK( ht )];
    ht->error = ENOKEY;

    while( e ) {
        if( e->key.size == key_size && !memcmp( e->key.key, key, key_size ) ) {
            ht->error = 0;
            break;
        }

        e = e->next;
    }

    __unlock( ht->lock );
    return e;
}

void const *HT_val( const HTable ht, const void *key, size_t key_size )
{
    HTItemConst item = HT_get( ht, key, key_size );
    return item ? item->data : NULL;
}

/*
 * Delete hash table item. Return 1 (success) or 0 (not found). Set internal
 * error code.
 */
int HT_del( const HTable ht, const void *key, size_t key_size )
{
    unsigned int hash;
    HTItem cursor;
    HTItem e;
    size_t idx;
    __lock( ht->lock );
    e = NULL;
    hash = ht->hf( key, key_size );
    idx = hash & HT_HASH_MASK( ht );
    cursor = ht->items[idx];
    ht->error = ENOKEY;

    while( cursor ) {
        if( cursor->key.size == key_size &&
                !memcmp( cursor->key.key, key, key_size ) ) {
            if( !e ) {
                ht->items[idx] = cursor->next;
            }
            else {
                e->next = cursor->next;
            }

            if( ht->destructor ) {
                ht->destructor( cursor->data );
            }

            Free( cursor->key.key );
            Free( cursor );
            ht->nitems--;
            ht->error = 0;

            if( !ht->items ) {
                ht->order = 0;
            }

            if( ( ht->nitems < ht->size / 2 ) && !( ht->flags & HTF_DISABLE_REDUCE ) ) {
                _HT_Reduce( ht );
            }

            break;
        }

        e = cursor;
        cursor = cursor->next;
    }

    __unlock( ht->lock );
    return ht->error;
}

/*
 * Insert hash table item. Return created item pointer (success) or NULL (failed).
 * Set internal error code.
 */
HTItemConst HT_set( const HTable ht, const void *key, size_t key_size,
                    void *data )
{
    unsigned int hash;
    HTItem e;
    HTItem item;
    size_t idx;
    __lock( ht->lock );
    hash = ht->hf( key, key_size );
    idx = hash & HT_HASH_MASK( ht );
    e = ht->items[idx];

    while( e ) {
        if( e->key.size == key_size && !memcmp( e->key.key, key, key_size ) ) {
            if( ht->destructor ) {
                ht->destructor( e->data );
            }

            e->data = data;
            ht->error = 0;
            __unlock( ht->lock );
            return e;
        }

        e = e->next;
    }

    item = Malloc( sizeof( struct _HTItem ) );

    if( !item ) {
        ht->error = ENOMEM;
        __unlock( ht->lock );
        return NULL;
    }

    item->key.key = Malloc( key_size );

    if( !item->key.key ) {
        ht->error = ENOMEM;
        Free( item );
        __unlock( ht->lock );
        return NULL;
    }

    memcpy( item->key.key, key, key_size );
    item->key.size = key_size;
    item->order = ht->order++;
    item->data = data;
    item->next = NULL;
    item->hash = hash;
    item->next = ht->items[idx];
    ht->items[idx] = item;
    ht->error = 0;
    ht->nitems++;

    if( ( ht->nitems > ht->size ) && !( ht->flags & HTF_DISABLE_EXPAND ) ) {
        _HT_Expand( ht );
    }

    __unlock( ht->lock );
    return item;
}

/*
 * Various key types:
 */
HTItemConst HT_set_c( const HTable ht, const char *key, void *data )
{
    return HT_set( ht, key, strlen( key ), data );
}

HTItemConst HT_get_c( const HTable ht, const char *key )
{
    return HT_get( ht, key, strlen( key ) );
}

int HT_del_c( const HTable ht, const char *key )
{
    return HT_del( ht, key, strlen( key ) );
}

#define HT_INTEGER_IMPL(tag, type) \
    HTItemConst HT_set_##tag(const HTable ht, type key, void *data ) { \
        return HT_set( ht, &key, sizeof(key), data ); \
    } \
    HTItemConst HT_get_##tag(const HTable ht, type key) {; \
        return HT_get( ht, &key, sizeof(key) ); \
    } \
    void const *HT_val_##tag(const HTable ht, type key) {; \
        return HT_val( ht, &key, sizeof(key) ); \
    } \
    int HT_del_##tag(const HTable ht, type key) { \
        return HT_del( ht, &key, sizeof(key) ); \
    }

HT_INTEGER_IMPL( szt, size_t );
HT_INTEGER_IMPL( char, char );
HT_INTEGER_IMPL( uchar, unsigned char );
HT_INTEGER_IMPL( short, short );
HT_INTEGER_IMPL( ushort, unsigned short );
HT_INTEGER_IMPL( int, int );
HT_INTEGER_IMPL( uint, unsigned int );
HT_INTEGER_IMPL( long, long );
HT_INTEGER_IMPL( ulong, unsigned long );
HT_INTEGER_IMPL( llong, long long );
HT_INTEGER_IMPL( ullong, unsigned long long );

/*
 *  That's All, Folks!
 */

