/*
 *  Created on: 15 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 *  Based on: https://github.com/Jamesits/CTryCatch
 */

#include "trycatch.h"

#ifndef TRYCATCH_NESTED

jmp_buf __ex_env;
__ex_types  __ex_type = Exception;
const char *__ex_msg = ( char * ) 0;

#else

jmp_buf __ex_env[TRYCATCH_MAX + 1];
__ex_types  __ex_type[TRYCATCH_MAX + 1] = { Exception };
const char *__ex_msgs[TRYCATCH_MAX + 1] = { ( char * ) 0 };
unsigned int __ex_idx = 0;

#endif

/*
 *  That's All, Folks!
 */
