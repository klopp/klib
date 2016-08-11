/*
 *  Created on: 6 июл. 2016 г.
 *      Author: klopp
 */

/*
 *  That's All, Folks!
 */
#ifndef T_DIFF_H_
#define T_DIFF_H_

#include <sys/time.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Simple usage:
 *
 *      struct timeval start;
 *      gettimeofday( &start, NULL );
 *
 *      ... some code ...
 *
 *      unsigned long nanoseconds = t_diff( &start, NULL );
 */
unsigned long t_diff( struct timeval *start, struct timeval *end );

#if defined(__cplusplus)
}; /* extern "C" */
#endif


#endif /* T_DIFF_H_ */
