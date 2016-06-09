/*
 * st_trace.c, part of "klib" project.
 *
 *  Created on: 30.05.2016, 16:54
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "st_trace.h"
#include "list.h"
#include "_lock.h"
#include "sig.h"

#ifndef NDEBUG

/* ---------------------------------------------------------------------------*/
typedef struct {
    char name[_ST_NAME_MAX + 1];
    char file[_ST_FILE_MAX + 1];
    char arg[_ST_ARG_MAX + 1];
    char ret[_ST_RET_MAX + 1];
    unsigned line;

} _st_trace;

static Stack _st_stack = NULL;
static size_t _st_over = 0;
int _st_lock = 0;

/* ---------------------------------------------------------------------------*/
static void _st_signal(int signo) {
    switch(signo) {
        case SIGSEGV:
        case SIGILL:
        case SIGABRT:
        case SIGBUS:
        case SIGFPE:
        case SIGSYS:
            fprintf(stderr, "\nGot signal %s, stack trace:\n\n",
                    signal_name(signo));
            st_dump(NULL, NULL, " ", 2);
            break;
        default:
            break;
    }
    signal(signo, SIG_DFL);
}

/* ---------------------------------------------------------------------------*/
static void _st_down(void) {
    sdestroy(_st_stack);
}

/* ---------------------------------------------------------------------------*/
void _st_init(void) {
    if(!_st_stack) {
        _st_stack = screate(lambda(void, (void * data) {
            Free(data);
        }));
        atexit(_st_down);
        signal(SIGSEGV, _st_signal);
        signal(SIGABRT, _st_signal);
        signal(SIGILL, _st_signal);
        signal(SIGBUS, _st_signal);
        signal(SIGFPE, _st_signal);
        signal(SIGSYS, _st_signal);
    }
}

/* ---------------------------------------------------------------------------*/
size_t _st_push(const char *ret, const char *name, const char *arg,
                const char *file, unsigned line) {
    _lock(&_st_lock);
    _st_trace *data = Calloc(sizeof(_st_trace), 1);
    strncpy(data->ret, ret, _ST_RET_MAX);
    strncpy(data->name, name, _ST_NAME_MAX);
    strncpy(data->file, file, _ST_FILE_MAX);
    strncpy(data->arg, arg, _ST_ARG_MAX);
    data->line = line;
    size_t i = strlen(data->arg);
    if(*arg == '(') {
        memmove(data->arg, data->arg + 1, i);
        i--;
    }
    if(i && *(data->arg + i - 1) == ')') {
        *(data->arg + i - 1) = 0;
    }
    if(!spush(_st_stack, data)) {
        _st_over++;
    }
    _unlock(_st_lock);
    return _st_stack->size;
}

/* ---------------------------------------------------------------------------*/
void _st_pop(void) {
    _lock(&_st_lock);
    if(_st_over) {
        _st_over--;
    } else {
        Free(spop(_st_stack));
    }
    _unlock(_st_lock);
}

/* ---------------------------------------------------------------------------*/
static void _default_dumper(const char *ret, const char *name,
                            const char *arg, const char *file, unsigned line, FILE *handle) {
    fprintf(handle, "%s %s(%s) at %s, line %u\n", ret, name, arg, file, line);
}

/* ---------------------------------------------------------------------------*/
void st_dump(const st_dumper dumper, FILE *file, const char *tab, int numbers) {
    _lock(&_st_lock);
    size_t n = 1;
    size_t i = _st_stack->size;
    st_dumper _dumper = dumper ? dumper : _default_dumper;
    const char *_tab = tab ? tab : "";
    FILE *_file = file ? file : stderr;
    LNode node = _st_stack->tail;
    while(node) {
        _st_trace *data = node->data;
        node = node->prev;
        size_t j = 0;
        if(numbers) {
            fprintf(_file, numbers == 2 ? "%zu. " : "%zu ", n);
            n++;
        }
        while(tab && *_tab && j < _st_stack->size - i) {
            fprintf(_file, "%s", _tab);
            j++;
        }
        i--;
        _dumper(data->ret, data->name, data->arg, data->file, data->line,
                _file);
    }
    _unlock(_st_lock);
}

#endif
