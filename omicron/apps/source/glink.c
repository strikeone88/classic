/*
    GLINK.C

    Generic Linkable Structures (GLINK) Version 0.17m

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <glink.h>

    /* This is the memory allocator. */
    void *__alloc_n (unsigned x)
    {
        void *p;

            if (!x) return NULL;

            if ((p = calloc (x)) == NULL)
            {
                printf ("*(glink) -- not enough memory, exiting.\n\n");
                exit (9);
            }

            return p;
    }

    /* Links q to the right of p. */
    void __link (linkable_t *p, linkable_t *q)
    {
            if (p != NULL) p->next = q;
            if (q != NULL) q->prev = p;
    }

    /* Unlinks the given linkable thing. */
    void __unlink (linkable_t *p)
    {
            if (p == NULL) return;

            if (p->prev != NULL) p->prev->next = p->next;
            if (p->next != NULL) p->next->prev = p->prev;
    }

    /* Walks the given linkable thing. */
    int __walkt (linkable_t *p, walker_t walker)
    {
        linkable_t *q;
        int i;

            while (p != NULL)
            {
                q = p->next;

                if ((i = walker (p)) != CONTINUE_WALKING)
                    return i;

                p = q;
            }

            return 0;
    }

    /* Searches for the linkable thing, if found returns it. */
    linkable_t *__srch (linkable_t *p, linkable_t *x, comparator_t c)
    {
            while (p != NULL)
            {
                if (!c (x, p)) return p;

                p = p->next;
            }

            return NULL;
    }

    /* Links one linkable thing to a list (aka add to a list). */
    void __add (list_t *p, linkable_t *q)
    {
            if (p == NULL) return;

            __link (p->bot, q);

            if (p->top == NULL) p->top = q;

            p->bot = q;

            p->count++;
    }

    /* Links one linkable thing to the top of the list. */
    void __addt (list_t *p, linkable_t *q)
    {
            if (p == NULL) return;

            __link (q, p->top);

            if (p->top == NULL) p->bot = q;

            p->top = q;

            p->count++;
    }

    /* Isolates an element (just sets NEXT and PREV to NULL). */
    linkable_t *__isolate (linkable_t *p)
    {
            p->next = p->prev = NULL;
            return p;
    }

    /* Unlinks one linkable thing off of a list (aka remove from a list). */
    linkable_t *__rem (list_t *p, linkable_t *q)
    {
            if (p == NULL || q == NULL) return q;

            __unlink (q);

            if (q->prev == NULL) p->top = q->next;
            if (q->next == NULL) p->bot = q->prev;

            __isolate (q);

            p->count--;

            return q;
    }

    /* Walks the given list. */
    int __walk (list_t *p, walker_t walker)
    {
            if (p == NULL) return 0;

            return __walkt (p->top, walker);
    }

    /* Searches for the linkable thing, if found returns it. */
    linkable_t *__search (list_t *p, linkable_t *q, comparator_t c)
    {
            if (p == NULL) return NULL;

            return __srch (p->top, q, c);
    }

    /* Adds a linkable thing to a list, but where it SHOULD be. */
    void __sorted_add (list_t *l, linkable_t *x, comparator_t c)
    {
        linkable_t *p;

            for (p = l->top; p != NULL && c (x, p) > 0; p = p->next);

            if (p != NULL)
            {
                if (p->prev != NULL)
                    __link (p->prev, x);
                else
                    l->top = x;

                __link (x, p);

                l->count++;
            }
            else
                __add (l, x);
    }

    /* Simply duplicates the given string. */
    char *dups (char *s)
    {
            if (s == NULL) return NULL;

            return strcpy (alloc (strlen (s) + 1), s);
    }
