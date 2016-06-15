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
    __ex_idx++; if(!(__ex_type[__ex_idx-1] = setjmp(__ex_env[__ex_idx-1])))

#define catch(X) \
    else if((X+Exception) == Exception || __ex_type[__ex_idx-1] == (X+Exception))

#define throw(X,...) \
    __ex_msgs[__ex_idx-1] = (__VA_ARGS__  + 0), longjmp(__ex_env[__ex_idx-1], (X))

/*
 *  finally block MUST be called, always!
 */
#define finally __ex_idx--;

#define __ex_msg __ex_msgs[__ex_idx-1]

typedef enum {
    /*
     *   Caution: 0 **IS** defined as "no error" to make it work.
     *   DO NOT modify this line.
     */
    Exception,

#include "trycatch.conf"
}
__ex_types;

extern char *__ex_msgs[];
extern jmp_buf __ex_env[];
extern __ex_types __ex_type[];
extern unsigned int __ex_idx;

#if defined(__cplusplus)
}; /* extern "C" */
#endif


#endif /* TRYCATCH_H_ */

/*
 *  That's All, Folks!
 */
