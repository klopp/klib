/*
 * sha256.h
 *
 *  Created on: 12 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 *  Source code: http://www.zedwood.com/article/cpp-sha256-function
 */

#ifndef SHA256_H_
#define SHA256_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "config.h"
#include <stdlib.h>
#include <string.h>

#define SHA224_256_BLOCK_SIZE   (512 / 8)
#define SHA256_DIGEST_SIZE      (256 / 8)

typedef struct {
    unsigned int m_tot_len;
    unsigned int m_len;
    unsigned char m_block[2 * SHA224_256_BLOCK_SIZE];
    unsigned int m_h[8];
} SHA256;

SHA256 *sha256_init( SHA256 *sha256 );
void sha256_update( SHA256 *sha256, const unsigned char *message,
                    unsigned int len );
unsigned char *sha256_finalize( SHA256 *sha256, unsigned char *digest );

#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (unsigned char) ((x)      );       \
    *((str) + 2) = (unsigned char) ((x) >>  8);       \
    *((str) + 1) = (unsigned char) ((x) >> 16);       \
    *((str) + 0) = (unsigned char) ((x) >> 24);       \
}
#define SHA2_PACK32(str, x)                   \
{                                             \
    *(x) =   ((unsigned int) *((str) + 3)      )    \
           | ((unsigned int) *((str) + 2) <<  8)    \
           | ((unsigned int) *((str) + 1) << 16)    \
           | ((unsigned int) *((str) + 0) << 24);   \
}

#if defined(__cplusplus)
};
#endif


#endif
