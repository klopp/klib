/*
 * lock.h
 *
 *  Created on: 3 июня 2016 г.
 *      Author: klopp
 */

#ifndef LOCK_H_
#define LOCK_H_

#ifndef __WINDOWS__
# include <pthread.h>
typedef pthread_spinlock_t __lock_t;
# define __lock(lock)    pthread_spin_lock(&lock)
# define __unlock(lock)  pthread_spin_unlock(&lock)
#else
# typedef __lock_t       long
# define __lock(lock)    EnterCriticalSection(&lock)
# define __unlock(lock)  LeaveCriticalSection(&lock)
#endif

#endif /* LOCK_H_ */
