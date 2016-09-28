/*
 *  Created on: 6 июл. 2016 г.
 *      Author: klopp
 */

#ifndef T_DIFF_H_
#define T_DIFF_H_

#include "config.h"
#include <sys/time.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Simple usage:
 *
 *      struct timeval start;
 *      t_diff_start( &start );
 *
 *      OR

 *      struct timeval *start = t_diff_start(NULL):
 *
 *      ... some code ...
 *
 *      unsigned long microseconds = t_diff( &start, NULL );
 */
struct timeval *t_diff_start( struct timeval *start );
unsigned long t_diff( struct timeval *start, struct timeval *end );

#if defined(__cplusplus)
}; /* extern "C" */
#endif


#endif /* T_DIFF_H_ */

/*
 *  That's All, Folks!
 */
