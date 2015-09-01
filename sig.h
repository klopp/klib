/*
 * sig.h, part of "klib" project.
 *
 *  Created on: 01.09.2015, 13:46
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef SIG_H_
#define SIG_H_

#include <signal.h>

typedef struct _SigNames
{
    int signo;
    const char * signame;
} *SigNames;

const char * signal_name( int signo );
const SigNames signal_names( void );


#endif /* SIG_H_ */
