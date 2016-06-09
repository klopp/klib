/*
 * lock.h
 *
 *  Created on: 3 июня 2016 г.
 *      Author: klopp
 */

#ifndef LOCK_H_
#define LOCK_H_

void _lock(int *lock);
#define _unlock( lock ) lock = 0


#endif /* LOCK_H_ */
