/*
 *  Created on: 6 июл. 2016 г.
 *      Author: klopp
 */

#include "t_diff.h"

/* ---------------------------------------------------------------------------*/
unsigned long t_diff( struct timeval *start, struct timeval *end ) {
    struct timeval lend;

    if( !end ) {
        gettimeofday( &lend, 0 );
        end = &lend;
    }

    return ( end->tv_sec - start->tv_sec ) * 1000000 + end->tv_usec -
           start->tv_usec;
}

/* ---------------------------------------------------------------------------*/

/*
 *  That's All, Folks!
 */

