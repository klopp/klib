/*
 *  Created on: 24 авг. 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "log.h"

LogInfo log_create(LOG_LEVEL level, const char *file, const char *format, size_t buf_size)
{
    LogInfo log = Calloc(sizeof(struct _LogInfo), 1);

    if (!log) {
        return NULL;
    }

    if (buf_size) {
        log->buf = Malloc(buf_size);

        if (!log->buf) {
            Free(log);
            return NULL;
        }

        log->buf_size = buf_size;
    }

    log->format = Strdup(format ? format : LOG_DEFAULT_FORMAT);

    if (!log->format) {
        Free(log->buf);
        Free(log);
        return NULL;
    }

    if (file) {
        log->file = Strdup(file);

        if (!log->file) {
            Free(log->format);
            Free(log->buf);
            Free(log);
            return NULL;
        }
    }

    log->level = level;
    return log;
}

void log_destroy(LogInfo log)
{
    Free(log->format);
    Free(log->file);
    Free(log->buf);
    Free(log);
}

/*
 *  That's All, Folks!
 */
