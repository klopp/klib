/*
 * sig.c, part of "klib" project.
 *
 *  Created on: 01.09.2015, 13:46
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "sig.h"

#include <stdio.h>
#include <signal.h>

#define _SI(s)  { (s), #s },
static struct _SigNames SigData[] =
{
#if defined(SIGLOST)
        _SI(SIGLOST)
#endif
#if defined(SIGEMT)
        _SI(SIGEMT)
#endif
        _SI(SIGABRT)
        _SI(SIGALRM)
        _SI(SIGBUS)
        _SI(SIGCHLD)
        _SI(SIGCONT)
        _SI(SIGFPE)
        _SI(SIGHUP)
        _SI(SIGILL)
        _SI(SIGINT)
        _SI(SIGKILL)
        _SI(SIGPIPE)
        _SI(SIGPOLL)
        _SI(SIGPROF)
        _SI(SIGPWR)
        _SI(SIGQUIT)
        _SI(SIGSEGV)
        _SI(SIGSTKFLT)
        _SI(SIGSTOP)
        _SI(SIGSYS)
        _SI(SIGTERM)
        _SI(SIGTRAP)
        _SI(SIGTSTP)
        _SI(SIGTTIN)
        _SI(SIGTTOU)
        _SI(SIGUNUSED)
        _SI(SIGURG)
        _SI(SIGUSR1)
        _SI(SIGUSR2)
        _SI(SIGVTALRM)
        _SI(SIGWINCH)
        _SI(SIGXCPU)
        _SI(SIGXFSZ)
        { 0, NULL } };

void signal_ign_all( void )
{
    SigNames sig_data = SigData;
    while( sig_data->signo )
    {
        signal( sig_data->signo, SIG_IGN );
        sig_data++;
    }
}

void signal_def_all( void )
{
    SigNames sig_data = SigData;
    while( sig_data->signo )
    {
        signal( sig_data->signo, SIG_DFL );
        sig_data++;
    }
}

const char * signal_name( int signo )
{
    SigNames sig_data = SigData;
    static char buf[64];

    if( signo >= SIGRTMIN && signo <= SIGRTMAX )
    {
        sprintf( buf, "SIG-RT (%d)", signo );
        return buf;
    }
    while( sig_data->signo )
    {
        if( signo == sig_data->signo ) return sig_data->signame;
        sig_data++;
    }
    sprintf( buf, "SIG-UNKNOWN (%d)", signo );
    return buf;
}

const SigNames signal_names( void )
{
    return SigData;
}
