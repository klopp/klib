/*
 * log.h, part of "klib" project.
 *
 *  Created on: 24.05.2015, 06:06
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef LOG_H_
#define LOG_H_

#include "config.h"

typedef enum _LogFlags
{
    LOG_FATAL = 1,
    LOG_ERROR = LOG_FATAL << 1,
    LOG_WARN = LOG_ERROR << 1,
    LOG_INFO = LOG_WARN << 1,
    LOG_DBG = LOG_INFO << 1,
    LOG_ALL_LVL = (LOG_FATAL | LOG_ERROR | LOG_WARN | LOG_INFO | LOG_DBG),

    LOG_STDERR = LOG_DBG << 1,
    LOG_STDOUT = LOG_STDERR << 1,
    LOG_LOGLEVEL = LOG_STDOUT << 1,

    LOG_LOGPID = LOG_LOGLEVEL << 1,
    LOG_DATE = LOG_LOGPID << 1,
    LOG_TIME = LOG_DATE << 1,
    LOG_DATETIME = (LOG_TIME | LOG_DATE),

    LOG_DEFAULTS = (LOG_ALL_LVL | LOG_DATETIME | LOG_LOGLEVEL)
} LogFlags;

typedef struct _Log
{
    LogFlags flags;
    char format_fatal;
    char format_err;
    char format_warn;
    char format_info;
    char format_dbg;
    char format_datetime;
    char * file;
}*Log;

/*
 * filename may be format string:
 *
 * %S - any C-style string
 * %U - unsigned int (%NU - use N zero for padding)
 * %P - process pid (%NP - use N zero for padding)
 *
 * %Y - current year (4-digit)
 * %M - current month (2-digit)
 * %D - current day (2-digit)
 *
 * %h - current hour (2-digit)
 * %m - current minute (2-digit)
 * %s - current second (2-digit)
 *
 * %a - alias for %Y.%M.%D
 * %A - alias for %Y-%M-%D
 * %b - alias for %h.%m.%s
 * %B - alias for %h-%m-%s
 *
 * %z - alias for %Y.%M.%D-%h.%m.%s
 * %Z - alias for %Y-%M-%D-%h-%m-%s
 */
Log log_create( LogFlags flags, const char * filename, ... );
void log_destroy( Log log );

int plog( Log, LogFlags level, const char * fmt, ... );
int flog( Log, const char * fmt, ... );
int elog( Log, const char * fmt, ... );
int wlog( Log, const char * fmt, ... );
int ilog( Log, const char * fmt, ... );
int dlog( Log, const char * fmt, ... );

const char * log_datetime( char separator );
const char * log_date( void );
const char * log_time( void );

#endif /* LOG_H_ */
