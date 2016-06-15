/*
 *  Created on: 15 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 *  Based on: https://github.com/Jamesits/CTryCatch
 */

#ifndef TRYCATCH_H_
#define TRYCATCH_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <setjmp.h>

#define TRYCATCH_MAX    16

#define TRYCATCH_CAT(a, ...) TRYCATCH_PRIMITIVE_CAT(a, __VA_ARGS__)
#define TRYCATCH_PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define try \
    if(!(__ex_type = setjmp(__ex_env[__ex_idx++])))

#define catch(X) \
    else if((X +0) == 0 || __ex_type == (X +0))

#define finally

#define throw(X,...) \
    __ex_msg = (__VA_ARGS__  +0), longjmp(__ex_env[--__ex_idx], (X))

typedef enum {
    /*
     *   Caution: 0 **IS** defined as "no error" to make it work.
     *   DO NOT modify this line.
     */
    Exception,

#include "trycatch.conf"
}
__ex_types;

extern char *__ex_msg;
extern jmp_buf __ex_env[];
extern unsigned int __ex_idx;
extern __ex_types __ex_type;

#if defined(__cplusplus)
}; /* extern "C" */
#endif


#endif /* TRYCATCH_H_ */

/*
 *  That's All, Folks!
 */
