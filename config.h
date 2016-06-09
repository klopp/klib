/*
 * config.h
 *
 *  Created on: 04.05.2015
 *      Author: klopp
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#ifndef __WINDOWS__
# ifndef __unix__
#  define __WINDOWS__
#  define WIN32_LEAN_AND_MEAN
# endif
#endif

#ifndef PATH_MAX
# if defined(MAX_PATH)
#  define PATH_MAX MAX_PATH
# else
#  define PATH_MAX 4096
# endif
#endif

#if defined(_MSC_VER)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# define pid_t          int
# define strcasecmp     _stricmp
//# define snprintf       _snprintf
# define _CRT_SECURE_NO_WARNINGS
# pragma warning(disable : 4996)
#else
# include <unistd.h>
#endif

#ifndef __func__
# if defined(__FUNCTION__)
#  define __func__ __FUNCTION__
# elif defined(__FUNC__)
#  define __func__ __FUNC__
# else
#  define __func__ "<?>"
# endif
#endif

#include <stdlib.h>
#include <string.h>

#if defined(USE_MPOOL)
# include "mpool.h"
# define Malloc( size )         mp_alloc( NULL, (size) )
# define Strdup( s )            mp_strdup( NULL, (s) )
# define Calloc( size, n )      mp_calloc( NULL, (size), (n) )
# define Realloc( src, n )      mp_realloc( NULL, (src), (n) )
# define Free( ptr )            mp_free( NULL, (ptr) )
# define Munlock( ptr )         mp_unlock( NULL, (ptr) )
# define Mlock( ptr )           mp_lock( NULL, (ptr) )
#else
# define Malloc( size )         malloc( (size) )
# define Strdup( s )            strdup( (s) )
# define Calloc( size, n )      calloc( (size), (n) )
# define Realloc( src, n )      realloc( (src), (n) )
# define Free( ptr )            free( (ptr) )
# define Munlock( ptr )         (void)ptr
# define Mlock( ptr )           (void)ptr
#endif

typedef char *pchar;
typedef int *pint;
typedef long *plong;
typedef unsigned char *puchar;
typedef unsigned int *puint;
typedef unsigned long *pulong;
typedef size_t *psizet;
#if defined(__WINDOWS__)
typedef unsigned int uint;
#endif

#define forever()       for(;;)
#define is              ==
#define isnot           !=
#define not             !
#define and             &&
#define or              ||
#define unused( var )   ((void)var)

#define lambda(return_type, function_body) \
   ({ \
   return_type __fn__ function_body \
   __fn__; \
   })

#define lmbd(c_) ({ c_ _;})

#define rnd_range(min,max) (min + rand() % (max - min))


#endif /* CONFIG_H_ */
