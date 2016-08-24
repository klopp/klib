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

static const char *_log_long_title( LOG_LEVEL level )
{
    size_t i = 0;
    static struct {
        LOG_LEVEL level;
        const char *title;
    } _log_long_titles[] = { { LOG_LEVEL_DEBUG, "debug" }, { LOG_LEVEL_INFO, "info " }, { LOG_LEVEL_WARN, "warn " }, {
            LOG_LEVEL_ERROR,
            "error"
        }, { LOG_LEVEL_FATAL, "fatal" }
    };

    while( i < sizeof( _log_long_titles ) / sizeof( _log_long_titles[0] ) ) {
        if( level == _log_long_titles[i].level ) {
            return _log_long_titles[i].title;
        }

        i++;
    }

    return "log";
}

static const char *_log_short_title( LOG_LEVEL level )
{
    size_t i = 0;
    static struct {
        LOG_LEVEL level;
        const char *title;
    } _log_short_titles[] = { { LOG_LEVEL_DEBUG, "#" }, { LOG_LEVEL_INFO, "i" }, { LOG_LEVEL_WARN, "?" }, { LOG_LEVEL_ERROR, "!" }, {
            LOG_LEVEL_FATAL, "*"
        }
    };

    while( i < sizeof( _log_short_titles ) / sizeof( _log_short_titles[0] ) ) {
        if( level == _log_short_titles[i].level ) {
            return _log_short_titles[i].title;
        }

        i++;
    }

    return "@";
}

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

            if( handle != fileno( stdout ) && handle != fileno( stderr ) ) {
                close( handle );
            }
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

    log->ibuf_size = LOG_IBUF_MIN_SIZE;
    log->ibuf = Malloc( LOG_IBUF_MIN_SIZE + 1 );

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

    log->prefix = ( format ? ( *format ? Strdup( format ) : NULL ) : Strdup(
                            LOG_DEFAULT_PREFIX ) );

    if( ( ( format && *format ) || !format ) && !log->prefix ) {
        Free( log->ibuf );
        Free( log->buf );
        Free( log );
        return NULL;
    }

    if( file ) {
        log->file = Strdup( file );

        if( !log->file ) {
            Free( log->ibuf );
            Free( log->prefix );
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
    Free( log->prefix );
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

            log->ibuf = ptr;
            log->ibuf_size = ( size + blen ) * 2;
        }

        memcpy( log->ibuf + size, buf, blen );
        size += blen - 1;
    }

    return size;
}

static void _log_cat_buf( LogInfo log, const char *buf, size_t blen )
{
    while( blen ) {
        size_t to_copy = blen;

        if( log->in_buf + blen >= log->buf_size ) {
            to_copy = log->buf_size - log->in_buf;
            memcpy( log->buf + log->in_buf, buf, to_copy );
            log->in_buf += to_copy;
            _log_flush( log );
        }
        else {
            memcpy( log->buf + log->in_buf, buf, blen );
            log->in_buf += to_copy;
        }

        buf += to_copy;
        blen -= to_copy;
    }
}

static size_t _log_make_prefix( LogInfo log, LOG_LEVEL level )
{
    const char *fmt = log->prefix;
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
                    if( !( size = _log_cat_ibuf( log, _log_short_title( level ), size ) ) ) {
                        return 0;
                    }

                    break;

                case 'L':
                    if( !( size = _log_cat_ibuf( log, _log_long_title( level ), size ) ) ) {
                        return 0;
                    }

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
                             tnow->tm_year + 1900, tnow->tm_hour, tnow->tm_min, tnow->tm_sec );

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

void plog( LogInfo log, LOG_LEVEL level, const char *fmt, ... )
{
    if( log->level & level ) {
        int handle = -1;
        size_t size;
        __lock( log->lock );

        if( log->prefix ) {
            if( !( size = _log_make_prefix( log, level ) ) ) {
                __unlock( log->lock );
                return;
            }

            if( !log->buf ) {
                handle = _log_get_handle( log );

                if( handle < 0 ) {
                    __unlock( log->lock );
                    return;
                }

                write( handle, log->ibuf, size );
            }
            else {
                _log_cat_buf( log, log->ibuf, size );
            }
        }

        while( 1 ) {
            char *ptr;
            int n;
            va_list ap;
            va_start( ap, fmt );
            n = vsnprintf( log->ibuf, log->ibuf_size, fmt, ap );
            va_end( ap );

            if( n < 0 ) {
                break;
            }

            if( n < log->ibuf_size ) {
                if( !log->buf ) {
                    write( handle, log->ibuf, n );
                }
                else {
                    _log_cat_buf( log, log->ibuf, n );
                }

                break;
            }

            ptr = Realloc( log->ibuf, n + 1 );

            if( !ptr ) {
                break;
            }

            log->ibuf = ptr;
            log->ibuf_size = n + 1;
        }

        if( handle >= 0 && handle != fileno( stdout ) && handle != fileno( stderr ) ) {
            close( handle );
        }

        __unlock( log->lock );
    }
}

/*
 *  That's All, Folks!
 */
