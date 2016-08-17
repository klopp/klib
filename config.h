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
# define _CRT_SECURE_NO_WARNINGS
# pragma warning(disable : 4996)
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

#ifndef __KERNEL__
# define __init
# define __exit
# define module_init(a)
# define module_exit(a)
# define MODULE_LICENSE(a)
# define MODULE_AUTHOR(a)
# define EXPORT_SYMBOL(a)
#endif

#if defined(__unix__)
# ifndef __KERNEL__
#  include <unistd.h>
#  include <stdlib.h>
#  include <string.h>
#  include <stdio.h>
# endif
#endif

#ifndef NULL
# define NULL (void *)0
#endif

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
# if defined(__KERNEL__)
#  include <linux/module.h>
#  include <linux/slab.h>
extern void *_k_calloc( int, int );
extern char *_k_strdup( char * );
#  define Malloc(sz)             kmalloc( (sz), GFP_KERNEL )
#  define Strdup( s )            _k_strdup( (s) )
#  define Calloc(sz,n)           _k_calloc( (sz), (n) )
#  define Realloc(ptr,sz)        krealloc( (ptr), (sz), GFP_KERNEL )
#  define Free(ptr)              kfree( (ptr) )
#  define Munlock( ptr )         (void)ptr
#  define Mlock( ptr )           (void)ptr
#  define printf printk
# else
#  define Malloc( size )         malloc( (size) )
#  define Strdup( s )            strdup( (s) )
#  define Calloc( size, n )      calloc( (size), (n) )
#  define Realloc( src, n )      realloc( (src), (n) )
#  define Free( ptr )            free( (ptr) )
#  define Munlock( ptr )         (void)ptr
#  define Mlock( ptr )           (void)ptr
# endif
#endif

#define forever()       for(;;)
#define unused( var )   ((void)var)

#define lambda(return_type, function_body) \
    ({ \
        return_type __fn__ function_body \
        __fn__; \
    })

#define lmbd(c_) ({ c_ _;})

#define rnd_range(min,max) ((min) + rand() % ((max) - (min)))


#endif /* CONFIG_H_ */
