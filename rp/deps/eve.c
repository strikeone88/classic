/*
    EVE.C

    Eve (Hash Generator) Version 0.01

    This is the official RedStar hashing algorithm, given an input
    of data it generates a 128-bit hash for it. The engine is fast
    and very easy to implement, plus this algorithm is free!.

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <string.h>

    /* Eve context structure. */
    typedef struct
    {
        unsigned    short tempbuf [8];
        unsigned    short magicA, magicB;

        unsigned    char resultText [33];
        unsigned    char result [16];
    }
    eve_ctx_t;

    /* Initializes the given context structure and returns it. */
    eve_ctx_t *eve__init (eve_ctx_t *x)
    {
        int i;

            /* The context pointer must be valid. */
            if (!x) return NULL;

            /* The initial 16-bit buffer should be nullified. */
            for (i = 0; i < 8; i++) x->tempbuf [i] = 0;

            /* These are the initial magic values. */
            x->magicA = 0x157A;
            x->magicB = 0xA751;

            return x;
    }

    /* Updates the context buffer using the given buffer. */
    void eve__update (eve_ctx_t *x, char *buf, unsigned len)
    {
        unsigned I, A, B;

            /* If got incorrect parms, then just return. */
            if (!x || !buf || !len) return;

            /* Update the temp buffer using the given buffer. */
            while (len--)
            {
                /* The byte range for this is not from 0 to 255 because
                   the zero value is forbidden. */

                I = 1 + (unsigned char)*buf++;

                /* Store previous magic values. */
                A = x->magicA;
                B = x->magicB;

                /* Update magic values using the character. */
                x->magicA += B - 0x0A*I;
                x->magicB -= A + 0x05*I;

                /* Magic value to be added to the temp buffer. */
                A = x->magicA;

                /* Magic value to select temp buffer index. */
                B = x->magicB;

                /* Update the temporal buffer. */
                for (I = 0; I < 6; I++, B >>= 3)
                    x->tempbuf [B & 0x07] += A;
            }
    }

    /* Finishes the generation process and sets result. */
    void eve__final (eve_ctx_t *x)
    {
        unsigned short A, B;
        int i, j;

            /* The context pointer must be valid. */
            if (!x) return;

            /* The last magic values that were used. */
            A = x->magicA;
            B = x->magicB;

            /* This is the final adjust cicle. */
            for (i = 0; i < 8; i++)
            {
                /* Update the magic values using the previous ones. */
                B -= 0x05*x->magicA;
                A += 0x0A*x->magicB;

                /* Update two consecutive elements. */
                x->tempbuf [(i - 1 + 8) & 7] -= B;
                x->tempbuf [i] += A;

                /* Store magic values. */
                x->magicA = A;
                x->magicB = B;
            }

            /* Set the result. */
            for (i = j = 0; i < 8; i++)
            {
                /* Each 16-bit unsigned word is stored in the result
                   buffer as a big endian word. */

                x->result [j++] = x->tempbuf [i] >> 8;
                x->result [j++] = x->tempbuf [i] & 0xFF;
            }
    }

    /* Using result it builds a 32 hex digit ASCIIz string. */
    char *eve__text (eve_ctx_t *x)
    {
        static char *hexdigit = "0123456789abcdef";
        int i, j;

            for (i = j = 0; i < 16; i++)
            {
                x->resultText [j++] = hexdigit [(x->result [i+0] & 0xF0) >> 4];
                x->resultText [j++] = hexdigit [x->result [i+0] & 0x0F];
            }

            x->resultText [j] = '\0';

            return x->resultText;
    }
