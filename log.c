/*
 * log.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 05:58
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "log.h"
#include <limits.h>

static const char * _log_format( Log log, LogFlags level )
{
    static char buf[8];
    static char fmt[] = "[%c] ";

    switch( level )
    {
        case LOG_FATAL:
            sprintf( buf, fmt, log->format_fatal );
            break;
        case LOG_ERROR:
            sprintf( buf, fmt, log->format_err );
            break;
        case LOG_WARN:
            sprintf( buf, fmt, log->format_warn );
            break;
        case LOG_INFO:
            sprintf( buf, fmt, log->format_info );
            break;
        case LOG_DBG:
            sprintf( buf, fmt, log->format_dbg );
            break;
        default:
            sprintf( buf, fmt, ' ' );
            break;
    }
    return buf;
}

int log_flags( Log log, LogFlags flags )
{
    if( !log ) return 0;

    log->flags = flags;
    log->format_fatal = '!';
    log->format_err = '*';
    log->format_warn = '?';
    log->format_info = '-';
    log->format_dbg = '#';
    return 1;
}

void log_destroy( Log log )
{
    free( log );
}

Log log_create( const char * filename, LogFlags flags )
{
    Log log = calloc( sizeof(struct _Log), 1 );
    if( !log ) return NULL;
    if( filename ) strncpy( log->filename, filename,
            sizeof(log->filename) - 1 );
    log_flags( log, flags );

    return log;
}

static void _log( FILE * file, const char * buf, const char * fmt, ... )
{
    va_list ap;
    fprintf( file, "%s", buf );
    va_start( ap, fmt );
    vfprintf( file, fmt, ap );
    va_end( ap );
    fprintf( file, "\n" );
}

const char * log_date( void )
{
    static char tbuf[32];
    struct tm *lt;
    time_t ttime;

    ttime = time( &ttime );
    lt = localtime( &ttime );
    sprintf( tbuf, "%u.%02u.%02u", lt->tm_year + 1900, lt->tm_mon + 1,
            lt->tm_mday );
    return tbuf;
}

const char * log_time( void )
{
    static char tbuf[32];
    struct tm *lt;
    time_t ttime;

    ttime = time( &ttime );
    lt = localtime( &ttime );
    sprintf( tbuf, "%02u:%02u:%02u", lt->tm_hour, lt->tm_min, lt->tm_sec );
    return tbuf;
}

int plog( Log log, LogFlags level, const char * fmt, ... )
{
    char buf[128];
    char tbuf[32];

    if( (level & log->flags) != level ) return 0;

    *buf = 0;

    if( log->flags & LOG_LOGPID )
    {
        sprintf( tbuf, "%-5u ", getpid() );
        strcat( buf, tbuf );
    }

    if( log->flags & LOG_LOGLEVEL )
    {
        strcat( buf, _log_format( log, level ) );
    }

    if( log->flags & LOG_DATE )
    {
        strcat( buf, log_date() );
        strcat( buf, " " );
    }
    if( log->flags & LOG_TIME )
    {
        strcat( buf, log_time() );
        strcat( buf, " " );
    }

    if( log->flags & LOG_STDOUT )
    {
        _log( stdout, buf, fmt );
    }
    if( log->flags & LOG_STDERR )
    {
        _log( stdout, buf, fmt );
    }
    if( *log->filename )
    {
        FILE * flog = fopen( log->filename, "a" );
        if( !flog ) return 0;
        _log( flog, buf, fmt );
        fclose( flog );
    }

    return 1;
}

