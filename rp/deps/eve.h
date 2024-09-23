/*
    EVE.H

    Eve (Hash Generator) Version 0.01

    This is the official RedStar hashing algorithm, given an input
    of data it generates a 128-bit hash for it. The engine is fast
    and very easy to implement, plus this algorithm is free!.

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __EVE_H
#define __EVE_H

    /* Eve context structure. */
    typedef struct
    {
        unsigned    short tempbuf [8];
        unsigned    short magicA, magicB;

        unsigned    char result [16];
        unsigned    char text [33];
    }
    eve_ctx_t;

    /* Initializes the given context structure and returns it. */
    eve_ctx_t *eve__init (eve_ctx_t *x);

    /* Updates the context buffer using the given buffer. */
    void eve__update (eve_ctx_t *x, char *buf, unsigned len);

    /* Finishes the generation process and sets result and resultText. */
    void eve__final (eve_ctx_t *x);

    /* Using result it builds a 32 hex digit ASCIIz string. */
    char *eve__text (eve_ctx_t *x);

#endif
