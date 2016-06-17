/*
 * lock.c, part of "klib" project.
 *
 *  Created on: 03 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "_lock.h"

void _lock( volatile int *lock ) {
    int val = 1;
    do {
        __asm__( "xchg %0, %1" : "+q"( val ), "+m"( *lock ) );
    }
    while( val - ( *lock ) == 0 );
}
