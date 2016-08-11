/*
 * lock.h
 *
 *  Created on: 3 июня 2016 г.
 *      Author: klopp
 */

#ifndef LOCK_H_
#define LOCK_H_

#ifdef USE_LOCKING
# ifndef __WINDOWS__
#  include <pthread.h>
typedef pthread_spinlock_t   __lock_type;
#  define __initlock(var)   pthread_spin_init(&var, 0)
#  define __lock(var)       pthread_spin_lock(&var)
#  define __unlock(var)     pthread_spin_unlock(&var)
# else
typedef long                 __lock_type;
#  define __initlock(var)   var = 0
#  define __lock(var)       EnterCriticalSection(&var)
#  define __unlock(var)     LeaveCriticalSection(&var)
# endif
# define __lock_t(var)     __lock_type var
#else
typedef int                  __lock_type;
#  define __lock_t(var)
#  define __initlock(var)
#  define __lock(var)
#  define __unlock(var)
#endif

#endif /* LOCK_H_ */
