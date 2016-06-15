/*
 *  Created on: 15 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 *  Based on: https://github.com/Jamesits/CTryCatch
 */

#include "trycatch.h"

jmp_buf __ex_env[TRYCATCH_MAX];
__ex_types __ex_type = Exception;
char *__ex_msg = ( char * )0;
unsigned int __ex_idx = 0;

/*
 *  That's All, Folks!
 */

