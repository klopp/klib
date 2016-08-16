/*
 *  Created on: 16 авг. 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "htable.h"

#include "../klib/crc.h"
#include "../klib/hash.h"

#define HT_MIN_SIZE     1024

/*
 * Because crc16() return short:
 */
static inline unsigned int _crc16( const void *buf, size_t size )
{
    return crc16( buf, size );
}

static struct {
    HT_Hash_Functions idx;
    HT_Hash_Function hf;

} _hf[] = { { HF_HASH_FAQ6, hash_faq6 }, { HF_HASH_LY, hash_ly }, { HF_HASH_ROT13, hash_rot13 }, {
        HF_HASH_RS,
        hash_rs
    }, { HF_HASH_CRC16, _crc16 }, { HF_HASH_CRC32, crc32 }
};

/*
 * Create hash table with given initial size. Return created table or NULL.
 */
HTable HT_create( HT_Hash_Functions hf, size_t size, HT_Destructor destructor )
{
    size_t i;
    HTable ht = Malloc( sizeof( struct _HTable ) );

    if( !ht ) {
        return NULL;
    }

    ht->size = size > HT_MIN_SIZE ? size : HT_MIN_SIZE;
    ht->items = Calloc( ht->size, sizeof( struct _HTItem ) );

    if( !ht->items ) {
        Free( ht );
        return NULL;
    }

    ht->nitems = 0;
    ht->hf = NULL;
    ht->mask = ht->size - 1;
    ht->destructor = destructor;
    ht->error = 0;
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
static void _HT_Destroy_Item( HTItem e, HTable ht )
{
    if( e->next ) {
        _HT_Destroy_Item( e->next, ht );
    }

    if( ht->destructor ) {
        ht->destructor( e->data );
    }

    Free( e->key );
    Free( e );
}

/*
 * Delete all hash table items:
 */
void HT_clear( HTable ht )
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
    __unlock( ht->lock );
}

/*
 * Destroy hash table:
 */
void HT_destroy( HTable ht )
{
    HT_clear( ht );
    Free( ht->items );
    Free( ht );
}

/*
 * Returm max items bucket length:
 */
size_t HT_maxdepth( HTable ht )
{
    size_t i;
    size_t maxdepth = 0;
    __lock( ht->lock );
    ht->error = 0;
    maxdepth = 0;

    for( i = 0; i < ht->size; i++ ) {
        if( ht->items[i] && ht->items[i]->next ) {
            size_t depth = 0;
            HTItem e = ht->items[i];

            while( e ) {
                depth++;
                e = e->next;
            }

            if( depth > maxdepth ) {
                maxdepth = depth;
            }
        }
    }

    __unlock( ht->lock );
    return maxdepth;
}

/*
 * Get hash table item data. Return data found or NULL. Set internal
 * error code.
 */
void *HT_get( HTable ht, const void *key, size_t key_size )
{
    unsigned int hash;
    HTItem e = NULL;
    __lock( ht->lock );
    hash = ht->hf( key, key_size );
    e = ht->items[hash & ht->mask];
    ht->error = ENOKEY;

    if( e->next ) {
        while( e ) {
            if( e->key_size == key_size && !memcmp( e->key, key, key_size ) ) {
                ht->error = 0;
                break;
            }

            e = e->next;
        }
    }

    __unlock( ht->lock );
    return e ? e->data : NULL;
}

/*
 * Delete hash table item. Return 1 (success) or 0 (not found). Set internal
 * error code.
 */
int HT_delete( HTable ht, const void *key, size_t key_size )
{
    unsigned int hash;
    HTItem cursor;
    HTItem e;
    int rc;
    size_t idx;
    __lock( ht->lock );
    rc = 0;
    e = NULL;
    hash = ht->hf( key, key_size );
    idx = hash & ht->mask;
    cursor = ht->items[idx];
    ht->error = ENOKEY;

    while( cursor ) {
        if( cursor->key_size == key_size && !memcmp( cursor->key, key, key_size ) ) {
            if( !e ) {
                ht->items[idx]->data = cursor->next;
            }
            else {
                e->next = cursor->next;
            }

            if( ht->destructor ) {
                ht->destructor( cursor->data );
            }

            Free( cursor->key );
            Free( cursor );
            ht->nitems--;
            ht->error = 0;
            rc++;
            break;
        }

        e = cursor;
        cursor = cursor->next;
    }

    __unlock( ht->lock );
    return rc;
}

/*
 * Insert  hash table item. Return item hash (success) or 0 (failed). Set internal
 * error code.
 */
unsigned int HT_set( HTable ht, const void *key, size_t key_size, void *data )
{
    unsigned int hash;
    HTItem e;
    HTItem cursor;
    size_t idx;
    __lock( ht->lock );
    ht->error = 0;
    e = Malloc( sizeof( struct _HTItem ) );

    if( !e ) {
        ht->error = ENOMEM;
        __unlock( ht->lock );
        return 0;
    }

    e->key = Malloc( key_size );

    if( !e->key ) {
        ht->error = ENOMEM;
        Free( e );
        __unlock( ht->lock );
        return 0;
    }

    memcpy( e->key, key, key_size );
    e->key_size = key_size;
    e->data = data;
    e->next = NULL;
    hash = ht->hf( key, key_size );
    e->hash = hash;
    idx = hash & ht->mask;

    if( !ht->items[idx] ) {
        ht->items[idx] = e;
        ht->nitems++;
        __unlock( ht->lock );
        return hash;
    }

    cursor = ht->items[idx];

    while( 1 ) {
        /*
         * Item found, check key:
         */
        if( cursor->key_size == key_size && !memcmp( cursor->key, key, key_size ) ) {
            /*
             * Keys equals, destroy & replace old data:
             */
            if( ht->destructor ) {
                ht->destructor( cursor->data );
            }

            cursor->data = data;
            Free( e->key );
            Free( e );
            ht->nitems++;
            __unlock( ht->lock );
            return hash;
        }

        if( cursor->next ) {
            cursor = cursor->next;
            continue;
        }

        cursor->next = e;
        ht->nitems++;
        break;
    }

    __unlock( ht->lock );
    return hash;
}

/*
 * Various key types:
 */
unsigned int HT_set_c( HTable ht, const char *key, void *data )
{
    return HT_set( ht, key, strlen( key ), data );
}

void *HT_get_c( HTable ht, const char *key )
{
    return HT_get( ht, key, strlen( key ) );
}

int HT_delete_c( HTable ht, const char *key )
{
    return HT_delete( ht, key, strlen( key ) );
}

#define HT_INTEGER_IMPL(tag, type) \
    unsigned int HT_set_##tag( HTable ht, type key, void *data ) { \
        return HT_set( ht, &key, sizeof(key), data ); \
    } \
    void *HT_get_##tag( HTable ht, type key) {; \
        return HT_get( ht, &key, sizeof(key) ); \
    } \
    int HT_delete_##tag( HTable ht, type key) { \
        return HT_delete( ht, &key, sizeof(key) ); \
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

