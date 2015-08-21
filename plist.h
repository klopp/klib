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

void * pair_Delete( void * ptr );
Pair pair_Create( const char * first, const char * second );

PList plcreate( void );
Pair pladd( List list, const char * first, const char * second );

#define pldestroy(slist)    ldestroy((slist))
#define plclear(slist)      lclear((slist))
#define plfirst(slist)      lfirst((slist))
#define plnext(slist)       lnext((slist))
#define plwalk(slist)       lwalk((slist))

#ifdef __cplusplus
}
#endif

#endif /* PLIST_H_ */
