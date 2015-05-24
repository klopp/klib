/*
 * log.h, part of "klib" project.
 *
 *  Created on: 24.05.2015, 06:06
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef LOG_H_
#define LOG_H_

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

typedef enum _LogFlags
{
    LOG_FATAL = 1,
    LOG_ERROR = LOG_FATAL << 1,
    LOG_WARN = LOG_ERROR << 1,
    LOG_INFO = LOG_WARN << 1,
    LOG_DBG = LOG_INFO << 1,
    LOG_ALL_LVL = (LOG_FATAL | LOG_ERROR | LOG_WARN |LOG_INFO|LOG_DBG),

    LOG_STDERR = LOG_DBG << 1,
    LOG_STDOUT = LOG_STDERR << 1,
    LOG_LOGLEVEL = LOG_STDOUT << 1,

    LOG_LOGPID = LOG_LOGLEVEL <<1,
    LOG_DATE = LOG_LOGPID << 1,
    LOG_TIME = LOG_DATE << 1,
    LOG_DATETIME = (LOG_TIME | LOG_DATE),

    LOG_DEFAULTS = (LOG_FATAL | LOG_ERROR | LOG_WARN | LOG_DATETIME
            | LOG_LOGLEVEL)
} LogFlags;

typedef struct _Log
{
    LogFlags flags;
    char filename[PATH_MAX + 1];
    char format_fatal;
    char format_err;
    char format_warn;
    char format_info;
    char format_dbg;
}*Log;

Log log_create( const char * filename, LogFlags flags );
int plog( Log log, LogFlags level, const char * fmt, ... );
void log_destroy( Log log );

#endif /* LOG_H_ */
