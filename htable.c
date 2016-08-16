/*
 *  Created on: 16 авг. 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "htable.h"

#include "../klib/crc.h"
#include "../klib/hash.h"

#define HT_MIN_SIZE     32

/*
 * Because crc16() return short:
 */
static inline unsigned int _crc16(const void *buf, size_t size)
{
    return crc16(buf, size);
}

static struct
{
    HT_Hash_Functions idx;
    HT_Hash_Function hf;

} _hf[] = { { HF_HASH_FAQ6, hash_faq6 }, { HF_HASH_LY, hash_ly }, { HF_HASH_ROT13, hash_rot13 }, { HF_HASH_RS,
        hash_rs }, { HF_HASH_CRC16, _crc16 }, { HF_HASH_CRC32, crc32 } };

HTable HT_create(HT_Hash_Functions hf, HT_Destructor destructor)
{
    size_t i;
    HTable ht = Malloc(sizeof(struct _HTable));
    if (!ht) {
        return NULL;
    }
    ht->size = HT_MIN_SIZE;
    ht->items = Calloc(HT_MIN_SIZE, sizeof(struct _HTItem));
    if (!ht->items) {
        Free(ht);
        return NULL;
    }
    ht->nitems = 0;
    ht->hf = NULL;
    ht->mask = ht->size - 1;
    ht->destructor = destructor;
    __initlock(ht->lock);

    for (i = 0; i < sizeof(_hf) / sizeof(_hf[0]); i++) {
        if (hf == _hf[i].idx) {
            ht->hf = _hf[i].hf;
        }
    }

    if (!ht->hf) {
        ht->hf = _hf[0].hf;
    }

    return ht;
}

static void _HT_Free_Item(HTItem e, HTable ht)
{
    if (e->next) {
        _HT_Free_Item(e->next, ht);
    }

    if (ht->destructor) {
        ht->destructor(e->data);
    }

    Free(e->key);
    Free(e);
}

void HT_clear(HTable ht)
{
    size_t i;
    __lock(ht->lock);

    for (i = 0; i < ht->nitems; i++) {
        _HT_Free_Item(ht->items[i], ht);
    }
    ht->nitems = 0;
    __unlock(ht->lock);
}

void HT_destroy(HTable ht)
{
    HT_clear(ht);
    Free(ht->items);
    Free(ht);
}

void *HT_get(HTable ht, const void *key, size_t key_size)
{
    unsigned int hash;
    HTItem e = NULL;

    __lock(ht->lock);
    hash = ht->hf(key, key_size);
    e = ht->items[hash & ht->mask];

    if (e->next) {
        while (e) {
            if (e->key_size == key_size && !memcmp(e->key, key, key_size)) {
                break;
            }
            e = e->next;
        }
    }

    __unlock(ht->lock);

    return e ? e->data : NULL;

}

int HT_delete( HTable ht, const void *key, size_t key_size )
{
    unsigned int hash;
    HTItem e = NULL;
    int rc = 0;

    __lock(ht->lock);
    hash = ht->hf(key, key_size);
    e = ht->items[hash & ht->mask];

    __unlock(ht->lock);

    return rc;
}

/*
 *  That's All, Folks!
 */

