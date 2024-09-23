/*
    GLINK.H

    Generic Linkable Structures (GLINK) Version 0.17m

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __GLINK_H
#define __GLINK_H

    #include <stdlib.h>

    /* Returns the next element on the list. */
    #define NEXT(p) (void *)(((linkable_t *)(p))->next)

    /* Returns the previous element on the list. */
    #define PREV(p) (void *)(((linkable_t *)(p))->prev)

    /* Returns the top element on the list. */
    #define TOP(p) (void *)(((list_t *)(p))->top)

    /* Returns the bottom element on the list. */
    #define BOT(p) (void *)(((list_t *)(p))->bot)

    /* Returns the count of elements on the list. */
    #define COUNT(p) ((list_t *)(p))->count

    /* Calls __isolate, but performs a type cast. */
    #define isolate(x) __isolate ((linkable_t *)x)

    /* Basically returns TRUE if the list is empty. */
    #define list__empty(x) (((list_t *)(x))->top == NULL)

    /* Adds the given element to the list. */
    #define list__add(x, y) __add ((list_t *)(x), (linkable_t *)(y))

    /* Adds the given element to the top of the list. */
    #define list__addt(x, y) __addt ((list_t *)(x), (linkable_t *)(y))

    /* Removes the given element from the list. */
    #define list__rem(x, y) (void *)__rem ((list_t *)(x), (linkable_t *)(y))

    /* Walks the given list. */
    #define list__walk(x, y) __walk ((list_t *)(x), (walker_t)(&y))

    /* Extracts an element (extracts from the top). */
    #define list__extract(x) (void *)__rem ((list_t *)(x), ((list_t *)(x))->top)

    /* Pops an element (extracts from the bottom). */
    #define list__pop(x) (void *)__rem ((list_t *)(x), ((list_t *)(x))->bot)

    /* Searches for an element on the given list. */
    #define list__search(x, y, c) __search ((list_t *)(x), \
                                    (linkable_t *)(y), (comparator_t)(&c))

    /* Adds an element to the list, but in sorted order. */
    #define list__sorted_add(x, y, c) __sorted_add ((list_t *)(x), \
                                    (linkable_t *)(y), (comparator_t)(&c))

    /* This are the returns values of the walker. */
    #define STOP_WALKING        0x01
    #define CONTINUE_WALKING    0x00

    /* Generic linkable object form. */
    typedef struct linkable_s
    {
        struct  linkable_s *prev;
        struct  linkable_s *next;
    }
    linkable_t;

    /* Generic list form. */
    typedef struct list_s /* direct cast: linkable_t */
    {
        linkable_t  link;

        long        count;

        linkable_t  *top, *bot;
    }
    list_t;

    /* This is the walker type. */
    typedef int (*walker_t) (linkable_t *);

    /* The comparator type. */
    typedef int (*comparator_t) (linkable_t *, linkable_t *);

    /* Given two linkable things, it links them (duh!). */
    void __link (linkable_t *p, linkable_t *q);

    /* Unlinks the given linkable thing. */
    void __unlink (linkable_t *p);

    /* Walks the given linkable thing. */
    int __walkt (linkable_t *p, walker_t walker);

    /* Searches for the linkable thing, if found returns it. */
    linkable_t *__srch (linkable_t *p, linkable_t *x, comparator_t c);

    /* Links one linkable thing to a list (aka add to a list). */
    void __add (list_t *p, linkable_t *q);

    /* Links one linkable thing to the top of the list. */
    void __addt (list_t *p, linkable_t *q);

    /* Isolates an element (just sets NEXT and PREV to NULL). */
    linkable_t *__isolate (linkable_t *p);

    /* Unlinks one linkable thing off of a list (aka remove from a list). */
    linkable_t *__rem (list_t *p, linkable_t *q);

    /* Walks the given list. */
    int __walk (list_t *p, walker_t walker);

    /* Searches for the linkable thing, if found returns it. */
    linkable_t *__search (list_t *p, linkable_t *q, comparator_t c);

    /* Adds a linkable thing to a list, but where it SHOULD be. */
    void __sorted_add (list_t *l, linkable_t *x, comparator_t c);

    /* Nice macro to allocate clear blocks of memory. */
    #define new(x)      (x *)alloc (sizeof(x))

    /* And here's one to deallocate the block. */
    #define delete(x)   free (x)

    /* Allocates an amount of bytes. */
    #define alloc(x)    __alloc_n (x)

    /* The memory allocator without debugging caps. */
    void *__alloc_n (unsigned);

    /* Simply duplicates the given string. */
    char *dups (char *s);

#endif
