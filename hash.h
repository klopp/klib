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

unsigned hash_faq6( unsigned startval, const char * buf, size_t size );
unsigned hash_rot13( unsigned startval, const char * buf, size_t size );
unsigned hash_ly( unsigned startval, const char * buf, size_t size );
unsigned hash_rs( unsigned startval, const char * buf, size_t size );

#define shash_faq6(s)   hash_faq6( 0, (s), strlen((s)) )
#define shash_rot13(s)  hash_rot13( 0, (s), strlen((s)) )
#define shash_ly(s)     hash_ly( 0, (s), strlen((s)) )
#define shash_rs(s)     hash_rs( 0, (s), strlen((s)) )

#ifdef __cplusplus
}
#endif

#endif /* HASH_H_ */
