/*
    TR.H

    Trillian Virtual Address Space Engine Version 0.01 (Interface)

    Copyright (C) 2007 RedStar Technologies
    Written by J. Palencia (zipox@ureach.com)
*/

#ifndef __TR_H
#define __TR_H

    /* Some definitions to make the interface easier. */
    #define vnew(type)      (type *)tr__alloc (sizeof(type))
    #define vdeleteb(p)     tr__freeb ((void *)p)
    #define vdelete(p)      tr__free ((void *)p)
    #define valloc(len)     tr__alloc (len)
    #define vclose(p)       tr__close ((void **)&p, 1)
    #define vdiscard(p)     tr__close ((void **)&p, 0)
    #define vopen(p)        tr__open ((void **)&p)

    /* Context of an space. */
    typedef struct
    {
        /* The storage file for the data. */
        void *file;

        /* Offset to the bottom and dead-list. */
        unsigned long bottom, deadlst;

        /* Maximum memory for trillian. */
        unsigned long TR_MAX_MEM;

        /* Indicates whether to destroy the temporal file or not. */
        int tr__remove;

        /* The name of file used for storage. */
        const char *fname;
    }
    spacectx_t;

    /* Starts Trillian, returns zero if no error occurred (max is in mb). */
    int tr__start (const char *fname, unsigned max);

    /* Stops Trillian. */
    void tr__stop (void);

    /* Saves the context of the current space.  */
    void tr__save (spacectx_t *);

    /* Restore the context of the current space. */
    void tr__restore (spacectx_t *);

    /* Allocates a new block and opens it automatically. */
    void *tr__alloc (unsigned len);

    /* Closes a slice (converts to block). */
    void tr__close (void **p, int flush);

    /* Opens a block to be used as a slice. */
    void tr__open (void **p);

    /* Releases a slice. */
    void tr__free (void *p);

    /* Releases a block. */
    void tr__freeb (void *p);

    /* Returns the memory left in the virtual space. */
    unsigned long tr__coreleft (void);

    /* Indicates whether to destroy the temporal file or not. */
    extern int tr__remove;

#endif
