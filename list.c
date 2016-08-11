/*
 * list.c, part of "lists" project.
 *
 *  Created on: 19 мая 2015 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "list.h"
#include "_lock.h"

void list_Free( void *data )
{
    Free( data );
}

List lcreate( L_destructor destructor )
{
    List list = Calloc( sizeof( struct _List ), 1 );

    if( !list ) {
        return NULL;
    }

    list->destructor = destructor;
    __initlock( list->lock );
    return list;
}

void lclear( List list )
{
    if( list ) {
        LNode node;
        __lock( list->lock );
        node = list->head;

        while( node ) {
            LNode lcurrent = node;
            node = node->next;

            if( list->destructor && lcurrent->data ) {
                list->destructor( lcurrent->data );
            }

            Free( lcurrent );
        }

        list->head = list->tail = list->cursor = NULL;
        list->size = 0;
        __unlock( list->lock );
    }
}

void ldestroy( List list )
{
    lclear( list );
    Free( list );
}

void *ladd( List list, void *data )
{
    if( list && data ) {
        LNode  node;
        __lock( list->lock );
        node = Calloc( sizeof( struct _LNode ), 1 );

        if( !node ) {
            __unlock( list->lock );
            return NULL;
        }

        node->data = data;

        if( !list->head ) {
            list->head = list->tail = node;
        }
        else {
            node->prev = list->tail;
            list->tail->next = node;
            list->tail = list->tail->next;
        }

        list->size++;
        __unlock( list->lock );
        return node->data;
    }

    return NULL;
}

void *lpoke( List list, void *data )
{
    if( list && data ) {
        LNode  node;
        __lock( list->lock );
        node = Calloc( sizeof( struct _LNode ), 1 );

        if( !node ) {
            __unlock( list->lock );
            return NULL;
        }

        node->data = data;

        if( !list->head ) {
            list->head = list->tail = node;
        }
        else {
            node->next = list->head;
            list->head->prev = node;
            list->head = node;
        }

        list->size++;
        __unlock( list->lock );
        return node->data;
    }

    return NULL;
}

void *lfirst( List list )
{
    if( !list ) {
        return NULL;
    }

    list->cursor = list->head;
    return list->cursor ? list->cursor->data : NULL;
}

void *lnext( List list )
{
    if( !list || !list->cursor ) {
        return NULL;
    }

    list->cursor = list->cursor->next;
    return list->cursor ? list->cursor->data : NULL;
}

void lwalk( List list, L_walk walker )
{
    if( list && walker ) {
        LNode  node;
        __lock( list->lock );
        node = list->head;

        while( node ) {
            walker( node->data );
            node = node->next;
        }

        __unlock( list->lock );
    }
}

void *lgethead( List list )
{
    if( list && list->head ) {
        void *data;
        LNode node;
        __lock( list->lock );
        data = list->head->data;
        node = list->head;
        list->head = list->head->next;
        Free( node );

        if( list->head ) {
            list->head->prev = NULL;
        }

        list->size--;

        if( !list->size ) {
            list->tail = NULL;
        }

        __unlock( list->lock );
        return data;
    }

    return NULL;
}

void *lgettail( List list )
{
    if( list && list->tail ) {
        void *data;
        LNode  node;
        __lock( list->lock );
        data = list->tail->data;
        node = list->tail;
        list->tail = list->tail->prev;
        Free( node );

        if( list->tail ) {
            list->tail->next = NULL;
        }

        list->size--;

        if( !list->size ) {
            list->head = NULL;
        }

        __unlock( list->lock );
        return data;
    }

    return NULL;
}

void ldeltail( List list )
{
    void *data = lgettail( list );

    if( data && list->destructor ) {
        list->destructor( data );
    }
}

void ldelhead( List list )
{
    void *data = lgethead( list );

    if( data && list->destructor ) {
        list->destructor( data );
    }
}

void *ltail( List list )
{
    return ( list && list->tail ) ? list->tail->data : NULL;
}
void *lhead( List list )
{
    return ( list && list->head ) ? list->head->data : NULL;
}

