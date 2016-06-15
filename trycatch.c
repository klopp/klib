/*
 *  Created on: 15 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "trycatch.h"

jmp_buf CTRYCATCH_NAME(env);
CTRYCATCH_NAME(types) CTRYCATCH_NAME(type);
char *CTRYCATCH_NAME(msg);

/*
 *  That's All, Folks!
 */

