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

unsigned long t_diff(struct timeval *start, struct timeval *end);

#if defined(__cplusplus)
  }; /* extern "C" */
#endif


#endif /* T_DIFF_H_ */
