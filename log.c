/*
 * log.c, part of "klib" project.
 *
 *  Created on: 24.05.2015, 05:58
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "log.h"
#include "mkpath.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/stat.h>
#ifndef __WINDOWS__
#include <sys/time.h>
#else
#include <process.h>
#endif

static const char * _log_format_level( Log log, LogFlags level )
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
    Free( log->file );
    Free( log );
}

static size_t _log_format_string( char ** data, const char * fmt, va_list ap )
{
    size_t size = 0;
    int pad = 0;
    size_t workhorse = 0;
    char buf[32];
    char * ptr = data ? *data : NULL;
    time_t ttime = time( &ttime );
    struct tm * lt = localtime( &ttime );

    while( *fmt )
    {
        if( *fmt != '%' )
        {
            if( ptr )
            {
                *ptr = *fmt;
                ptr++;
            }
            fmt++;
            size++;
            continue;
        }

        fmt++;
        if( isdigit( *fmt ) )
        {
            do
            {
                pad = pad * 10 + (*fmt - '0');
                fmt++;
            } while( isdigit( *fmt ) );
        }
        else
        {
            pad = 0;
        }

        switch( *fmt )
        {
            case 'P':
                sprintf( buf, "%0*u", pad, getpid() );
                goto pcopy;

            case 'U':
                do
                {
                    unsigned int u = va_arg( ap, uint );
                    sprintf( buf, "%0*u", pad, u );
                } while( 0 );
                goto pcopy;

            case 'S':
                do
                {
                    char * s = va_arg( ap, pchar );
                    workhorse = strlen( s );
                    if( ptr )
                    {
                        memcpy( ptr, s, workhorse );
                        ptr += workhorse;
                    }
                    size += workhorse;
                    *buf = 0;
                } while( 0 );
                break;

            case 'Y':
                sprintf( buf, "%u", lt->tm_year + 1900 );
                goto pcopy;

            case 'M':
                sprintf( buf, "%02u", lt->tm_mon + 1 );
                goto pcopy;

            case 'D':
                sprintf( buf, "%02u", lt->tm_mday );
                goto pcopy;

            case 'h':
                sprintf( buf, "%02u", lt->tm_hour );
                goto pcopy;

            case 'm':
                sprintf( buf, "%02u", lt->tm_min );
                goto pcopy;

            case 's':
                sprintf( buf, "%02u", lt->tm_sec );
                goto pcopy;

            case 'z':
                sprintf( buf, "%u.%02u.%02u-%02u.%02u.%02u", lt->tm_year + 1900,
                        lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min,
                        lt->tm_sec );
                goto pcopy;

            case 'Z':
                sprintf( buf, "%u-%02u-%02u-%02u-%02u-%02u", lt->tm_year + 1900,
                        lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min,
                        lt->tm_sec );
                goto pcopy;

            case 'b':
                sprintf( buf, "%02u.%02u.%02u", lt->tm_hour, lt->tm_min,
                        lt->tm_sec );
                goto pcopy;

            case 'B':
                sprintf( buf, "%02u-%02u-%02u", lt->tm_hour, lt->tm_min,
                        lt->tm_sec );
                goto pcopy;

            case 'a':
                sprintf( buf, "%u.%02u.%02u", lt->tm_year + 1900,
                        lt->tm_mon + 1, lt->tm_mday );
                goto pcopy;

            case 'A':
                sprintf( buf, "%u-%02u-%02u", lt->tm_year + 1900,
                        lt->tm_mon + 1, lt->tm_mday );
                goto pcopy;

            default:
                *buf = *fmt;
                *(buf + 1) = 0;
                goto pcopy;
        }
        pcopy: workhorse = strlen( buf );
        if( ptr && workhorse )
        {
            memcpy( ptr, buf, workhorse );
            ptr += workhorse;
        }
        size += workhorse;
        fmt++;
    }
    if( ptr )
    {
        *ptr = 0;
    }
    return size;
}

Log log_create( LogFlags flags, const char * filename, ... )
{
    Log log = Calloc( sizeof(struct _Log), 1 );
    if( !log ) return NULL;

    if( filename )
    {
        va_list ap;
        va_list op;
        size_t size;

        va_start( ap, filename );
        size = _log_format_string( NULL, filename, ap );
        va_end( ap );

        log->file = Malloc( size + 1 );
        if( log->file )
        {
            char * fullpath;
            va_start( op, filename );
            _log_format_string( &log->file, filename, op );
            va_end( op );
            fullpath = expand_home( log->file );
            if( fullpath )
            {
                Free( log->file );
                log->file = fullpath;
            }
        }
        if( !log->file )
        {
            Free( log );
            return NULL;
        }
    }

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

/*
 #define _log( file, prefix, data ) \
        fprintf( (file), "%s", (prefix) ); \
        fprintf( (file), "%s", data ); \
        fprintf( (file), "\n" )
 */

static const char * _log_datetime_separator( char separator )
{
    static char dtsep[4] =
    { 0 };
    if( separator == ' ' ) separator = 0;
    dtsep[0] = separator ? separator : ' ';
    dtsep[1] = separator ? ' ' : 0;
    return dtsep;
}

/*
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
 */

static int _plog( Log log, LogFlags level, const char * fmt, va_list ap )
{
    char buf[128];
    va_list cp;

//    char * msg = _ssprintf( NULL, fmt, ap );
//    if( !msg ) return 0;

    if( (level & log->flags) != level ) return 0;

    *buf = 0;

    if( log->flags & LOG_LOGPID )
    {
        sprintf( buf, "%-5u ", getpid() );
    }

    if( log->flags & LOG_LOGLEVEL )
    {
        strcat( buf, _log_format_level( log, level ) );
    }

    if( log->flags & (LOG_DATE | LOG_TIME | LOG_MILLISEC) )
    {
        char dateptr[16];
        char timeptr[16];
        char mseconds[16];
#ifndef __WINDOWS__
        struct tm *lt;
        struct timeval tv;

        gettimeofday( &tv, NULL );
        lt = localtime( (time_t *)&tv.tv_sec );

        sprintf( dateptr, "%u.%02u.%02u", lt->tm_year + 1900, lt->tm_mon + 1,
                lt->tm_mday );
        sprintf( timeptr, "%02u:%02u:%02u", lt->tm_hour, lt->tm_min,
                lt->tm_sec );
        sprintf( mseconds, "%-3lu", tv.tv_usec / 1000 );
#else
		SYSTEMTIME lt;
		GetLocalTime(&lt);

		sprintf(dateptr, "%u.%02u.%02u", lt.wYear, lt.wMonth,
			lt.wDay);
		sprintf(timeptr, "%02u:%02u:%02u", lt.wHour, lt.wMinute,
			lt.wSecond);
		sprintf(mseconds, "%-3lu", lt.wMilliseconds);
#endif
        if( log->flags & (LOG_DATE | LOG_TIME | LOG_MILLISEC) )
        {
            if( log->flags & LOG_DATE )
            {
                strcat( buf, dateptr );
                if( log->flags & (LOG_TIME | LOG_MILLISEC) )
                {
                    strcat( buf,
                            _log_datetime_separator( log->format_datetime ) );

                }
            }
            if( log->flags & (LOG_TIME | LOG_MILLISEC) )
            {
                strcat( buf, timeptr );
            }
            if( log->flags & LOG_MILLISEC )
            {
                strcat( buf, "." );
                strcat( buf, mseconds );
            }
            strcat( buf, " " );
        }
    }

    if( log->flags & LOG_STDOUT )
    {
        va_copy( cp, ap );
        _log( stdout, buf, fmt, ap );
        va_copy( ap, cp );
    }
    if( log->flags & LOG_STDERR )
    {
        va_copy( cp, ap );
        _log( stderr, buf, fmt, ap );
        va_copy( ap, cp );
    }
    if( *log->file )
    {
        FILE * flog = fopen( log->file, "a" );
        if( !flog ) return 0;
        _log( flog, buf, fmt, ap );
        fclose( flog );
    }

//    Free( msg );

    return 1;
}

int plog( Log log, LogFlags level, const char * fmt, ... )
{
    int rc;
    va_list ap;
    va_start( ap, fmt );
    rc = _plog( log, level, fmt, ap );
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

