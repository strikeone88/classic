/*
    SSTRING.H

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __SSTRING_H
#define __SSTRING_H

    #ifndef __GSTRING_H
    #include <gstring.h>
    #endif

    #ifndef __GSET_H
    #include <gset.h>
    #endif

    /* String node. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;
        char        *value;
    }
    StrNode;

    /* Initializes the module. */
    void iString (void);

    /* Deinitializes the module. */
    void diString (void);

    /* Adds a string. */
    char *aString (char *value);

    /* Destroys a string. */
    void dString (char *value);

    /* Adds a string (returns node). */
    StrNode *aStringN (char *value);

    /* Destroys a string node. */
    void dStringN (StrNode *);

#endif
