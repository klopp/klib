/*
 * crc.h
 *
 *  Created on: 11 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef CRC_H_
#define CRC_H_

#include <string.h>

unsigned short crc16(const char *buf, size_t len);
#define scrc16(buf) crc16( (buf),strlen( (buf) ) )
#define xcrc16(buf) ( crc16( (buf),strlen( (buf) ) ) ^ 0xFFFF )
#define xscrc16(buf) ( scrc16( (buf) ) ^ 0xFFFF )

unsigned int crc32(const char *buf, size_t len);
#define scrc32(buf) crc32( (buf),strlen( (buf) ) )
#define xcrc32(buf) ( crc32( (buf),strlen( (buf) ) ) ^ 0xFFFFFFFF)
#define xscrc32(buf) ( scrc32( (buf) ) ^ 0xFFFFFFFF)

#endif /* CRC_H_ */
