/*
 *  Created on: 15 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

/*
 *  That's All, Folks!
 */
#ifndef TRYCATCH_H_
#define TRYCATCH_H_

#if defined(__cplusplus)
  extern "C" {
#endif

#include <setjmp.h>

#define CTRYCATCH_CAT(a, ...) CTRYCATCH_PRIMITIVE_CAT(a, __VA_ARGS__)
#define CTRYCATCH_PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__
#define CTRYCATCH_NAME(X) CTRYCATCH_CAT(__ex_, X)

#define try \
    if(!(CTRYCATCH_NAME(type) = setjmp(CTRYCATCH_NAME(env))))

#define catch(X) \
    else if((X +0) == 0 || CTRYCATCH_NAME(type) == (X +0))

#define finally

#define throw(X,...) \
    CTRYCATCH_NAME(msg) = (__VA_ARGS__  +0), longjmp(CTRYCATCH_NAME(env), (X))

typedef int CTRYCATCH_NAME(types);

enum type {
    /*
     *   Caution: 0 **IS** defined as "no error" to make it work.
     *   DO NOT modify this line.
     */
    Exception,

#include "trycatch.conf"
};


#if defined(__cplusplus)
  }; /* extern "C" */
#endif


#endif /* TRYCATCH_H_ */
