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

/*
 * Internal data structure, represent log levels abreviations:
 */
typedef struct _log_titles {
    LOG_FLAGS level;
    const char *title;
} log_titles;

/*
 * Internal, get log levels abreviations:
 */
static const char *_log_title( LOG_FLAGS level, log_titles *titles,
                               const char *dflt )
{
    while( titles->title ) {
        if( level == titles->level ) {
            return titles->title;
        }

        titles++;
    }

    return dflt;
}

/*
 * Long log levels abreviations:
 */
static const char *_log_long_title( LOG_FLAGS level )
{
    static log_titles titles[] = {
        { LOG_LEVEL_DEBUG, "debug" }, { LOG_LEVEL_INFO, "info" }, { LOG_LEVEL_WARN, "warn" }, {
            LOG_LEVEL_ERROR, "error"
        }, { LOG_LEVEL_FATAL, "fatal" }, { 0, NULL }
    };
    return _log_title( level, titles, "log" );
}

/*
 * Short log levels abreviations:
 */
static const char *_log_short_title( LOG_FLAGS level )
{
    static log_titles titles[] = {
        { LOG_LEVEL_DEBUG, "dbg" }, { LOG_LEVEL_INFO, "inf" }, { LOG_LEVEL_WARN, "wrn" }, {
            LOG_LEVEL_ERROR, "err"
        }, { LOG_LEVEL_FATAL, "fat" }, {0, NULL }
    };
    return _log_title( level, titles, "@" );
}

/*
 * Symbolic log levels abreviations:
 */
static const char *_log_sym_title( LOG_FLAGS level )
{
    static log_titles titles[] = {
        { LOG_LEVEL_DEBUG, "#" }, { LOG_LEVEL_INFO, "i" }, { LOG_LEVEL_WARN, "?" }, {
            LOG_LEVEL_ERROR, "!"
        }, { LOG_LEVEL_FATAL, "*" }, {0, NULL }
    };
    return _log_title( level, titles, "@" );
}

/*
 * Get log file handle. Open file or return stderr/stdout:
 */
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

void log_flush( LogInfo log )
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

/*
 * Create log info structure:
 */
LogInfo log_create( LOG_FLAGS flags, const char *file, const char *prefix,
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

    log->prefix = ( prefix ? ( *prefix ? Strdup( prefix ) : NULL ) : Strdup(
                            LOG_DEFAULT_PREFIX ) );

    if( ( ( prefix && *prefix ) || !prefix ) && !log->prefix ) {
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

    log->flags = flags;
    log->timefunc = ( flags & LOG_USE_GMTIME ) ? gmtime : localtime;
    __initlock( log->lock );
    return log;
}

/*
 * Destroy log info structure. Unsaved buffer will be saved.
 */
void log_destroy( LogInfo log )
{
    log_flush( log );
    Free( log->prefix );
    Free( log->file );
    Free( log->ibuf );
    Free( log->buf );
    Free( log );
}

/*
 * Lazy time initialization:
 */
static struct tm *_log_init_time( LogInfo log )
{
    if( !log->tnow ) {
        time_t tt = time( NULL );
        log->tnow = log->timefunc( &tt );
    }

    return log->tnow;
}

/*
 * Expand internal buffer if needed:
 */
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

/*
 * Move internal buffer content to common buffer:
 */
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
            log_flush( log );
        }
        else {
            memcpy( log->buf + log->in_buf, buf, blen );
            log->in_buf += to_copy;
        }

        buf += to_copy;
        blen -= to_copy;
    }
}

/*
 * Make log prefix and move it to common buffer:
 */
static size_t _log_make_prefix( LogInfo log, LOG_FLAGS level )
{
    const char *fmt = log->prefix;
    size_t size = 0;
    char buf[0x40];
    log->tnow = NULL;

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
                    if( !( size = _log_cat_ibuf( log, _log_long_title( level ), size ) ) ) {
                        return 0;
                    }

                    break;

                case 's':
                    if( !( size = _log_cat_ibuf( log, _log_short_title( level ), size ) ) ) {
                        return 0;
                    }

                    break;

                case '~':
                    if( !( size = _log_cat_ibuf( log, _log_sym_title( level ), size ) ) ) {
                        return 0;
                    }

                    break;

                case 'd':
                    _log_init_time( log );
                    sprintf( buf, "%02u", log->tnow->tm_mday );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'm':
                    _log_init_time( log );
                    sprintf( buf, "%02u", log->tnow->tm_mon + 1 );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'y':
                    _log_init_time( log );
                    sprintf( buf, "%04u", log->tnow->tm_year + 1900 );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'H':
                    _log_init_time( log );
                    sprintf( buf, "%02u", log->tnow->tm_hour );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'M':
                    _log_init_time( log );
                    sprintf( buf, "%02u", log->tnow->tm_min );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'S':
                    _log_init_time( log );
                    sprintf( buf, "%02u", log->tnow->tm_sec );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'X':
                    _log_init_time( log );
                    sprintf( buf, "%02u:%02u:%02u", log->tnow->tm_hour, log->tnow->tm_min,
                             log->tnow->tm_sec );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'Y':
                    _log_init_time( log );
                    sprintf( buf, "%02u.%02u.%04u", log->tnow->tm_mday, log->tnow->tm_mon + 1,
                             log->tnow->tm_year + 1900 );

                    if( !( size = _log_cat_ibuf( log, buf, size ) ) ) {
                        return 0;
                    }

                    break;

                case 'Z':
                    _log_init_time( log );
                    sprintf( buf, "%02u.%02u.%04u %02u:%02u:%02u", log->tnow->tm_mday,
                             log->tnow->tm_mon + 1,
                             log->tnow->tm_year + 1900, log->tnow->tm_hour, log->tnow->tm_min,
                             log->tnow->tm_sec );

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

    if( size && log->ibuf[size - 1] != ' ' ) {
        if( !_log_check_ibuf( log, size + 1 ) ) {
            return 0;
        }

        log->ibuf[size] = ' ';
        size++;
    }

    log->ibuf[size] = 0;
    return size;
}

/*
 * Main log function:
 */
void plog( LogInfo log, LOG_FLAGS level, const char *fmt, ... )
{
    if( log->flags & level ) {
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
            int cr = ( log->flags & LOG_APPEND_CR ) != 0;
            va_start( ap, fmt );
            n = vsnprintf( log->ibuf, log->ibuf_size, fmt, ap );
            va_end( ap );

            if( n < 0 ) {
                break;
            }

            if( n + cr < log->ibuf_size ) {
                if( cr ) {
                    log->ibuf[n] = '\n';
                    n++;
                }

                if( !log->buf ) {
                    write( handle, log->ibuf, n );
                }
                else {
                    _log_cat_buf( log, log->ibuf, n );
                }

                break;
            }

            ptr = Realloc( log->ibuf, n + 1 + cr );

            if( !ptr ) {
                break;
            }

            log->ibuf = ptr;
            log->ibuf_size = n + 1 + cr;
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
