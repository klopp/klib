/*
 *  Created on: 22 авг. 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef LOG_H_
#define LOG_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "config.h"
#include "_lock.h"
#include <stdio.h>

typedef enum _LOG_FLAGS
{
    LOG_LEVEL_DEBUG = 0x01,
    LOG_LEVEL_INFO = 0x02,
    LOG_LEVEL_WARN = 0x04,
    LOG_LEVEL_ERROR = 0x08,
    LOG_LEVEL_FATAL = 0x10,
    LOG_APPEND_CR = 0x400,
    LOG_LEVEL_DEFAULT = LOG_LEVEL_INFO | LOG_LEVEL_WARN | LOG_LEVEL_ERROR |
                        LOG_LEVEL_FATAL
}
LOG_FLAGS;

typedef struct _LogInfo {
    char *buf;
    char *ibuf;
    char *file;
    char *prefix;
    size_t buf_size;
    size_t ibuf_size;
    size_t in_buf;
    LOG_FLAGS flags;
    __lock_t( lock );
} *LogInfo;

#define LOG_BUF_MIN_SIZE            (1024 * 4)
#define LOG_IBUF_MIN_SIZE           64
#define LOG_DEFAULT_PREFIX          "[%s] %Z"

/*
 * 'file'    : NULL, "-" -> stdout, "=" -> stderr
 * 'buf_len' : 0 to no bufferization
 * 'prefix'  :
 *      NULL - LOG_DEFAULT_PREFIX
 *      "" - no prefix
 *      %l - verbose log level ("debug", "info", "warn", "error", "fatal")
 *      %s - short log level ("dbg", "inf", "wrn", "err", "fat")
 *      %~ - symbol log level ("#", "i", "?", "!", "*")
 *      %p - PID
 *      %d, %m, %y, %H, %M, %S - day, month, year, hour, min, sec
 *      %X - hour:min:sec
 *      %Y - day.month.year
 *      %Z - day.month.year hour:min:sec
 */
LogInfo log_create( LOG_FLAGS level, const char *file, const char *prefix,
                    size_t buf_size );
void log_destroy( LogInfo log );
void plog( LogInfo log, LOG_FLAGS level, const char *fmt, ... );

#define dlog( log, fmt, ... )   plog( (log), LOG_LEVEL_DEBUG, (fmt), __VA_ARGS__ )
#define ilog( log, fmt, ... )   plog( (log), LOG_LEVEL_INFO,  (fmt), __VA_ARGS__ )
#define wlog( log, fmt, ... )   plog( (log), LOG_LEVEL_WARN,  (fmt), __VA_ARGS__ )
#define elog( log, fmt, ... )   plog( (log), LOG_LEVEL_ERROR, (fmt), __VA_ARGS__ )
#define flog( log, fmt, ... )   plog( (log), LOG_LEVEL_FATAL, (fmt), __VA_ARGS__ )

#if defined(__cplusplus)
}; /* extern "C" */
#endif

#endif /* LOG_H_ */

/*
 *  That's All, Folks!
 */
