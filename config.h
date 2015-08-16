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
# define Munlock( ptr )         NO_Munlock_implementation
# define Mlock( ptr )           NO_Mlock_implementation
#endif

typedef char *              pchar;
typedef int *               pint;
typedef long *              plong;
typedef unsigned char *     puchar;
typedef unsigned int *      puint;
typedef unsigned long *     pulong;
typedef size_t *            psizet;
#if defined(__WINDOWS__)
typedef unsigned int      uint;
#endif

#ifndef forever
# define forever while(1)
#endif

/*
typedef union var
{
    char c;
    unsigned char uc;
    short s;
    unsigned short us;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;

    void * v;

    char * pc;
    unsigned char * puc;
    short * ps;
    unsigned short * pus;
    int * pi;
    unsigned int * pui;
    long * pl;
    unsigned long * pul;
} var;
*/

#endif /* CONFIG_H_ */
