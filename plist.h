/*
 * plist.h, part of "klib" project.
 *
 *  Created on: 20.08.2015, 16:03
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef PLIST_H_
# define PLIST_H_

#include "list.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *  List for strings pairs
 */
typedef List    PList;

typedef struct _Pair {
    char *first;
    char *second;
} *Pair;

#define PAIR_KEY(pair)      (pair)->first
#define PAIR_VAL(pair)      (pair)->second

void pair_Delete(void *ptr);
Pair pair_Create(const char *first, const char *second);

PList plcreate(void);
Pair pladd(PList list, const char *first, const char *second);
const char *plget(PList list, const char *key);

#define pldestroy(plist)    ldestroy((plist))
#define plclear(plist)      lclear((plist))
#define plfirst(plist)      lfirst((plist))
#define plnext(plist)       lnext((plist))
#define plwalk(plist)       lwalk((plist))
#define plhead(plist)       lhead((plist))

#ifdef __cplusplus
}
#endif

#endif /* PLIST_H_ */
