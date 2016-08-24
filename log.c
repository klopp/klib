/*`
 *  Created on: 24 авг. 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "log.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>

static int _log_get_handle( LogInfo log )
{
    if( !log->file || !log->file[0] ) {
        return fileno( stdout );
    }

    if( !log->file[1] ) {
        if( log->file[0] == '-' ) {
            return fileno( stdout );
        }

        if( log->file[0] == '=' ) {
            return fileno( stderr );
        }
    }

    return open( log->file, O_APPEND | O_CREAT );
}

static void _log_flush( LogInfo log )
{
    if( log->buf && log->in_buf ) {
        int handle = _log_get_handle( log );

        if( handle >= 0 ) {
            write( handle, log->buf, log->in_buf );
            log->in_buf = 0;
            close( handle );
        }
    }
}

LogInfo log_create( LOG_LEVEL level, const char *file, const char *format,
                    size_t buf_size )
{
    LogInfo log = Calloc( sizeof( struct _LogInfo ), 1 );

    if( !log ) {
        return NULL;
    }

    log->ibuf_size = LOG_BUF_MIN_SIZE;
    log->ibuf = Malloc( LOG_BUF_MIN_SIZE + 1 );

    if( !log->ibuf ) {
        Free( log );
        return 0;
    }

    if( buf_size ) {
        log->buf_size = buf_size < LOG_BUF_MIN_SIZE ? LOG_BUF_MIN_SIZE : buf_size;
        log->buf = Malloc( log->buf_size );

        if( !log->buf ) {
            Free( log->ibuf );
            Free( log );
            return NULL;
        }
    }

    log->format = Strdup( format ? format : LOG_DEFAULT_FORMAT );

    if( !log->format ) {
        Free( log->ibuf );
        Free( log->buf );
        Free( log );
        return NULL;
    }

    if( file ) {
        log->file = Strdup( file );

        if( !log->file ) {
            Free( log->ibuf );
            Free( log->format );
            Free( log->buf );
            Free( log );
            return NULL;
        }
    }

    log->level = level;
    __initlock( log->lock );
    return log;
}

void log_destroy( LogInfo log )
{
    _log_flush( log );
    Free( log->format );
    Free( log->file );
    Free( log->ibuf );
    Free( log->buf );
    Free( log );
}

static struct tm *_log_init_time( struct tm *tnow )
{
    if( !tnow ) {
        time_t tt = time( NULL );
        return localtime( &tt );
    }

    return tnow;
}

static int _log_check_ibuf( LogInfo log, size_t size )
{
    if( size >= log->ibuf_size ) {
        char *ptr = Realloc( log->ibuf, ( size * 2 ) + 1 );

        if( ptr ) {
            log->ibuf_size = size * 2;
            Free( log->ibuf );
            log->ibuf = ptr;
            log->ibuf[size] = 0;
        }
        else {
            return 0;
        }
    }

    return 1;
}

static size_t _log_cat_ibuf( LogInfo log, const char *buf, size_t size )
{
    size_t blen = strlen( buf );

    if( blen ) {
        if( size + blen >= log->ibuf_size ) {
            char *ptr = Realloc( log->ibuf, ( ( size + blen ) * 2 ) + 1 );

            if( !ptr ) {
                return 0;
            }

            Free( log->ibuf );
            log->ibuf = ptr;
        }

        memcpy( log->ibuf + size, buf, blen );
        size += blen - 1;
    }

    return size;
}

static size_t _log_make_prefix( LogInfo log )
{
    const char *fmt = log->format;
    size_t size = 0;
    struct tm *tnow = NULL;
    char buf[0x40];

    while( *fmt ) {
        if( *fmt != '%' ) {
            log->ibuf[size] = *fmt;
        }
        else {
            fmt++;

            switch( *fmt ) {
                case 'p':
                    sprintf( buf, "%u", getpid() );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'l':
                    break;

                case 'L':
                    break;

                case 'd':
                    tnow = _log_init_time( tnow );
                    sprintf( buf, "%02u", tnow->tm_mday );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'm':
                    tnow = _log_init_time( tnow );
                    sprintf( buf, "%02u", tnow->tm_mon + 1 );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'y':
                    tnow = _log_init_time( tnow );
                    sprintf( buf, "%04u", tnow->tm_year + 1900 );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'H':
                    tnow = _log_init_time( tnow );
                    sprintf( buf, "%02u", tnow->tm_hour );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'M':
                    tnow = _log_init_time( tnow );
                    sprintf( buf, "%02u", tnow->tm_min );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'S':
                    tnow = _log_init_time( tnow );
                    sprintf( buf, "%02u", tnow->tm_sec );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'X':
                    tnow = _log_init_time( tnow );
                    sprintf( buf, "%02u:%02u:%02u", tnow->tm_hour, tnow->tm_min, tnow->tm_sec );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'Y':
                    tnow = _log_init_time( tnow );
                    sprintf( buf, "%02u.%02u.%04u", tnow->tm_mday, tnow->tm_mon + 1,
                             tnow->tm_year + 1900 );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'Z':
                    tnow = _log_init_time( tnow );
                    sprintf( buf, "%02u.%02u.%04u %02u:%02u:%02u", tnow->tm_mday, tnow->tm_mon + 1,
                             tnow->tm_year + 1900,
                             tnow->tm_hour, tnow->tm_min, tnow->tm_sec );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                default:
                    log->ibuf[size] = *fmt;
                    break;
            }
        }

        fmt++;
        size++;

        if( !_log_check_ibuf( log, size ) ) {
            return 0;
        }
    }

    log->ibuf[size] = 0;
    return size;
}

static void _log( LogInfo log, LOG_LEVEL level, const char *fmt, va_list ap )
{
    if( !_log_make_prefix( log ) ) {
        return;
    }
}

void ilog( LogInfo log, const char *fmt, ... )
{
    va_list ap;
    va_start( ap, fmt );
    _log( log, LOG_INFO, fmt, ap );
    va_end( ap );
}

/*
 *  That's All, Folks!
 */
