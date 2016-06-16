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
#include <assert.h>

#define TRYCATCH_MAX    32

#define try \
        assert(__ex_idx < TRYCATCH_MAX); __ex_idx++; \
            if(!(__ex_type[__ex_idx-1] = setjmp(__ex_env[__ex_idx-1])))

#define catch(X) \
        else if(((X)+Exception) == Exception || \
                __ex_type[__ex_idx-1] == ((X)+Exception))

#define throw(X,...) \
        __ex_msgs[__ex_idx-1] = (__VA_ARGS__+0), \
            longjmp(__ex_env[__ex_idx-1], (X))

/*
 *  finally block MUST be called, always!
 */
#define finally \
        assert(__ex_idx); __ex_idx--; __ex_type[__ex_idx] = Exception; \
            __ex_msgs[__ex_idx] = (const char *)0

#if defined(NDEBUG)
# define __ex_have_msg (__ex_idx && __ex_idx < TRYCATCH_MAX ) ? \
         __ex_msgs[__ex_idx-1] : 0
#else
# define __ex_have_msg __ex_msgs[__ex_idx-1]
#endif

#define __ex_with_msg   __ex_have_msg

#define __ex_msg        __ex_msgs[__ex_idx-1]

typedef enum {
    Exception = 0,
#include "trycatch.conf"
}
__ex_types;

extern const char *__ex_msgs[];
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
