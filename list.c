/*
 * list.c, part of "lists" project.
 *
 *  Created on: 19 мая 2015 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "list.h"

#include "_lock.h"

List lcreate(L_destructor destructor) {
    List list = Calloc(sizeof(struct _List), 1);
    if(!list) {
        return NULL;
    }
    list->destructor = destructor;
    return list;
}

void lclear(List list) {
    if(list) {
        _lock(&list->lock);
        LNode node = list->head;
        while(node) {
            LNode current = node;
            node = node->next;
            if(list->destructor && current->data) {
                list->destructor(current->data);
            }
            Free(current);
        }
        list->head = list->tail = list->cursor = NULL;
        list->size = 0;
        _unlock(list->lock);
    }
}

void ldestroy(List list) {
    lclear(list);
    Free(list);
}

void *ladd(List list, void *data) {
    if(list && data) {
        _lock(&list->lock);
        LNode node = Calloc(sizeof(struct _LNode), 1);
        if(!node) {
            _unlock(list->lock);
            return NULL;
        }
        node->data = data;
        if(!list->head) {
            list->head = list->tail = node;
        } else {
            node->prev = list->tail;
            list->tail->next = node;
            list->tail = list->tail->next;
        }
        list->size++;
        _unlock(list->lock);
        return node->data;
    }
    return NULL;
}

void *lpoke(List list, void *data) {
    if(list && data) {
        _lock(&list->lock);
        LNode node = Calloc(sizeof(struct _LNode), 1);
        if(!node) {
            _unlock(list->lock);
            return NULL;
        }
        node->data = data;
        if(!list->head) {
            list->head = list->tail = node;
        } else {
            node->next = list->head;
            list->head->prev = node;
            list->head = node;
        }
        list->size++;
        _unlock(list->lock);
        return node->data;
    }
    return NULL;
}

void *lfirst(List list) {
    if(!list) {
        return NULL;
    }
    list->cursor = list->head;
    return list->cursor ? list->cursor->data : NULL;
}

void *lnext(List list) {
    if(!list || !list->cursor) {
        return NULL;
    }
    list->cursor = list->cursor->next;
    return list->cursor ? list->cursor->data : NULL;
}

void lwalk(List list, L_walk walker) {
    if(list && walker) {
        _lock(&list->lock);
        LNode node = list->head;
        while(node) {
            walker(node->data);
            node = node->next;
        }
        _unlock(list->lock);
    }
}

void *lgethead(List list) {
    if(list && list->head) {
        _lock(&list->lock);
        void *data = list->head->data;
        LNode node = list->head;
        list->head = list->head->next;
        Free(node);
        if(list->head) {
            list->head->prev = NULL;
        }
        list->size--;
        if(!list->size) {
            list->tail = NULL;
        }
        _unlock(list->lock);
        return data;
    }
    return NULL;
}

void *lgettail(List list) {
    if(list && list->tail) {
        _lock(&list->lock);
        void *data = list->tail->data;
        LNode node = list->tail;
        list->tail = list->tail->prev;
        Free(node);
        if(list->tail) {
            list->tail->next = NULL;
        }
        list->size--;
        if(!list->size) {
            list->head = NULL;
        }
        _unlock(list->lock);
        return data;
    }
    return NULL;
}

void ldeltail(List list) {
    void *data = lgettail(list);
    if(data && list->destructor) {
        list->destructor(data);
    }
}

void ldelhead(List list) {
    void *data = lgethead(list);
    if(data && list->destructor) {
        list->destructor(data);
    }
}

void *ltail(List list) {
    return (list && list->tail) ? list->tail->data : NULL;
}
void *lhead(List list) {
    return (list && list->head) ? list->head->data : NULL;
}

