/*
 * uniqid.c, part of "klib" project.
 *
 *  Created on: 05.09.2015, 01:08
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "uniqid.h"
#include <sys/time.h>
#include <stdio.h>

char *uniqid(char *storage, size_t storage_len) {
    struct timeval tv;
    unsigned int sec, usec;
    if(!storage) {
        storage_len = 8 + 5;
        storage = Malloc(storage_len + 1);
        if(!storage) {
            return NULL;
        }
    }
    gettimeofday(&tv, NULL);
    sec = (unsigned int)tv.tv_sec;
    usec = ((unsigned int)tv.tv_usec % 0x100000);
    snprintf(storage, storage_len, "%08x%05x", sec, usec);
    return storage;
}
