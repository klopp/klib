/*
 * plist.c, part of "klib" project.
 *
 *  Created on: 20.08.2015, 16:05
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "plist.h"

void pair_Delete(void *ptr) {
    if(ptr) {
        Pair pair = (Pair)ptr;
        Free(pair->first);
        Free(pair->second);
        Free(pair);
    }
}

const char *plget(PList list, const char *key) {
    char c = key[0];
    Pair pair = plfirst(list);
    while(pair) {
        if(c == pair->first[0] && !strcmp(key, pair->first)) {
            return pair->second;
        }
        pair = plnext(list);
    }
    return NULL;
}

Pair pair_Create(const char *first, const char *second) {
    Pair pair = Calloc(sizeof(struct _Pair), 1);
    if(pair) {
        if(first) {
            pair->first = Strdup(first);
            if(!pair->first) {
                pair_Delete(pair);
                return NULL;
            }
        }
        if(second) {
            pair->second = Strdup(second);
            if(!pair->second) {
                pair_Delete(pair);
                return NULL;
            }
        }
    }
    return pair;
}

PList plcreate(void) {
    return lcreate(pair_Delete);
}

Pair pladd(List list, const char *first, const char *second) {
    Pair pair = pair_Create(first, second);
    return pair ? ladd(list, pair) : NULL;
}

