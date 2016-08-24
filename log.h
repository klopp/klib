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

typedef enum _LOG_LEVEL
{
    LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL, LOG_LVL_MAX
} LOG_LEVEL;

typedef struct _LogInfo
{
    char *buf;
    char *file;
    char *format;
    size_t buf_size;
    size_t in_buf;
    LOG_LEVEL level;
}*LogInfo;

#define LOG_BUF_MIN_SIZE        (1024 * 4)
#define LOG_DEFAULT_FORMAT      "[%L] %d.%m.%y %H:%M:%S "

/*
 * 'file': NULL, "-" -> stdout, "=" > stderr
 * 'buf_len': 0 to no bufferization
 * 'format':
 *      %l - verbose log level ("debug", "info", "warn", "error", "fatal")
 *      %L - short log level ("#", "i", "?", "!", "*")
 *      %* - message
 *      %p - PID
 *      %d, %m, %y, %H, %M, %S - day, month, year, hour, min, sec
 */
LogInfo log_create(LOG_LEVEL level, const char *file, const char *format, size_t buf_size);
void log_destroy(LogInfo log);

#if defined(__cplusplus)
}; /* extern "C" */
#endif

#endif /* LOG_H_ */

/*
 *  That's All, Folks!
 */
