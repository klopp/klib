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

    log->flags = flags;
    log->format_fatal = '!';
    log->format_err = '*';
    log->format_warn = '?';
    log->format_info = '-';
    log->format_dbg = '#';
    log->format_datetime = 0;

    return log;
}

static void _log( FILE * file, const char * buf, const char * fmt, va_list ap )
{
    fprintf( file, "%s", buf );
    vfprintf( file, fmt, ap );
    fprintf( file, "\n" );
}

static void _log_datetime( char ** dateptr, char ** timeptr )
{
    static char tbuf[16];
    static char dbuf[16];
    struct tm *lt;
    time_t ttime;

    ttime = time( &ttime );
    lt = localtime( &ttime );
    if( dateptr )
    {
        sprintf( dbuf, "%u.%02u.%02u", lt->tm_year + 1900, lt->tm_mon + 1,
                lt->tm_mday );
        *dateptr = dbuf;
    }
    if( timeptr )
    {
        sprintf( tbuf, "%02u:%02u:%02u", lt->tm_hour, lt->tm_min, lt->tm_sec );
        *timeptr = tbuf;
    }
}

static const char * _log_datetime_separator( char separator )
{
    static char dtsep[4] =
    { 0 };
    if( separator == ' ' ) separator = 0;
    dtsep[0] = separator ? separator : ' ';
    dtsep[1] = separator ? ' ' : 0;
    return dtsep;
}

const char * log_datetime( char separator )
{
    static char datetime[32];
    char * timeptr;
    char * dateptr;

    _log_datetime( &dateptr, &timeptr );
    strcpy( datetime, dateptr );
    strcat( datetime, _log_datetime_separator( separator ) );
    strcat( datetime, timeptr );
    return datetime;
}

const char * log_date( void )
{
    char * tbuf;
    _log_datetime( NULL, &tbuf );
    return tbuf;
}

const char * log_time( void )
{
    char * dbuf;
    _log_datetime( &dbuf, NULL );
    return dbuf;
}

static int _plog( Log log, LogFlags level, const char * fmt, va_list ap )
{
    char buf[128];

    if( (level & log->flags) != level ) return 0;

    *buf = 0;

    if( log->flags & LOG_LOGPID )
    {
        sprintf( buf, "%-5u ", getpid() );
    }

    if( log->flags & LOG_LOGLEVEL )
    {
        strcat( buf, _log_format( log, level ) );
    }

    if( log->flags & (LOG_DATE | LOG_TIME) )
    {
        char * dateptr;
        char * timeptr;
        _log_datetime( &dateptr, &timeptr );
        if( log->flags & LOG_DATE )
        {
            strcat( buf, dateptr );
            strcat( buf, " " );
        }
        if( log->flags & LOG_TIME )
        {
            strcat( buf, timeptr );
            strcat( buf, " " );
        }
    }
    if( log->flags & LOG_STDOUT )
    {
        _log( stdout, buf, fmt, ap );
    }
    if( log->flags & LOG_STDERR )
    {
        _log( stderr, buf, fmt, ap );
    }
    if( *log->filename )
    {
        FILE * flog = fopen( log->filename, "a" );
        if( !flog ) return 0;
        _log( flog, buf, fmt, ap );
        fclose( flog );
    }

    return 1;
}

int plog( Log log, LogFlags level, const char * fmt, ... )
{
    int rc;
    va_list ap;
    va_start( ap, fmt );
    rc = _plog( log, level, fmt, ap );
    va_end( ap );
    va_end( ap );
    return rc;
}

int flog( Log log, const char * fmt, ... )
{
    int rc;
    va_list ap;
    va_start( ap, fmt );
    rc = _plog( log, LOG_FATAL, fmt, ap );
    va_end( ap );
    return rc;
}

int elog( Log log, const char * fmt, ... )
{
    int rc;
    va_list ap;
    va_start( ap, fmt );
    rc = _plog( log, LOG_ERROR, fmt, ap );
    va_end( ap );
    return rc;
}

int wlog( Log log, const char * fmt, ... )
{
    int rc;
    va_list ap;
    va_start( ap, fmt );
    rc = _plog( log, LOG_WARN, fmt, ap );
    va_end( ap );
    return rc;
}

int ilog( Log log, const char * fmt, ... )
{
    int rc;
    va_list ap;
    va_start( ap, fmt );
    rc = _plog( log, LOG_INFO, fmt, ap );
    va_end( ap );
    return rc;
}

int dlog( Log log, const char * fmt, ... )
{
    int rc;
    va_list ap;
    va_start( ap, fmt );
    rc = _plog( log, LOG_DBG, fmt, ap );
    va_end( ap );
    return rc;
}

