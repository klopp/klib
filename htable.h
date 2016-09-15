/*
 *  Created on: 16 авг. 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef HTABLE_H_
#define HTABLE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "config.h"
#include "_lock.h"
#include <errno.h>

#define HT_MIN_SIZE     64

typedef struct _HTItem {
    void *key;
    size_t key_size;
    size_t order;
    void *data;
    unsigned int hash;
    struct _HTItem *next;
} HTItem;

typedef enum {
    HF_HASH_JEN, HF_HASH_LY, HF_HASH_ROT13, HF_HASH_RS, HF_HASH_CRC16,
    HF_HASH_CRC32
} HT_Hash_Functions;

typedef unsigned int ( *HT_Hash_Function )( const void *data, size_t size );
typedef void ( *HT_Destructor )( void *data );

typedef enum _HT_Flags {
    HTF_DISABLE_EXPAND = 0x01,
    HTF_DISABLE_REDUCE = 0x02
} HT_Flags;

typedef struct _HTable {
    size_t size;
    size_t nitems;
    HTItem **items;
    HT_Destructor destructor;
    HT_Hash_Function hf;
    HT_Flags flags;
    size_t order;
    int error;
    __lock_t( lock );
} *HTable;

typedef void ( *HT_Foreach )( const HTItem const *item, void *data );
typedef int ( *HT_Compare )( const HTItem const *a, const HTItem const *b );

/*
 * 'size' will be rounded up to the next highest power of 2: 100 => 128, 1000 => 1024 etc.
 * Can be 0 or < HT_SIZE_MIN, HT_SIZE_MIN will be used.
 * 'destructor' is a function to delete elements data. Can be NULL.
 */
HTable HT_create( HT_Hash_Functions hf, size_t size, HT_Destructor destructor );
void HT_clear( const HTable ht );
void HT_destroy( const HTable ht );

/*
 * Foreach iterator:
 */
void HT_foreach( const HTable ht, HT_Foreach foreach, void *data );

/*
 * Get hash table items:
 */
HTItem const **HT_items( const HTable ht );
/*
 * Get items sorted by insertion order:
 */
HTItem const **HT_ordered_items( const HTable ht );
/*
 * Get items with custom sorting:
 */
HTItem const **HT_sorted_items( const HTable ht, HT_Compare compare );
/*
 * Sort items array:
 */
HTItem const **HT_sort_items( HTItem const **items, size_t nitems,
                              HT_Compare compare );

size_t HT_max_bucket( const HTable ht );
/*
 * Disable/enable expand and reduce internal data storage (enabled by default).
 * Return HTable pointer from arguments.
 */
HTable HT_disable_expand( const HTable ht );
HTable HT_disable_reduce( const HTable ht );
HTable HT_enable_expand( const HTable ht );
HTable HT_enable_reduce( const HTable ht );

/*
 * Used error codes (HTable.error): 0 (no errors), ENOKEY, ENOMEM
 */
HTItem const *HT_set( const HTable ht, const void *key, size_t key_size,
                      void *data );
HTItem const *HT_get( const HTable ht, const void *key, size_t key_size );
void const *HT_val( const HTable ht, const void *key, size_t key_size );
/*
 * Return ENOKEY or 0 (success):
 */
int HT_del( const HTable ht, const void *key, size_t key_size );

/*
 * C-strings keys handling:
 * HT_set_c( ht, "fookey", data );
 * ... etc
 */
HTItem const *HT_set_c( const HTable ht, const char *key, void *data );
HTItem const *HT_get_c( const HTable ht, const char *key );
int HT_del_c( const HTable ht, const char *key );

/*
 * HT_set_char( ht, 'c', data );
 * HT_set_int( ht, -1, data );
 * HT_set_ulong( ht, 12345678, data );
 * ... etc
 */
#define HT_INTEGER_DECL(tag, type) \
    HTItem const *HT_set_##tag(const HTable ht, type key, void *data ); \
    HTItem const *HT_get_##tag(const HTable ht, type key); \
    void const *HT_val_##tag(const HTable ht, type key); \
    int HT_del_##tag(const HTable ht, type key);

HT_INTEGER_DECL( szt, size_t );
HT_INTEGER_DECL( char, char );
HT_INTEGER_DECL( uchar, unsigned char );
HT_INTEGER_DECL( short, short );
HT_INTEGER_DECL( ushort, unsigned short );
HT_INTEGER_DECL( int, int );
HT_INTEGER_DECL( uint, unsigned int );
HT_INTEGER_DECL( long, long );
HT_INTEGER_DECL( ulong, unsigned long );
HT_INTEGER_DECL( llong, long long );
HT_INTEGER_DECL( ullong, unsigned long long );

#if defined(__cplusplus)
}; /* extern "C" */
#endif

#endif /* HTABLE_H_ */

/*
 *  That's All, Folks!
 */
