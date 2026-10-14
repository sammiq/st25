/* seg011: doubly linked lists */

#include "compat.h"
#include "list.h"

void ListInit(List *l)
{
    l->head.next = &l->tail;
    l->head.prev = &l->tail;
    l->tail.next = &l->head;
    l->tail.prev = &l->head;
}

/* Link n in next to the tail sentinel. */
void ListAdd(List *l, Node *n)
{
    Node *last;
    Node *tail;

    last = l->tail.next;
    tail = &l->tail;
    n->prev = tail;
    n->next = last;
    last->prev = n;
    tail->next = n;
}

void ListRemove(List *l, Node *n)
{
    Node *a;
    Node *b;

    a = n->next;
    b = n->prev;
    a->prev = b;
    b->next = a;
    n->next = 0;
    n->prev = 0;
}
