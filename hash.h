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

unsigned hash_faq6( unsigned startval, const void *buf, size_t size );
unsigned hash_rot13( unsigned startval, const void *buf, size_t size );
unsigned hash_ly( unsigned startval, const void *buf, size_t size );
unsigned hash_rs( unsigned startval, const void *buf, size_t size );

unsigned shash_faq6( unsigned startval, const char *buf );
unsigned shash_rot13( unsigned startval, const char *buf );
unsigned shash_ly( unsigned startval, const char *buf );
unsigned shash_rs( unsigned startval, const char *buf );

#ifdef __cplusplus
}
#endif

#endif /* HASH_H_ */
