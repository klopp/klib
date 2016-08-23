/*
 *  Created on: 16 авг. 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef HTABLE_H_
#define HTABLE_H_

#if defined(__cplusplus)
extern "C" {
#endif

/*
*  Created on: 11 авг. 2016 г.
*      Author: Vsevolod Lutovinov <klopp@yandex.ru>
*/

#include "config.h"
#include "_lock.h"
#include <errno.h>

#define HT_MIN_SIZE     64

typedef struct _HTItem {
    void *key;
    size_t key_size;
    void *data;
    unsigned int hash;
    struct _HTItem *next;
} *HTItem;

typedef enum {
    HF_HASH_JEN, HF_HASH_LY, HF_HASH_ROT13, HF_HASH_RS, HF_HASH_CRC16,
    HF_HASH_CRC32
}
HT_Hash_Functions;

typedef unsigned int ( *HT_Hash_Function )( const void *data, size_t size );

typedef void ( *HT_Destructor )( void *data );

typedef enum _HT_Flags {
    HTF_DISABLE_EXPAND = 0x01,
    HTF_DISABLE_REDUCE = 0x02
} HT_Flags;

typedef struct _HTable {
    size_t size;
    size_t nitems;
    HTItem *items;
    HT_Destructor destructor;
    HT_Hash_Function hf;
    HT_Flags flags;
    int error;
    __lock_t( lock );
} *HTable;

typedef void ( *HT_Foreach )( HTItem item );

/*
 * 'size' will be rounded up to the next highest power of 2: 100 => 128, 1000 => 1024 etc.
 * Can be 0 or < HT_SIZE_MIN, HT_SIZE_MIN will be used.
 * 'destructor' is a function to delete elements data. Can be NULL.
 */
HTable HT_create( HT_Hash_Functions hf, size_t size, HT_Destructor destructor );
void HT_clear( HTable ht );
void HT_destroy( HTable ht );

/*
 * Foreach iterator
 */
void HT_foreach( HTable ht, HT_Foreach foreach );

size_t HT_max_bucket( HTable ht );
/*
 * Disable/enable expand and reduce internal data storage (enabled by default).
 * Return HTable pointer from arguments.
 */
HTable HT_disable_expand( HTable ht );
HTable HT_disable_reduce( HTable ht );
HTable HT_enable_expand( HTable ht );
HTable HT_enable_reduce( HTable ht );

/*
 * Used error codes (HTable.error): 0 (no errors), ENOKEY, ENOMEM
 */
HTItem HT_set( HTable ht, const void *key, size_t key_size, void *data );
void *HT_get( HTable ht, const void *key, size_t key_size );
int HT_delete( HTable ht, const void *key, size_t key_size );

/*
 * C-strings keys handling:
 * HT_set_c( ht, "fookey", data );
 * ... etc
 */
HTItem HT_set_c( HTable ht, const char *key, void *data );
void *HT_get_c( HTable ht, const char *key );
int HT_delete_c( HTable ht, const char *key );

/*
 * HT_set_char( ht, 'c', data );
 * HT_set_int( ht, -1, data );
 * HT_set_ulong( ht, 12345678, data );
 * ... etc
 */
#define HT_INTEGER_DECL(tag, type) \
    HTItem HT_set_##tag( HTable ht, type key, void *data ); \
    void *HT_get_##tag( HTable ht, type key); \
    int HT_delete_##tag( HTable ht, type key);

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
