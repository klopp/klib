/*
 * hash.h, part of "klib" project.
 *
 *  Created on: 24.05.2015, 03:33
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef HASH_H_
#define HASH_H_

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

unsigned hash_jen( const void *buf, size_t size );
unsigned hash_rot13( const void *buf, size_t size );
unsigned hash_ly( const void *buf, size_t size );
unsigned hash_rs( const void *buf, size_t size );

unsigned hash_jen_update( unsigned startval, const void *buf, size_t size );
unsigned hash_rot13_update( unsigned startval, const void *buf, size_t size );
unsigned hash_ly_update( unsigned startval, const void *buf, size_t size );
unsigned hash_rs_update( unsigned startval, const void *buf, size_t size );

unsigned shash_jen( const char *buf );
unsigned shash_rot13( const char *buf );
unsigned shash_ly( const char *buf );
unsigned shash_rs( const char *buf );

unsigned shash_jen_update( unsigned startval, const char *buf );
unsigned shash_rot13_update( unsigned startval, const char *buf );
unsigned shash_ly_update( unsigned startval, const char *buf );
unsigned shash_rs_update( unsigned startval, const char *buf );

#ifdef __cplusplus
}
#endif

#endif /* HASH_H_ */
