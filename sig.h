/*
 * sig.h, part of "klib" project.
 *
 *  Created on: 01.09.2015, 13:46
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef SIG_H_
#define SIG_H_

#include <signal.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _SigNames
{
    int signo;
    const char * signame;
} *SigNames;

const char * signal_name( int signo );
const SigNames signal_names( void );
void signal_ign_all( void  );
void signal_def_all( void  );

#ifdef __cplusplus
};
#endif

#endif /* SIG_H_ */
