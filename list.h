/*
 * list.h, part of "klib" project.
 *
 *  Created on: 19 мая 2015 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef LIST_H_
#define LIST_H_

#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _LNode {
    void *data;
    struct _LNode *next;
    struct _LNode *prev;
} *LNode;

typedef void (*L_destructor)(void *data);
typedef void (*L_walk)(void *data);

typedef struct _List {
    LNode head;
    LNode tail;
    LNode cursor;
    L_destructor destructor;
    size_t size;
    int lock;
} *List;

List lcreate(L_destructor destructor);
void ldestroy(List list);
/*
 * lclear() remove all list elements
 */
void lclear(List list);
/*
 * ladd() add data to list TAIL
 */
void *ladd(List list, void *data);
/*
 * lpoke() add data to list HEAD
 */
void *lpoke(List list, void *data);
/*
 * ltail() peek data from list TAIL
 */
void *ltail(List list);
/*
 * lhead() peek data from list HEAD
 */
void *lhead(List list);
/*
 * lgettail() extract data from list TAIL
 * (data is not destroyed)
 */
void *lgettail(List list);
/*
 * lgethead() extract data from list HEAD
 * (data is not destroyed)
 */
void *lgethead(List list);
/*
 * ldeltail() delete data from list TAIL
 * and destroy it
 */
void ldeltail(List list);
/*
 * ldelhead() delete data from list HEAD
 * and destroy it
 */
void ldelhead(List list);

/*
 * lfirst() set internal cursor to first list
 * element and return it data
 */
void *lfirst(List list);
/*
 * lnext() move internal cursor to next list
 * element and return it data
 */
void *lnext(List list);
/*
 * lwalk() call function 'walker' for data of
 * all list elements
 */
void lwalk(List list, L_walk walker);

/*
 * Queue stuff:
 */
typedef List Queue;

#define qcreate(destructor) lcreate((destructor))
#define qdestroy(q)         ldestroy((q))
#define qclear(q)           lclear((q))
#define qwalk(q)            lwalk((q))
#define qput(q,data)        ladd( (q), (data) )
#define enqueue(q,data)     qput( (q), (data) )
#define qpoke(q,data)       lpoke( (q), (data) )
#define qpeek(q)            lhead((q))
#define dequeue(q)          qpeek((q))
#define qget(q)             lgethead((q))

/*
 * Stack stuff:
 */
typedef List Stack;

#define screate(destructor) lcreate((destructor))
#define sdestroy(stack)     ldestroy((stack))
#define sclear(stack)       lclear((stack))
#define swalk(stack)        lwalk((stack))
#define spush(stack,data)   ladd((stack),(data))
#define stop(stack)         ltail((stack))
#define spop(stack)         lgettail((stack))

#ifdef __cplusplus
}
#endif

#endif /* LIST_H_ */
