/*
 *  Created on: 24 авг. 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "log.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

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

    if( buf_size ) {
        log->buf_size = buf_size < LOG_BUF_MIN_SIZE ? LOG_BUF_MIN_SIZE : buf_size;
        log->buf = Malloc( log->buf_size );

        if( !log->buf ) {
            Free( log );
            return NULL;
        }
    }

    log->format = Strdup( format ? format : LOG_DEFAULT_FORMAT );

    if( !log->format ) {
        Free( log->buf );
        Free( log );
        return NULL;
    }

    if( file ) {
        log->file = Strdup( file );

        if( !log->file ) {
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
    Free( log->buf );
    Free( log );
}

static void _log( LogInfo log, LOG_LEVEL level, const char *fmt, va_list ap )
{
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
