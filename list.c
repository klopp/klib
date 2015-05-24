/*
 * list.c, part of "lists" project.
 *
 *  Created on: 19 мая 2015 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "list.h"

List lcreate( L_destructor destructor )
{
    List list = calloc( sizeof(struct _List), 1 );
    if( !list ) return NULL;
    list->destructor = destructor;
    return list;
}
void lclear( List list )
{
    if( list )
    {
        LNode node = list->head;
        while( node )
        {
            LNode current = node;
            node = node->next;
            if( list->destructor && current->data ) list->destructor(
                    current->data );
            free( current );
        }
        list->head = list->tail = list->cursor = NULL;
        list->size = 0;
    }
}

void ldestroy( List list )
{
    if( list )
    {
        lclear( list );
        free( list );
    }
}

void * ladd( List list, void * data )
{
    if( list && data )
    {
        LNode node = calloc( sizeof(struct _LNode), 1 );
        if( !node ) return NULL;
        node->data = data;
        if( !list->head )
        {
            list->head = list->tail = node;
        }
        else
        {
            node->prev = list->tail;
            list->tail->next = node;
            list->tail = list->tail->next;
        }
        list->size++;
        return node->data;
    }
    return NULL;
}

void * lpoke( List list, void * data )
{
    if( list && data )
    {
        LNode node = calloc( sizeof(struct _LNode), 1 );
        if( !node ) return NULL;
        node->data = data;
        if( !list->head )
        {
            list->head = list->tail = node;
        }
        else
        {
            node->next = list->head;
            list->head->prev = node;
            list->head = node;
        }
        list->size++;
        return node->data;
    }
    return NULL;
}

void * lfirst( List list )
{
    if( !list ) return NULL;
    list->cursor = list->head;
    return list->cursor ? list->cursor->data : NULL;
}

void * lnext( List list )
{
    if( !list || !list->cursor ) return NULL;
    list->cursor = list->cursor->next;
    return list->cursor ? list->cursor->data : NULL;
}

void lwalk( List list, L_walk walker )
{
    if( list && walker )
    {
        LNode node = list->head;
        while( node )
        {
            walker( node->data );
            node = node->next;
        }
    }
}

void * lgethead( List list )
{
    if( list && list->head )
    {
        void * data = list->head->data;
        LNode node = list->head;
        list->head = list->head->next;
        free( node );
        if( list->head ) list->head->prev = NULL;
        list->size--;
        if( !list->size ) list->tail = NULL;
        return data;
    }
    return NULL;
}

void * lgettail( List list )
{
    if( list && list->tail )
    {
        void * data = list->tail->data;
        LNode node = list->tail;
        list->tail = list->tail->prev;
        free( node );
        if( list->tail ) list->tail->next = NULL;
        list->size--;
        if( !list->size ) list->head = NULL;
        return data;
    }
    return NULL;
}

void ldeltail( List list )
{
    void * data = lgettail( list );
    if( data && list->destructor ) list->destructor( data );
}

void ldelhead( List list )
{
    void * data = lgethead( list );
    if( data && list->destructor ) list->destructor( data );
}

void * ltail( List list )
{
    return (list && list->tail) ? list->tail->data : NULL;
}
void * lhead( List list )
{
    return (list && list->head) ? list->head->data : NULL;
}

/*
void ldelnode( List list, LNode node )
{
    if( node->next )
    {
        node->next->prev = node->prev;
    }
    if( node->prev )
    {
        node->prev->next = node->next;
    }
    if( node->data && list->destructor) list->destructor( node->data );
    free( node);
}
*/
