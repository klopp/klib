/*
 * mkpath.h, part of "klib" project.
 *
 *  Created on: 05.06.2015, 16:19
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef MKPATH_H_
#define MKPATH_H_

#include "config.h"
#include <stdio.h>

#if defined(__cplusplus)
extern "C"
{
#endif

int mkpath( const char * path, mode_t mode );
FILE * openpath( const char * file, const char * mode, mode_t dirmode );
char * expand_home( const char * path );

#if defined(__cplusplus)
}
#endif

#endif /* MKPATH_H_ */
