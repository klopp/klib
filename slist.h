/*
 * slist.h, part of "klib" project.
 *
 *  Created on: 21.05.2015, 04:55
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef SLIST_H_
#define SLIST_H_

#include "config.h"

#include "list.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *  List for strings
 */
typedef List    SList;

SList slcreate(void);
char *sladd(List list, const char *data);
char *slpoke(List list, const char *data);

#define sldestroy(slist)    ldestroy((slist))
#define slclear(slist)      lclear((slist))
#define slfirst(slist)      lfirst((slist))
#define slnext(slist)       lnext((slist))
#define slwalk(slist)       lwalk((slist))

#ifdef __cplusplus
}
#endif

#endif /* SLIST_H_ */
