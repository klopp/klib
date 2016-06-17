/*
 * st_trace.h, part of "klib" project.
 *
 *  Created on: 30.05.2016, 16:54
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef ST_TRACE_H_
#define ST_TRACE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef NDEBUG

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "va_arg.h"

#define return(...)    JOIN(return,__VA_NARG__(__VA_ARGS__))(__VA_ARGS__)

#define _ST_MAX         0x400
#define _ST_FILE_MAX    PATH_MAX
#define _ST_NAME_MAX    0x100
#define _ST_ARG_MAX     0x400
#define _ST_RET_MAX     0x100

typedef void ( *st_dumper )( const char *ret, const char *name, const char *arg,
                             const char *file, unsigned line, FILE *handle );
/*
 * dumper:
 *   if NULL default dumper will be used
 * file:
 *   if NULL STDERR will be used
 * tab:
 *   if NULL no indentation will be used
 * number:
 *   1 - print line numbers (1, 2, 3 ...)
 *   2 - print line numbers with dot (1., 2., 3. ...)
 */
void st_dump( const st_dumper dumper, FILE *file, const char *tab,
              int numbers );

/*
 * internal functions:
 */
size_t _st_push( const char *ret, const char *name, const char *arg,
                 const char *file, unsigned line );
void _st_pop( void );
void _st_init( void );

/*
 *  ret function( args )
 */
#define f_start( ret, func, args )  ret func args { \
                                    _st_push( #ret, #func, #args,  \
                                    __FILE__, __LINE__ );

#define return0()                   { _st_pop(); return; }
#define return1( ret )              { _st_pop(); return(ret); }

#else /* NDEBUG */

/*
 *  ret function( args )
 */
#define f_start( ret, func, args )  ret function args {

#define _return0()                  { return; }
#define _return1( ret )             { return(ret); }

#define _st_dump(...)               (void)0

#endif /* NDEBUG */

/*
 * void function( args )
 */
#define f_vstart( function, args )  f_start( void, function, args )

/*
 * ret function()
 */
#define f_startv( ret, function )   f_start( ret, function, () )

/*
 * void function()
 */
#define f_vstartv( function )       f_start( void, function, () )

#define _main()                     int main(int argc, char **argv, char **env) { \
                                    _st_init(); \
                                    _st_push( "int", "main", "argc, argv, env", \
                                    __FILE__, __LINE__); \
                                    unused(argc); unused(argv); unused(env);

#ifdef __cplusplus
}
#endif

#endif /* ST_TRACE_H_ */
