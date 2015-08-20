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

typedef struct _Pair
{
    char * first;
    char * second;
} *Pair;

void delPair( void * ptr );

PList plcreate( void );
Pair pladd( List list, const char * first, const char * second );

#define sldestroy(slist)    ldestroy((slist))
#define slclear(slist)      lclear((slist))
#define slfirst(slist)      lfirst((slist))
#define slnext(slist)       lnext((slist))
#define slwalk(slist)       lwalk((slist))

#ifdef __cplusplus
}
#endif

#endif /* PLIST_H_ */
