/*
    PRSR-R-S.TEM

    Source code template file for the RedStar Paige Parser Generator
    Version 0.18, part of the Pegasus Project.

    Copyright (C) 2007-2010 RedStar Technologies (May 25 2010)
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <string.h>

    /* Simply shifts an empty symbol to take a stack space. */
    #define eshift() args [top++] = buftop; *buftop++ = '\0'

    /* Pushes the given value on the stack. */
    #define push(x) stack [sp++] = x

    /* Maximum stack depth. */
    #ifndef MAX_DEPTH
    #define MAX_DEPTH   1024
    #endif

    /* The parser context structure. */
    typedef struct
    {
        /* The states stack. */
        unsigned stack [MAX_DEPTH];
        unsigned sp;

        /* The arguments and index stack and its top index. */
        unsigned index [MAX_DEPTH];
        char *args [MAX_DEPTH];
        unsigned top;

        /* Buffer to store the arguments and the top pointer. */
        char *buf, *buftop;
    }
    context_t;

    /* The states stack. */
    static unsigned stack [MAX_DEPTH];
    static unsigned sp;

    /* The arguments and index stack and its top index. */
    static unsigned index [MAX_DEPTH];
    static char *args [MAX_DEPTH];
    static unsigned top;

    /* Shift table. */
    static unsigned long *shtbl [];

    /* Shift targets table. */
    static unsigned *sttbl [];

    /* Reductions table. */
    static unsigned long *rdtbl [];

    /* Transitions table. */
    static unsigned *gttbl [];

    /* Reductions parameters table. */
    static unsigned *rptbl [];

    /* Function calls table. */
    static void (*fctbl []) (unsigned, int, char *[], unsigned []);

    /* Nonterminals table. */
    static unsigned nttbl [];

    /* Catcher base table. */
    static unsigned long cbtbl [];

    /* Internal catcher. */
    static int catcher (unsigned, unsigned, unsigned);

    /* Buffer to store the arguments and the top pointer. */
    static char *buf, *buftop;

    /* Buffer to store the scanned symbol. */
    extern char symbolstr [];

    /* The length of the buffer (above). */
    extern int symbollen;

    /* Indicates to finish the parsing immediately. */
    int p__finish;

    /* Returns the current symbol. */
    unsigned long readsymbol (void);

    /* Shifts the current symbol (forces to scan on next read). */
    void shiftsymbol (void);

    /* Symbol to push back to the stack (after reduction call). */
    static unsigned pushback__index;
    static char *pushback__value;

    /* This function sets the pushback__xx internal variables. */
    void p__pushback (char *value, unsigned index)
    {
            pushback__value = value;
            pushback__index = index;
    }

    /* Pops x elements and returns the top value. */
    static unsigned pop (unsigned x)
    {
            if (x) buftop = args [top -= x];

            return stack [(sp -= x) - 1];
    }

    /* Shifts the given symbol and returns the next one. */
    static unsigned long shift (unsigned long sym)
    {
            index [top] = sym >> 16;
            args [top++] = buftop;

            memcpy (buftop, symbolstr, symbollen);

            buftop [symbollen] = '\0';

            buftop += symbollen + 1;

            shiftsymbol ();

            return readsymbol ();
    }

    /* Searches for the given key on the given list. */
    static unsigned srch_key (unsigned count, unsigned long *lb,
                              unsigned long key)
    {
        unsigned cnt = count, damper = 0;
        register unsigned long elem;
        unsigned long *p = lb;
        register unsigned mid, x;

            x = 1;

            while (count)
            {
                mid = count >> 1;

                elem = lb [mid];

                if (elem >> 16 == 0x7FFFU)
                {
                    if ((unsigned short)elem == (unsigned short)key)
                        damper = x + mid;
                }

                if (elem == key) return x + mid;

                if (elem < key)
                {
                    mid++;

                    lb += mid;
                    x += mid;

                    count -= mid;
                }
                else
                    count = mid;
            }

            if (!damper)
            {
                while (cnt--)
                {
                    elem = p [cnt];

                    if (elem >> 16 != 0x7FFFU) break;

                    if ((unsigned short)elem == (unsigned short)key)
                    {
                        damper = cnt + 1;
                        break;
                    }
                }
            }

            return damper;
    }

    /* Searches for the given target on the given list. */
    static unsigned srch_target (unsigned count, unsigned *lb,
                                 unsigned key)
    {
        register unsigned mid, p;

            while (count)
            {
                mid = count >> 1;
                p = mid << 1;

                if (lb [p] == key) return lb [p + 1];

                if (lb [p] < key)
                {
                    count -= mid + 1;
                    lb += p + 2;
                }
                else
                    count = mid;
            }

            return 0;
    }

    /* Runs the parser. */
    static int parser__run (void)
    {
        register unsigned long *lptr;
        register unsigned long symbol;
        register unsigned *iptr;
        unsigned i, j, k, r, state;

            symbol = readsymbol ();

            push (0);

            while (1)
            {
                state = stack [sp - 1];

                lptr = shtbl [state];
                iptr = sttbl [state];

                if (lptr != NULL)
                {
                    if ((i = srch_key (*lptr, lptr + 1, symbol)) != 0)
                    {
                        symbol = shift (symbol);
                        push (iptr [i - 1]);

                        continue;
                    }
                }

                lptr = rdtbl [state];
                iptr = rptbl [state];

                if (lptr != NULL)
                {
                    if ((i = srch_key (*lptr, lptr + 1, symbol)) != 0)
                    {
                        i = (i << 1) + i - 3;

               reduce:  r = iptr [i + 0];
                        j = iptr [i + 1];
                        k = iptr [i + 2];

                        if (!r) return 1;

                        pushback__value = NULL;

                        if (fctbl [j] != NULL)
                        {
                            i = top - k;

                            fctbl [j] (r - nttbl [j], k,
                                             &args [i], &index [i]);

                            if (p__finish) return p__finish;
                        }

                        state = pop (k);

                        iptr = gttbl [state];

                        i = srch_target (*iptr, iptr + 1, j);
                        
                        push (i);

                        if (pushback__value)
                        {
                            index [top] = pushback__index;
                            args [top++] = buftop;

                            i = strlen (pushback__value);

                            memcpy (buftop, pushback__value, i);
                            buftop [i++] = '\0';
                            buftop += i;
                        }
                        else
                            eshift ();

                        continue;
                    }

                    if (lptr [1] == 0) goto reduce;
                }

                break;
            }

            lptr = cbtbl;
            j = state;
            i = 1;

            while (sp > 0 && !lptr [state])
            {
                state = pop (1);
                i++;
            }

            return catcher (lptr [state] >> 16, i, j);
    }

    /* Parses the input stream. */
    int parseinput (void)
    {
        static int initialized = 0;

            if (!initialized)
            {
                buf = malloc (4096);
                initialized = 1;
            }

            sp = top = p__finish = 0;
            buftop = buf;

            return parser__run ();
    }

    /* Saves the context of the parser and returns a pointer as void
       to the structure, returns NULL if can't save the context. */

    void *p__savectx (void)
    {
        context_t *c = malloc (sizeof (context_t));

            if (c == NULL) return NULL;

            /* Save all the single variables. */
            c->buftop = buftop;
            c->top = top;
            c->buf = buf;
            c->sp = sp;

            /* Save all the arrays. */
            memcpy (c->stack, stack, sizeof (stack));
            memcpy (c->index, index, sizeof (index));
            memcpy (c->args, args, sizeof (args));

            return (void *)c;
    }

    /* Loads the context of the parser. */
    void p__loadctx (context_t *c)
    {
            if (c == NULL) return;

            /* Restore all the single variables. */
            buftop = c->buftop;
            top = c->top;
            buf = c->buf;
            sp = c->sp;

            /* Save all the arrays. */
            memcpy (stack, c->stack, sizeof (stack));
            memcpy (index, c->index, sizeof (index));
            memcpy (args, c->args, sizeof (args));

            free (c);
    }

/****************************************************************************/
    #define ppNULL  NULL

    static unsigned gt0000 [] =
    {
        0x0002, 0x0001, 0x0001, 0x0004, 0x0002, 
    };

    static unsigned gt0002 [] =
    {
        0x0002, 0x0005, 0x0004, 0x0006, 0x0003, 
    };

    static unsigned gt0006 [] =
    {
        0x0001, 0x0002, 0x000d, 
    };

    static unsigned gt0008 [] =
    {
        0x0001, 0x0017, 0x000e, 
    };

    static unsigned gt0012 [] =
    {
        0x0002, 0x0007, 0x0013, 0x0009, 0x0014, 
    };

    static unsigned gt0013 [] =
    {
        0x0005, 0x0003, 0x0017, 0x0005, 0x0019, 0x0006, 0x001a, 0x000b, 
        0x0018, 0x000c, 0x001b, 
    };

    static unsigned gt0014 [] =
    {
        0x0001, 0x0017, 0x001e, 
    };

    static unsigned gt0016 [] =
    {
        0x0002, 0x0007, 0x001f, 0x0009, 0x0014, 
    };

    static unsigned gt0017 [] =
    {
        0x0002, 0x0007, 0x0020, 0x0009, 0x0014, 
    };

    static unsigned gt0018 [] =
    {
        0x0002, 0x0007, 0x0021, 0x0009, 0x0014, 
    };

    static unsigned gt0020 [] =
    {
        0x0001, 0x0008, 0x0022, 
    };

    static unsigned gt0024 [] =
    {
        0x0001, 0x000a, 0x0024, 
    };

    static unsigned gt0027 [] =
    {
        0x0001, 0x000d, 0x0025, 
    };

    static unsigned gt0030 [] =
    {
        0x0002, 0x0007, 0x0029, 0x0009, 0x0014, 
    };

    static unsigned gt0036 [] =
    {
        0x0002, 0x000b, 0x002c, 0x000c, 0x001b, 
    };

    static unsigned gt0038 [] =
    {
        0x0001, 0x000a, 0x002d, 
    };

    static unsigned gt0039 [] =
    {
        0x0002, 0x000b, 0x002e, 0x000c, 0x001b, 
    };

    static unsigned gt0040 [] =
    {
        0x0004, 0x0007, 0x004a, 0x0009, 0x0014, 0x000e, 0x002f, 0x000f, 
        0x0031, 
    };

    static unsigned gt0041 [] =
    {
        0x0001, 0x0016, 0x0053, 
    };

    static unsigned gt0045 [] =
    {
        0x0002, 0x000b, 0x002c, 0x000c, 0x001b, 
    };

    static unsigned gt0084 [] =
    {
        0x0002, 0x0015, 0x007b, 0x0017, 0x007c, 
    };

    static unsigned gt0089 [] =
    {
        0x0003, 0x0007, 0x0096, 0x0009, 0x0014, 0x000f, 0x007d, 
    };

    static unsigned gt0090 [] =
    {
        0x0002, 0x0007, 0x009f, 0x0009, 0x0014, 
    };

    static unsigned gt0091 [] =
    {
        0x0002, 0x0015, 0x00a1, 0x0017, 0x007c, 
    };

    static unsigned gt0092 [] =
    {
        0x0002, 0x0015, 0x00a2, 0x0017, 0x007c, 
    };

    static unsigned gt0093 [] =
    {
        0x0001, 0x0014, 0x00a3, 
    };

    static unsigned gt0094 [] =
    {
        0x0003, 0x0010, 0x00a7, 0x0011, 0x00a9, 0x0017, 0x00a8, 
    };

    static unsigned gt0095 [] =
    {
        0x0003, 0x0010, 0x00ac, 0x0011, 0x00a9, 0x0017, 0x00a8, 
    };

    static unsigned gt0096 [] =
    {
        0x0003, 0x0011, 0x00ae, 0x0012, 0x00ad, 0x0017, 0x00af, 
    };

    static unsigned gt0097 [] =
    {
        0x0003, 0x0011, 0x00ae, 0x0012, 0x00b3, 0x0017, 0x00af, 
    };

    static unsigned gt0098 [] =
    {
        0x0001, 0x0013, 0x00b4, 
    };

    static unsigned gt0099 [] =
    {
        0x0001, 0x0016, 0x00b8, 
    };

    static unsigned gt0103 [] =
    {
        0x0001, 0x0017, 0x00bf, 
    };

    static unsigned gt0105 [] =
    {
        0x0003, 0x0010, 0x00c2, 0x0011, 0x00a9, 0x0017, 0x00a8, 
    };

    static unsigned gt0106 [] =
    {
        0x0003, 0x0010, 0x00c3, 0x0011, 0x00a9, 0x0017, 0x00a8, 
    };

    static unsigned gt0107 [] =
    {
        0x0003, 0x0011, 0x00ae, 0x0012, 0x00c4, 0x0017, 0x00af, 
    };

    static unsigned gt0108 [] =
    {
        0x0003, 0x0011, 0x00ae, 0x0012, 0x00c5, 0x0017, 0x00af, 
    };

    static unsigned gt0109 [] =
    {
        0x0001, 0x0016, 0x00c6, 
    };

    static unsigned gt0110 [] =
    {
        0x0001, 0x0016, 0x00c7, 
    };

    static unsigned gt0111 [] =
    {
        0x0001, 0x0018, 0x00c8, 
    };

    static unsigned gt0112 [] =
    {
        0x0001, 0x0017, 0x00d2, 
    };

    static unsigned gt0113 [] =
    {
        0x0001, 0x0016, 0x00d4, 
    };

    static unsigned gt0115 [] =
    {
        0x0001, 0x0016, 0x00d7, 
    };

    static unsigned gt0118 [] =
    {
        0x0001, 0x0017, 0x00da, 
    };

    static unsigned gt0119 [] =
    {
        0x0001, 0x0017, 0x00db, 
    };

    static unsigned gt0120 [] =
    {
        0x0001, 0x0017, 0x00dc, 
    };

    static unsigned gt0177 [] =
    {
        0x0001, 0x0017, 0x00e4, 
    };

    static unsigned gt0185 [] =
    {
        0x0002, 0x0015, 0x007b, 0x0017, 0x007c, 
    };

    static unsigned gt0211 [] =
    {
        0x0001, 0x0017, 0x00e8, 
    };

    static unsigned gt0223 [] =
    {
        0x0001, 0x0017, 0x00ee, 
    };

    static unsigned gt0224 [] =
    {
        0x0001, 0x0017, 0x00ef, 
    };

    static unsigned gt0225 [] =
    {
        0x0001, 0x0016, 0x00f0, 
    };

    static unsigned gt0226 [] =
    {
        0x0001, 0x0016, 0x00f4, 
    };

    static unsigned gt0230 [] =
    {
        0x0001, 0x0016, 0x00f7, 
    };

    static unsigned gt0234 [] =
    {
        0x0001, 0x0017, 0x00fa, 
    };

    static unsigned gt0235 [] =
    {
        0x0001, 0x0017, 0x00fb, 
    };

    static unsigned gt0236 [] =
    {
        0x0001, 0x0017, 0x00fc, 
    };

    static unsigned gt0241 [] =
    {
        0x0002, 0x0015, 0x00ff, 0x0017, 0x0100, 
    };

    static unsigned gt0248 [] =
    {
        0x0001, 0x0017, 0x0105, 
    };

    static unsigned gt0259 [] =
    {
        0x0001, 0x0016, 0x0108, 
    };

    static unsigned gt0262 [] =
    {
        0x0001, 0x0017, 0x00ee, 
    };

    static unsigned gt0263 [] =
    {
        0x0001, 0x0017, 0x00ef, 
    };

    static unsigned gt0267 [] =
    {
        0x0002, 0x0016, 0x010c, 0x0017, 0x010d, 
    };

    static unsigned gt0271 [] =
    {
        0x0001, 0x0017, 0x0111, 
    };

    /* Transitions Table */
    static unsigned *gttbl [] =
    {
        gt0000, ppNULL, gt0002, ppNULL, ppNULL, ppNULL, gt0006, ppNULL, 
        gt0008, ppNULL, ppNULL, ppNULL, gt0012, gt0013, gt0014, ppNULL, 
        gt0016, gt0017, gt0018, ppNULL, gt0020, ppNULL, ppNULL, ppNULL, 
        gt0024, ppNULL, ppNULL, gt0027, ppNULL, ppNULL, gt0030, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, gt0036, ppNULL, gt0038, gt0039, 
        gt0040, gt0041, ppNULL, ppNULL, ppNULL, gt0045, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, gt0084, ppNULL, ppNULL, ppNULL, 
        ppNULL, gt0089, gt0090, gt0091, gt0092, gt0093, gt0094, gt0095, 
        gt0096, gt0097, gt0098, gt0099, ppNULL, ppNULL, ppNULL, gt0103, 
        ppNULL, gt0105, gt0106, gt0107, gt0108, gt0109, gt0110, gt0111, 
        gt0112, gt0113, ppNULL, gt0115, ppNULL, ppNULL, gt0118, gt0119, 
        gt0120, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, gt0177, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, gt0185, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, gt0211, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, gt0223, 
        gt0224, gt0225, gt0226, ppNULL, ppNULL, ppNULL, gt0230, ppNULL, 
        ppNULL, ppNULL, gt0234, gt0235, gt0236, ppNULL, ppNULL, ppNULL, 
        ppNULL, gt0241, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        gt0248, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, gt0259, ppNULL, ppNULL, gt0262, gt0263, 
        ppNULL, ppNULL, ppNULL, gt0267, ppNULL, ppNULL, ppNULL, gt0271, 
        ppNULL, ppNULL, ppNULL, 
    };

    static unsigned long sh0002 [] =
    {
        0x00000001, 0x00000025, 
    };

    static unsigned st0002 [] =
    {
        0x0005, 
    };

    static unsigned long sh0003 [] =
    {
        0x00000001, 0x0000003b, 
    };

    static unsigned st0003 [] =
    {
        0x0006, 
    };

    static unsigned long sh0004 [] =
    {
        0x00000001, 0x0000003b, 
    };

    static unsigned st0004 [] =
    {
        0x0007, 
    };

    static unsigned long sh0005 [] =
    {
        0x00000005, 0x0008010b, 0x0009010b, 0x000a010b, 0x000b010b, 
        0x000c010b, 
    };

    static unsigned st0005 [] =
    {
        0x0008, 0x000c, 0x0009, 0x000a, 0x000b, 
    };

    static unsigned long sh0008 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0008 [] =
    {
        0x000f, 
    };

    static unsigned long sh0009 [] =
    {
        0x00000001, 0x00000105, 
    };

    static unsigned st0009 [] =
    {
        0x0010, 
    };

    static unsigned long sh0010 [] =
    {
        0x00000001, 0x00000105, 
    };

    static unsigned st0010 [] =
    {
        0x0011, 
    };

    static unsigned long sh0011 [] =
    {
        0x00000001, 0x00000105, 
    };

    static unsigned st0011 [] =
    {
        0x0012, 
    };

    static unsigned long sh0012 [] =
    {
        0x00000002, 0x00000108, 0x00000109, 
    };

    static unsigned st0012 [] =
    {
        0x0015, 0x0016, 
    };

    static unsigned long sh0013 [] =
    {
        0x00000002, 0x00000025, 0x7fff010c, 
    };

    static unsigned st0013 [] =
    {
        0x001c, 0x001d, 
    };

    static unsigned long sh0014 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0014 [] =
    {
        0x000f, 
    };

    static unsigned long sh0016 [] =
    {
        0x00000002, 0x00000108, 0x00000109, 
    };

    static unsigned st0016 [] =
    {
        0x0015, 0x0016, 
    };

    static unsigned long sh0017 [] =
    {
        0x00000002, 0x00000108, 0x00000109, 
    };

    static unsigned st0017 [] =
    {
        0x0015, 0x0016, 
    };

    static unsigned long sh0018 [] =
    {
        0x00000002, 0x00000108, 0x00000109, 
    };

    static unsigned st0018 [] =
    {
        0x0015, 0x0016, 
    };

    static unsigned long sh0023 [] =
    {
        0x00000001, 0x0000003b, 
    };

    static unsigned st0023 [] =
    {
        0x0023, 
    };

    static unsigned long sh0027 [] =
    {
        0x00000002, 0x0000003a, 0x0000007b, 
    };

    static unsigned st0027 [] =
    {
        0x0027, 0x0026, 
    };

    static unsigned long sh0028 [] =
    {
        0x00000005, 0x0008010b, 0x0009010b, 0x000a010b, 0x000b010b, 
        0x000c010b, 
    };

    static unsigned st0028 [] =
    {
        0x0008, 0x000c, 0x0009, 0x000a, 0x000b, 
    };

    static unsigned long sh0029 [] =
    {
        0x00000001, 0x00000028, 
    };

    static unsigned st0029 [] =
    {
        0x0028, 
    };

    static unsigned long sh0030 [] =
    {
        0x00000002, 0x00000108, 0x00000109, 
    };

    static unsigned st0030 [] =
    {
        0x0015, 0x0016, 
    };

    static unsigned long sh0034 [] =
    {
        0x00000002, 0x00000108, 0x00000109, 
    };

    static unsigned st0034 [] =
    {
        0x002a, 0x002b, 
    };

    static unsigned long sh0036 [] =
    {
        0x00000001, 0x7fff010c, 
    };

    static unsigned st0036 [] =
    {
        0x001d, 
    };

    static unsigned long sh0039 [] =
    {
        0x00000001, 0x7fff010c, 
    };

    static unsigned st0039 [] =
    {
        0x001d, 
    };

    static unsigned long sh0040 [] =
    {
        0x00000023, 0x00000029, 0x00000108, 0x00000109, 0x0000010d, 
        0x0001010d, 0x0001010e, 0x0002010d, 0x0002010e, 0x0003010d, 
        0x0003010e, 0x0004010d, 0x0005010d, 0x0006010d, 0x0007010d, 
        0x0008010d, 0x0009010d, 0x000a010d, 0x000b010d, 0x000c010d, 
        0x000d010b, 0x000d010d, 0x000e010b, 0x000e010d, 0x000f010b, 
        0x000f010d, 0x0010010b, 0x0010010d, 0x0011010d, 0x0012010d, 
        0x0013010d, 0x0014010d, 0x0015010d, 0x0016010d, 0x0017010d, 
        0x0018010d, 
    };

    static unsigned st0040 [] =
    {
        0x0030, 0x0015, 0x0016, 0x0036, 0x0037, 0x0051, 0x0038, 0x0050, 
        0x0039, 0x004f, 0x003a, 0x003b, 0x0035, 0x0033, 0x0034, 0x0032, 
        0x003f, 0x0041, 0x0042, 0x003c, 0x0043, 0x003d, 0x0044, 0x003e, 
        0x0045, 0x0040, 0x0046, 0x0047, 0x0048, 0x0049, 0x004b, 0x004c, 
        0x004d, 0x004e, 0x0052, 
    };

    static unsigned long sh0041 [] =
    {
        0x00000003, 0x0000003c, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0041 [] =
    {
        0x0054, 0x0056, 0x0055, 
    };

    static unsigned long sh0045 [] =
    {
        0x00000002, 0x0000007d, 0x7fff010c, 
    };

    static unsigned st0045 [] =
    {
        0x0057, 0x001d, 
    };

    static unsigned long sh0047 [] =
    {
        0x00000002, 0x00000029, 0x0000002c, 
    };

    static unsigned st0047 [] =
    {
        0x0058, 0x0059, 
    };

    static unsigned long sh0050 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0050 [] =
    {
        0x005a, 
    };

    static unsigned long sh0051 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0051 [] =
    {
        0x005b, 
    };

    static unsigned long sh0052 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0052 [] =
    {
        0x005c, 
    };

    static unsigned long sh0053 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0053 [] =
    {
        0x005d, 
    };

    static unsigned long sh0054 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0054 [] =
    {
        0x005e, 
    };

    static unsigned long sh0055 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0055 [] =
    {
        0x005f, 
    };

    static unsigned long sh0056 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0056 [] =
    {
        0x0060, 
    };

    static unsigned long sh0057 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0057 [] =
    {
        0x0061, 
    };

    static unsigned long sh0058 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0058 [] =
    {
        0x0062, 
    };

    static unsigned long sh0059 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0059 [] =
    {
        0x0063, 
    };

    static unsigned long sh0060 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0060 [] =
    {
        0x0064, 
    };

    static unsigned long sh0061 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0061 [] =
    {
        0x0065, 
    };

    static unsigned long sh0062 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0062 [] =
    {
        0x0066, 
    };

    static unsigned long sh0063 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0063 [] =
    {
        0x0067, 
    };

    static unsigned long sh0064 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0064 [] =
    {
        0x0068, 
    };

    static unsigned long sh0065 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0065 [] =
    {
        0x0069, 
    };

    static unsigned long sh0066 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0066 [] =
    {
        0x006a, 
    };

    static unsigned long sh0067 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0067 [] =
    {
        0x006b, 
    };

    static unsigned long sh0068 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0068 [] =
    {
        0x006c, 
    };

    static unsigned long sh0069 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0069 [] =
    {
        0x006d, 
    };

    static unsigned long sh0070 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0070 [] =
    {
        0x006e, 
    };

    static unsigned long sh0071 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0071 [] =
    {
        0x006f, 
    };

    static unsigned long sh0072 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0072 [] =
    {
        0x0070, 
    };

    static unsigned long sh0073 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0073 [] =
    {
        0x0071, 
    };

    static unsigned long sh0075 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0075 [] =
    {
        0x0072, 
    };

    static unsigned long sh0076 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0076 [] =
    {
        0x0073, 
    };

    static unsigned long sh0077 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0077 [] =
    {
        0x0074, 
    };

    static unsigned long sh0078 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0078 [] =
    {
        0x0075, 
    };

    static unsigned long sh0079 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0079 [] =
    {
        0x0076, 
    };

    static unsigned long sh0080 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0080 [] =
    {
        0x0077, 
    };

    static unsigned long sh0081 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0081 [] =
    {
        0x0078, 
    };

    static unsigned long sh0082 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0082 [] =
    {
        0x0079, 
    };

    static unsigned long sh0083 [] =
    {
        0x00000001, 0x00000028, 
    };

    static unsigned st0083 [] =
    {
        0x007a, 
    };

    static unsigned long sh0084 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0084 [] =
    {
        0x000f, 
    };

    static unsigned long sh0089 [] =
    {
        0x00000022, 0x00000108, 0x00000109, 0x0000010d, 0x0001010d, 
        0x0001010e, 0x0002010d, 0x0002010e, 0x0003010d, 0x0003010e, 
        0x0004010d, 0x0005010d, 0x0006010d, 0x0007010d, 0x0008010d, 
        0x0009010d, 0x000a010d, 0x000b010d, 0x000c010d, 0x000d010b, 
        0x000d010d, 0x000e010b, 0x000e010d, 0x000f010b, 0x000f010d, 
        0x0010010b, 0x0010010d, 0x0011010d, 0x0012010d, 0x0013010d, 
        0x0014010d, 0x0015010d, 0x0016010d, 0x0017010d, 0x0018010d, 
    };

    static unsigned st0089 [] =
    {
        0x0015, 0x0016, 0x0082, 0x0083, 0x009d, 0x0084, 0x009c, 0x0085, 
        0x009b, 0x0086, 0x0087, 0x0081, 0x007f, 0x0080, 0x007e, 0x008b, 
        0x008d, 0x008e, 0x0088, 0x008f, 0x0089, 0x0090, 0x008a, 0x0091, 
        0x008c, 0x0092, 0x0093, 0x0094, 0x0095, 0x0097, 0x0098, 0x0099, 
        0x009a, 0x009e, 
    };

    static unsigned long sh0090 [] =
    {
        0x00000003, 0x00000108, 0x00000109, 0x7fff0111, 
    };

    static unsigned st0090 [] =
    {
        0x0015, 0x0016, 0x00a0, 
    };

    static unsigned long sh0091 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0091 [] =
    {
        0x000f, 
    };

    static unsigned long sh0092 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0092 [] =
    {
        0x000f, 
    };

    static unsigned long sh0093 [] =
    {
        0x00000003, 0x0002010b, 0x0006010b, 0x0007010b, 
    };

    static unsigned st0093 [] =
    {
        0x00a6, 0x00a5, 0x00a4, 
    };

    static unsigned long sh0094 [] =
    {
        0x00000003, 0x00000106, 0x0000010b, 0x0001010b, 
    };

    static unsigned st0094 [] =
    {
        0x000f, 0x00ab, 0x00aa, 
    };

    static unsigned long sh0095 [] =
    {
        0x00000003, 0x00000106, 0x0000010b, 0x0001010b, 
    };

    static unsigned st0095 [] =
    {
        0x000f, 0x00ab, 0x00aa, 
    };

    static unsigned long sh0096 [] =
    {
        0x00000004, 0x0000002d, 0x00000106, 0x0000010b, 0x0011010b, 
    };

    static unsigned st0096 [] =
    {
        0x00b1, 0x000f, 0x00b0, 0x00b2, 
    };

    static unsigned long sh0097 [] =
    {
        0x00000004, 0x0000002d, 0x00000106, 0x0000010b, 0x0011010b, 
    };

    static unsigned st0097 [] =
    {
        0x00b1, 0x000f, 0x00b0, 0x00b2, 
    };

    static unsigned long sh0098 [] =
    {
        0x00000003, 0x0003010b, 0x0004010b, 0x0005010b, 
    };

    static unsigned st0098 [] =
    {
        0x00b7, 0x00b6, 0x00b5, 
    };

    static unsigned long sh0099 [] =
    {
        0x00000003, 0x0000003c, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0099 [] =
    {
        0x00b9, 0x00bb, 0x00ba, 
    };

    static unsigned long sh0100 [] =
    {
        0x00000001, 0x00000105, 
    };

    static unsigned st0100 [] =
    {
        0x00bc, 
    };

    static unsigned long sh0101 [] =
    {
        0x00000001, 0x00000105, 
    };

    static unsigned st0101 [] =
    {
        0x00bd, 
    };

    static unsigned long sh0102 [] =
    {
        0x00000001, 0x00000105, 
    };

    static unsigned st0102 [] =
    {
        0x00be, 
    };

    static unsigned long sh0103 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0103 [] =
    {
        0x000f, 
    };

    static unsigned long sh0104 [] =
    {
        0x00000002, 0x0001010c, 0x0002010c, 
    };

    static unsigned st0104 [] =
    {
        0x00c0, 0x00c1, 
    };

    static unsigned long sh0105 [] =
    {
        0x00000003, 0x00000106, 0x0000010b, 0x0001010b, 
    };

    static unsigned st0105 [] =
    {
        0x000f, 0x00ab, 0x00aa, 
    };

    static unsigned long sh0106 [] =
    {
        0x00000003, 0x00000106, 0x0000010b, 0x0001010b, 
    };

    static unsigned st0106 [] =
    {
        0x000f, 0x00ab, 0x00aa, 
    };

    static unsigned long sh0107 [] =
    {
        0x00000004, 0x0000002d, 0x00000106, 0x0000010b, 0x0011010b, 
    };

    static unsigned st0107 [] =
    {
        0x00b1, 0x000f, 0x00b0, 0x00b2, 
    };

    static unsigned long sh0108 [] =
    {
        0x00000004, 0x0000002d, 0x00000106, 0x0000010b, 0x0011010b, 
    };

    static unsigned st0108 [] =
    {
        0x00b1, 0x000f, 0x00b0, 0x00b2, 
    };

    static unsigned long sh0109 [] =
    {
        0x00000003, 0x0000003c, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0109 [] =
    {
        0x00b9, 0x00bb, 0x00ba, 
    };

    static unsigned long sh0110 [] =
    {
        0x00000003, 0x0000003c, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0110 [] =
    {
        0x00b9, 0x00bb, 0x00ba, 
    };

    static unsigned long sh0111 [] =
    {
        0x00000008, 0x0000010f, 0x00000113, 0x0001010f, 0x00010113, 
        0x0002010f, 0x0003010f, 0x0012010b, 0x0013010b, 
    };

    static unsigned st0111 [] =
    {
        0x00c9, 0x00cf, 0x00ca, 0x00d0, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 
    };

    static unsigned long sh0112 [] =
    {
        0x00000003, 0x0000002f, 0x00000106, 0x7fff0112, 
    };

    static unsigned st0112 [] =
    {
        0x00d3, 0x000f, 0x00d1, 
    };

    static unsigned long sh0113 [] =
    {
        0x00000003, 0x0000003c, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0113 [] =
    {
        0x00b9, 0x00bb, 0x00ba, 
    };

    static unsigned long sh0114 [] =
    {
        0x00000002, 0x0001010b, 0x7fff0113, 
    };

    static unsigned st0114 [] =
    {
        0x00d6, 0x00d5, 
    };

    static unsigned long sh0115 [] =
    {
        0x00000003, 0x0000003c, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0115 [] =
    {
        0x00b9, 0x00bb, 0x00ba, 
    };

    static unsigned long sh0116 [] =
    {
        0x00000001, 0x7fff0112, 
    };

    static unsigned st0116 [] =
    {
        0x00d8, 
    };

    static unsigned long sh0117 [] =
    {
        0x00000001, 0x7fff0112, 
    };

    static unsigned st0117 [] =
    {
        0x00d9, 
    };

    static unsigned long sh0118 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0118 [] =
    {
        0x000f, 
    };

    static unsigned long sh0119 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0119 [] =
    {
        0x000f, 
    };

    static unsigned long sh0120 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0120 [] =
    {
        0x000f, 
    };

    static unsigned long sh0121 [] =
    {
        0x00000001, 0x7fff0112, 
    };

    static unsigned st0121 [] =
    {
        0x00dd, 
    };

    static unsigned long sh0122 [] =
    {
        0x00000001, 0x7fff0110, 
    };

    static unsigned st0122 [] =
    {
        0x00de, 
    };

    static unsigned long sh0123 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static unsigned st0123 [] =
    {
        0x00df, 
    };

    static unsigned long sh0124 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static unsigned st0124 [] =
    {
        0x00e0, 
    };

    static unsigned long sh0126 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0126 [] =
    {
        0x005a, 
    };

    static unsigned long sh0127 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0127 [] =
    {
        0x005b, 
    };

    static unsigned long sh0128 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0128 [] =
    {
        0x005c, 
    };

    static unsigned long sh0129 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0129 [] =
    {
        0x005d, 
    };

    static unsigned long sh0130 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0130 [] =
    {
        0x005e, 
    };

    static unsigned long sh0131 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0131 [] =
    {
        0x005f, 
    };

    static unsigned long sh0132 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0132 [] =
    {
        0x0060, 
    };

    static unsigned long sh0133 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0133 [] =
    {
        0x0061, 
    };

    static unsigned long sh0134 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0134 [] =
    {
        0x0062, 
    };

    static unsigned long sh0135 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0135 [] =
    {
        0x0063, 
    };

    static unsigned long sh0136 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0136 [] =
    {
        0x0064, 
    };

    static unsigned long sh0137 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0137 [] =
    {
        0x0065, 
    };

    static unsigned long sh0138 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0138 [] =
    {
        0x0066, 
    };

    static unsigned long sh0139 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0139 [] =
    {
        0x0067, 
    };

    static unsigned long sh0140 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0140 [] =
    {
        0x0068, 
    };

    static unsigned long sh0141 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0141 [] =
    {
        0x0069, 
    };

    static unsigned long sh0142 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0142 [] =
    {
        0x006a, 
    };

    static unsigned long sh0143 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0143 [] =
    {
        0x006b, 
    };

    static unsigned long sh0144 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0144 [] =
    {
        0x006c, 
    };

    static unsigned long sh0145 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0145 [] =
    {
        0x006d, 
    };

    static unsigned long sh0146 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0146 [] =
    {
        0x006e, 
    };

    static unsigned long sh0147 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0147 [] =
    {
        0x006f, 
    };

    static unsigned long sh0148 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0148 [] =
    {
        0x0070, 
    };

    static unsigned long sh0149 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0149 [] =
    {
        0x0071, 
    };

    static unsigned long sh0151 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0151 [] =
    {
        0x0072, 
    };

    static unsigned long sh0152 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0152 [] =
    {
        0x0073, 
    };

    static unsigned long sh0153 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0153 [] =
    {
        0x0074, 
    };

    static unsigned long sh0154 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0154 [] =
    {
        0x0075, 
    };

    static unsigned long sh0155 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0155 [] =
    {
        0x0076, 
    };

    static unsigned long sh0156 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0156 [] =
    {
        0x0077, 
    };

    static unsigned long sh0157 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0157 [] =
    {
        0x0078, 
    };

    static unsigned long sh0158 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static unsigned st0158 [] =
    {
        0x0079, 
    };

    static unsigned long sh0164 [] =
    {
        0x00000001, 0x00000028, 
    };

    static unsigned st0164 [] =
    {
        0x00e1, 
    };

    static unsigned long sh0165 [] =
    {
        0x00000001, 0x00000028, 
    };

    static unsigned st0165 [] =
    {
        0x00e2, 
    };

    static unsigned long sh0168 [] =
    {
        0x00000001, 0x00000025, 
    };

    static unsigned st0168 [] =
    {
        0x00e3, 
    };

    static unsigned long sh0175 [] =
    {
        0x00000001, 0x00000025, 
    };

    static unsigned st0175 [] =
    {
        0x00e3, 
    };

    static unsigned long sh0177 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0177 [] =
    {
        0x000f, 
    };

    static unsigned long sh0181 [] =
    {
        0x00000001, 0x00000028, 
    };

    static unsigned st0181 [] =
    {
        0x00e5, 
    };

    static unsigned long sh0182 [] =
    {
        0x00000001, 0x00000028, 
    };

    static unsigned st0182 [] =
    {
        0x00e6, 
    };

    static unsigned long sh0185 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0185 [] =
    {
        0x000f, 
    };

    static unsigned long sh0210 [] =
    {
        0x00000001, 0x00000025, 
    };

    static unsigned st0210 [] =
    {
        0x00e7, 
    };

    static unsigned long sh0211 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0211 [] =
    {
        0x00e9, 
    };

    static unsigned long sh0218 [] =
    {
        0x00000001, 0x0000010a, 
    };

    static unsigned st0218 [] =
    {
        0x00ea, 
    };

    static unsigned long sh0219 [] =
    {
        0x00000001, 0x0000010a, 
    };

    static unsigned st0219 [] =
    {
        0x00eb, 
    };

    static unsigned long sh0220 [] =
    {
        0x00000001, 0x0000010a, 
    };

    static unsigned st0220 [] =
    {
        0x00ec, 
    };

    static unsigned long sh0222 [] =
    {
        0x00000001, 0x00000029, 
    };

    static unsigned st0222 [] =
    {
        0x00ed, 
    };

    static unsigned long sh0223 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0223 [] =
    {
        0x00e9, 
    };

    static unsigned long sh0224 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0224 [] =
    {
        0x00e9, 
    };

    static unsigned long sh0225 [] =
    {
        0x00000003, 0x0000003c, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0225 [] =
    {
        0x00f1, 0x00f3, 0x00f2, 
    };

    static unsigned long sh0226 [] =
    {
        0x00000003, 0x0000003c, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0226 [] =
    {
        0x00f1, 0x00f3, 0x00f2, 
    };

    static unsigned long sh0227 [] =
    {
        0x00000001, 0x00000025, 
    };

    static unsigned st0227 [] =
    {
        0x00f5, 
    };

    static unsigned long sh0229 [] =
    {
        0x00000001, 0x7fff010f, 
    };

    static unsigned st0229 [] =
    {
        0x00f6, 
    };

    static unsigned long sh0230 [] =
    {
        0x00000003, 0x0000003c, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0230 [] =
    {
        0x00f1, 0x00f3, 0x00f2, 
    };

    static unsigned long sh0231 [] =
    {
        0x00000001, 0x0000002f, 
    };

    static unsigned st0231 [] =
    {
        0x00f8, 
    };

    static unsigned long sh0232 [] =
    {
        0x00000001, 0x00000025, 
    };

    static unsigned st0232 [] =
    {
        0x00f9, 
    };

    static unsigned long sh0234 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0234 [] =
    {
        0x00e9, 
    };

    static unsigned long sh0235 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0235 [] =
    {
        0x00e9, 
    };

    static unsigned long sh0236 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0236 [] =
    {
        0x00e9, 
    };

    static unsigned long sh0238 [] =
    {
        0x00000001, 0x0000003e, 
    };

    static unsigned st0238 [] =
    {
        0x00fd, 
    };

    static unsigned long sh0240 [] =
    {
        0x00000001, 0x00000029, 
    };

    static unsigned st0240 [] =
    {
        0x00fe, 
    };

    static unsigned long sh0241 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0241 [] =
    {
        0x0101, 
    };

    static unsigned long sh0244 [] =
    {
        0x00000001, 0x00000029, 
    };

    static unsigned st0244 [] =
    {
        0x0102, 
    };

    static unsigned long sh0246 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static unsigned st0246 [] =
    {
        0x0103, 
    };

    static unsigned long sh0247 [] =
    {
        0x00000001, 0x00000029, 
    };

    static unsigned st0247 [] =
    {
        0x0104, 
    };

    static unsigned long sh0248 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0248 [] =
    {
        0x0101, 
    };

    static unsigned long sh0255 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static unsigned st0255 [] =
    {
        0x0106, 
    };

    static unsigned long sh0256 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static unsigned st0256 [] =
    {
        0x0107, 
    };

    static unsigned long sh0259 [] =
    {
        0x00000003, 0x0000003c, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0259 [] =
    {
        0x00f1, 0x00f3, 0x00f2, 
    };

    static unsigned long sh0261 [] =
    {
        0x00000001, 0x00000025, 
    };

    static unsigned st0261 [] =
    {
        0x0109, 
    };

    static unsigned long sh0262 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0262 [] =
    {
        0x010a, 
    };

    static unsigned long sh0263 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0263 [] =
    {
        0x010a, 
    };

    static unsigned long sh0264 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static unsigned st0264 [] =
    {
        0x010b, 
    };

    static unsigned long sh0267 [] =
    {
        0x00000004, 0x0000003c, 0x00000106, 0x00000107, 0x7fff010e, 
    };

    static unsigned st0267 [] =
    {
        0x00f1, 0x010a, 0x00f3, 0x00f2, 
    };

    static unsigned long sh0268 [] =
    {
        0x00000002, 0x00000029, 0x0000002c, 
    };

    static unsigned st0268 [] =
    {
        0x010e, 0x010f, 
    };

    static unsigned long sh0269 [] =
    {
        0x00000001, 0x00000029, 
    };

    static unsigned st0269 [] =
    {
        0x0110, 
    };

    static unsigned long sh0271 [] =
    {
        0x00000001, 0x00000106, 
    };

    static unsigned st0271 [] =
    {
        0x010a, 
    };

    static unsigned long sh0273 [] =
    {
        0x00000001, 0x00000029, 
    };

    static unsigned st0273 [] =
    {
        0x0112, 
    };

    /* Shifts Table */
    static unsigned long *shtbl [] =
    {
        ppNULL, ppNULL, sh0002, sh0003, sh0004, sh0005, ppNULL, ppNULL, 
        sh0008, sh0009, sh0010, sh0011, sh0012, sh0013, sh0014, ppNULL, 
        sh0016, sh0017, sh0018, ppNULL, ppNULL, ppNULL, ppNULL, sh0023, 
        ppNULL, ppNULL, ppNULL, sh0027, sh0028, sh0029, sh0030, ppNULL, 
        ppNULL, ppNULL, sh0034, ppNULL, sh0036, ppNULL, ppNULL, sh0039, 
        sh0040, sh0041, ppNULL, ppNULL, ppNULL, sh0045, ppNULL, sh0047, 
        ppNULL, ppNULL, sh0050, sh0051, sh0052, sh0053, sh0054, sh0055, 
        sh0056, sh0057, sh0058, sh0059, sh0060, sh0061, sh0062, sh0063, 
        sh0064, sh0065, sh0066, sh0067, sh0068, sh0069, sh0070, sh0071, 
        sh0072, sh0073, ppNULL, sh0075, sh0076, sh0077, sh0078, sh0079, 
        sh0080, sh0081, sh0082, sh0083, sh0084, ppNULL, ppNULL, ppNULL, 
        ppNULL, sh0089, sh0090, sh0091, sh0092, sh0093, sh0094, sh0095, 
        sh0096, sh0097, sh0098, sh0099, sh0100, sh0101, sh0102, sh0103, 
        sh0104, sh0105, sh0106, sh0107, sh0108, sh0109, sh0110, sh0111, 
        sh0112, sh0113, sh0114, sh0115, sh0116, sh0117, sh0118, sh0119, 
        sh0120, sh0121, sh0122, sh0123, sh0124, ppNULL, sh0126, sh0127, 
        sh0128, sh0129, sh0130, sh0131, sh0132, sh0133, sh0134, sh0135, 
        sh0136, sh0137, sh0138, sh0139, sh0140, sh0141, sh0142, sh0143, 
        sh0144, sh0145, sh0146, sh0147, sh0148, sh0149, ppNULL, sh0151, 
        sh0152, sh0153, sh0154, sh0155, sh0156, sh0157, sh0158, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, sh0164, sh0165, ppNULL, ppNULL, 
        sh0168, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, sh0175, 
        ppNULL, sh0177, ppNULL, ppNULL, ppNULL, sh0181, sh0182, ppNULL, 
        ppNULL, sh0185, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, sh0210, sh0211, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, sh0218, sh0219, sh0220, ppNULL, sh0222, sh0223, 
        sh0224, sh0225, sh0226, sh0227, ppNULL, sh0229, sh0230, sh0231, 
        sh0232, ppNULL, sh0234, sh0235, sh0236, ppNULL, sh0238, ppNULL, 
        sh0240, sh0241, ppNULL, ppNULL, sh0244, ppNULL, sh0246, sh0247, 
        sh0248, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, sh0255, 
        sh0256, ppNULL, ppNULL, sh0259, ppNULL, sh0261, sh0262, sh0263, 
        sh0264, ppNULL, ppNULL, sh0267, sh0268, sh0269, ppNULL, sh0271, 
        ppNULL, sh0273, ppNULL, 
    };

    /* Shifts Targets Table */
    static unsigned *sttbl [] =
    {
        ppNULL, ppNULL, st0002, st0003, st0004, st0005, ppNULL, ppNULL, 
        st0008, st0009, st0010, st0011, st0012, st0013, st0014, ppNULL, 
        st0016, st0017, st0018, ppNULL, ppNULL, ppNULL, ppNULL, st0023, 
        ppNULL, ppNULL, ppNULL, st0027, st0028, st0029, st0030, ppNULL, 
        ppNULL, ppNULL, st0034, ppNULL, st0036, ppNULL, ppNULL, st0039, 
        st0040, st0041, ppNULL, ppNULL, ppNULL, st0045, ppNULL, st0047, 
        ppNULL, ppNULL, st0050, st0051, st0052, st0053, st0054, st0055, 
        st0056, st0057, st0058, st0059, st0060, st0061, st0062, st0063, 
        st0064, st0065, st0066, st0067, st0068, st0069, st0070, st0071, 
        st0072, st0073, ppNULL, st0075, st0076, st0077, st0078, st0079, 
        st0080, st0081, st0082, st0083, st0084, ppNULL, ppNULL, ppNULL, 
        ppNULL, st0089, st0090, st0091, st0092, st0093, st0094, st0095, 
        st0096, st0097, st0098, st0099, st0100, st0101, st0102, st0103, 
        st0104, st0105, st0106, st0107, st0108, st0109, st0110, st0111, 
        st0112, st0113, st0114, st0115, st0116, st0117, st0118, st0119, 
        st0120, st0121, st0122, st0123, st0124, ppNULL, st0126, st0127, 
        st0128, st0129, st0130, st0131, st0132, st0133, st0134, st0135, 
        st0136, st0137, st0138, st0139, st0140, st0141, st0142, st0143, 
        st0144, st0145, st0146, st0147, st0148, st0149, ppNULL, st0151, 
        st0152, st0153, st0154, st0155, st0156, st0157, st0158, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, st0164, st0165, ppNULL, ppNULL, 
        st0168, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, st0175, 
        ppNULL, st0177, ppNULL, ppNULL, ppNULL, st0181, st0182, ppNULL, 
        ppNULL, st0185, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, st0210, st0211, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, st0218, st0219, st0220, ppNULL, st0222, st0223, 
        st0224, st0225, st0226, st0227, ppNULL, st0229, st0230, st0231, 
        st0232, ppNULL, st0234, st0235, st0236, ppNULL, st0238, ppNULL, 
        st0240, st0241, ppNULL, ppNULL, st0244, ppNULL, st0246, st0247, 
        st0248, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, st0255, 
        st0256, ppNULL, ppNULL, st0259, ppNULL, st0261, st0262, st0263, 
        st0264, ppNULL, ppNULL, st0267, st0268, st0269, ppNULL, st0271, 
        ppNULL, st0273, ppNULL, 
    };

    static unsigned long rd0000 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0000 [] =
    {
        0x0008, 0x0004, 0x0000, 
    };

    static unsigned long rd0001 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0001 [] =
    {
        0x0000, 0x0000, 0x0001, 
    };

    static unsigned long rd0006 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0006 [] =
    {
        0x0003, 0x0002, 0x0000, 
    };

    static unsigned long rd0007 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0007 [] =
    {
        0x0007, 0x0004, 0x0003, 
    };

    static unsigned long rd0013 [] =
    {
        0x00000001, 0x00000101, 
    };

    static unsigned rp0013 [] =
    {
        0x0001, 0x0001, 0x0004, 
    };

    static unsigned long rd0015 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0015 [] =
    {
        0x0061, 0x0017, 0x0001, 
    };

    static unsigned long rd0019 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0019 [] =
    {
        0x000c, 0x0005, 0x0003, 
    };

    static unsigned long rd0020 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0020 [] =
    {
        0x0012, 0x0008, 0x0000, 
    };

    static unsigned long rd0021 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0021 [] =
    {
        0x0013, 0x0009, 0x0001, 
    };

    static unsigned long rd0022 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0022 [] =
    {
        0x0014, 0x0009, 0x0001, 
    };

    static unsigned long rd0024 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0024 [] =
    {
        0x0016, 0x000a, 0x0000, 
    };

    static unsigned long rd0025 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0025 [] =
    {
        0x0005, 0x0003, 0x0001, 
    };

    static unsigned long rd0026 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0026 [] =
    {
        0x0006, 0x0003, 0x0001, 
    };

    static unsigned long rd0027 [] =
    {
        0x00000003, 0x0000003b, 0x0000007d, 0x7fff010c, 
    };

    static unsigned rp0027 [] =
    {
        0x001d, 0x000d, 0x0000, 0x001d, 0x000d, 0x0000, 0x001d, 0x000d, 
        0x0000, 
    };

    static unsigned long rd0029 [] =
    {
        0x00000005, 0x0000003a, 0x0000003b, 0x0000007b, 0x0000007d, 
        0x7fff010c, 
    };

    static unsigned rp0029 [] =
    {
        0x001a, 0x000c, 0x0001, 0x001a, 0x000c, 0x0001, 0x001a, 0x000c, 
        0x0001, 0x001a, 0x000c, 0x0001, 0x001a, 0x000c, 0x0001, 
    };

    static unsigned long rd0031 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0031 [] =
    {
        0x0009, 0x0005, 0x0004, 
    };

    static unsigned long rd0032 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0032 [] =
    {
        0x000a, 0x0005, 0x0004, 
    };

    static unsigned long rd0033 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0033 [] =
    {
        0x000b, 0x0005, 0x0004, 
    };

    static unsigned long rd0034 [] =
    {
        0x00000006, 0x00000029, 0x0000002c, 0x0000003b, 0x0000003c, 
        0x00000107, 0x7fff010e, 
    };

    static unsigned rp0034 [] =
    {
        0x000f, 0x0007, 0x0002, 0x000f, 0x0007, 0x0002, 0x000f, 0x0007, 
        0x0002, 0x000f, 0x0007, 0x0002, 0x000f, 0x0007, 0x0002, 0x000f, 
        0x0007, 0x0002, 
    };

    static unsigned long rd0035 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0035 [] =
    {
        0x0002, 0x0002, 0x0003, 
    };

    static unsigned long rd0036 [] =
    {
        0x00000001, 0x0000003b, 
    };

    static unsigned rp0036 [] =
    {
        0x0004, 0x0003, 0x0002, 
    };

    static unsigned long rd0037 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0037 [] =
    {
        0x0017, 0x000b, 0x0002, 
    };

    static unsigned long rd0038 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0038 [] =
    {
        0x0016, 0x000a, 0x0000, 
    };

    static unsigned long rd0042 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0042 [] =
    {
        0x0010, 0x0008, 0x0002, 
    };

    static unsigned long rd0043 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0043 [] =
    {
        0x0011, 0x0008, 0x0002, 
    };

    static unsigned long rd0044 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0044 [] =
    {
        0x0015, 0x000a, 0x0002, 
    };

    static unsigned long rd0046 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0046 [] =
    {
        0x001c, 0x000d, 0x0002, 
    };

    static unsigned long rd0048 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0048 [] =
    {
        0x0019, 0x000c, 0x0003, 
    };

    static unsigned long rd0049 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0049 [] =
    {
        0x001f, 0x000e, 0x0001, 
    };

    static unsigned long rd0074 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0074 [] =
    {
        0x003b, 0x000f, 0x0001, 
    };

    static unsigned long rd0083 [] =
    {
        0x00000001, 0x0000003b, 
    };

    static unsigned rp0083 [] =
    {
        0x000d, 0x0006, 0x0006, 
    };

    static unsigned long rd0085 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0085 [] =
    {
        0x005f, 0x0016, 0x0001, 
    };

    static unsigned long rd0086 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0086 [] =
    {
        0x0060, 0x0016, 0x0001, 
    };

    static unsigned long rd0087 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0087 [] =
    {
        0x001b, 0x000d, 0x0003, 
    };

    static unsigned long rd0088 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0088 [] =
    {
        0x0018, 0x000c, 0x0004, 
    };

    static unsigned long rd0125 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0125 [] =
    {
        0x001e, 0x000e, 0x0003, 
    };

    static unsigned long rd0150 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0150 [] =
    {
        0x003b, 0x000f, 0x0001, 
    };

    static unsigned long rd0159 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0159 [] =
    {
        0x0020, 0x000f, 0x0003, 
    };

    static unsigned long rd0160 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0160 [] =
    {
        0x002e, 0x000f, 0x0003, 
    };

    static unsigned long rd0161 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0161 [] =
    {
        0x0021, 0x000f, 0x0003, 
    };

    static unsigned long rd0162 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0162 [] =
    {
        0x0022, 0x000f, 0x0003, 
    };

    static unsigned long rd0163 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0163 [] =
    {
        0x0023, 0x000f, 0x0003, 
    };

    static unsigned long rd0166 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0166 [] =
    {
        0x005c, 0x0014, 0x0001, 
    };

    static unsigned long rd0167 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0167 [] =
    {
        0x0024, 0x000f, 0x0003, 
    };

    static unsigned long rd0168 [] =
    {
        0x00000002, 0x00000029, 0x0000002c, 
    };

    static unsigned rp0168 [] =
    {
        0x004a, 0x0010, 0x0001, 0x004a, 0x0010, 0x0001, 
    };

    static unsigned long rd0169 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0169 [] =
    {
        0x004b, 0x0010, 0x0001, 
    };

    static unsigned long rd0170 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0170 [] =
    {
        0x004c, 0x0010, 0x0001, 
    };

    static unsigned long rd0171 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0171 [] =
    {
        0x004d, 0x0010, 0x0001, 
    };

    static unsigned long rd0172 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0172 [] =
    {
        0x0025, 0x000f, 0x0003, 
    };

    static unsigned long rd0173 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0173 [] =
    {
        0x0026, 0x000f, 0x0003, 
    };

    static unsigned long rd0174 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0174 [] =
    {
        0x0050, 0x0012, 0x0001, 
    };

    static unsigned long rd0175 [] =
    {
        0x00000002, 0x00000029, 0x0000002c, 
    };

    static unsigned rp0175 [] =
    {
        0x0051, 0x0012, 0x0001, 0x0051, 0x0012, 0x0001, 
    };

    static unsigned long rd0176 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0176 [] =
    {
        0x0052, 0x0012, 0x0001, 
    };

    static unsigned long rd0178 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0178 [] =
    {
        0x0054, 0x0012, 0x0001, 
    };

    static unsigned long rd0179 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0179 [] =
    {
        0x0027, 0x000f, 0x0003, 
    };

    static unsigned long rd0180 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0180 [] =
    {
        0x0028, 0x000f, 0x0003, 
    };

    static unsigned long rd0183 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0183 [] =
    {
        0x0059, 0x0013, 0x0001, 
    };

    static unsigned long rd0184 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0184 [] =
    {
        0x0029, 0x000f, 0x0003, 
    };

    static unsigned long rd0186 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0186 [] =
    {
        0x005f, 0x0016, 0x0001, 
    };

    static unsigned long rd0187 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0187 [] =
    {
        0x0060, 0x0016, 0x0001, 
    };

    static unsigned long rd0188 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0188 [] =
    {
        0x002a, 0x000f, 0x0003, 
    };

    static unsigned long rd0189 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0189 [] =
    {
        0x002b, 0x000f, 0x0003, 
    };

    static unsigned long rd0190 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0190 [] =
    {
        0x002c, 0x000f, 0x0003, 
    };

    static unsigned long rd0191 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0191 [] =
    {
        0x002d, 0x000f, 0x0003, 
    };

    static unsigned long rd0192 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0192 [] =
    {
        0x002f, 0x000f, 0x0003, 
    };

    static unsigned long rd0193 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0193 [] =
    {
        0x0030, 0x000f, 0x0003, 
    };

    static unsigned long rd0194 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0194 [] =
    {
        0x0031, 0x000f, 0x0003, 
    };

    static unsigned long rd0195 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0195 [] =
    {
        0x0032, 0x000f, 0x0003, 
    };

    static unsigned long rd0196 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0196 [] =
    {
        0x0033, 0x000f, 0x0003, 
    };

    static unsigned long rd0197 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0197 [] =
    {
        0x0034, 0x000f, 0x0003, 
    };

    static unsigned long rd0198 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0198 [] =
    {
        0x0035, 0x000f, 0x0003, 
    };

    static unsigned long rd0199 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0199 [] =
    {
        0x0036, 0x000f, 0x0003, 
    };

    static unsigned long rd0200 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0200 [] =
    {
        0x0037, 0x000f, 0x0003, 
    };

    static unsigned long rd0201 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0201 [] =
    {
        0x0062, 0x0018, 0x0001, 
    };

    static unsigned long rd0202 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0202 [] =
    {
        0x0063, 0x0018, 0x0001, 
    };

    static unsigned long rd0203 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0203 [] =
    {
        0x0064, 0x0018, 0x0001, 
    };

    static unsigned long rd0204 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0204 [] =
    {
        0x0065, 0x0018, 0x0001, 
    };

    static unsigned long rd0205 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0205 [] =
    {
        0x0066, 0x0018, 0x0001, 
    };

    static unsigned long rd0206 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0206 [] =
    {
        0x0067, 0x0018, 0x0001, 
    };

    static unsigned long rd0207 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0207 [] =
    {
        0x0068, 0x0018, 0x0001, 
    };

    static unsigned long rd0208 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0208 [] =
    {
        0x0069, 0x0018, 0x0001, 
    };

    static unsigned long rd0209 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0209 [] =
    {
        0x0038, 0x000f, 0x0003, 
    };

    static unsigned long rd0212 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0212 [] =
    {
        0x003a, 0x000f, 0x0003, 
    };

    static unsigned long rd0213 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0213 [] =
    {
        0x003c, 0x000f, 0x0003, 
    };

    static unsigned long rd0214 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0214 [] =
    {
        0x003d, 0x000f, 0x0003, 
    };

    static unsigned long rd0215 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0215 [] =
    {
        0x003e, 0x000f, 0x0003, 
    };

    static unsigned long rd0216 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0216 [] =
    {
        0x003f, 0x000f, 0x0003, 
    };

    static unsigned long rd0217 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0217 [] =
    {
        0x0040, 0x000f, 0x0003, 
    };

    static unsigned long rd0218 [] =
    {
        0x00000002, 0x00000029, 0x0000002c, 
    };

    static unsigned rp0218 [] =
    {
        0x0044, 0x000f, 0x0003, 0x0044, 0x000f, 0x0003, 
    };

    static unsigned long rd0219 [] =
    {
        0x00000002, 0x00000029, 0x0000002c, 
    };

    static unsigned rp0219 [] =
    {
        0x0046, 0x000f, 0x0003, 0x0046, 0x000f, 0x0003, 
    };

    static unsigned long rd0220 [] =
    {
        0x00000002, 0x00000029, 0x0000002c, 
    };

    static unsigned rp0220 [] =
    {
        0x0048, 0x000f, 0x0003, 0x0048, 0x000f, 0x0003, 
    };

    static unsigned long rd0221 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0221 [] =
    {
        0x0049, 0x000f, 0x0003, 
    };

    static unsigned long rd0227 [] =
    {
        0x00000002, 0x00000029, 0x0000002c, 
    };

    static unsigned rp0227 [] =
    {
        0x004e, 0x0011, 0x0002, 0x004e, 0x0011, 0x0002, 
    };

    static unsigned long rd0228 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0228 [] =
    {
        0x0053, 0x0012, 0x0002, 
    };

    static unsigned long rd0231 [] =
    {
        0x00000002, 0x00000029, 0x0000002c, 
    };

    static unsigned rp0231 [] =
    {
        0x0039, 0x000f, 0x0004, 0x0039, 0x000f, 0x0004, 
    };

    static unsigned long rd0233 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0233 [] =
    {
        0x0061, 0x0017, 0x0001, 
    };

    static unsigned long rd0237 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0237 [] =
    {
        0x000e, 0x0006, 0x0009, 
    };

    static unsigned long rd0239 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0239 [] =
    {
        0x005d, 0x0015, 0x0003, 
    };

    static unsigned long rd0242 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0242 [] =
    {
        0x005f, 0x0016, 0x0001, 
    };

    static unsigned long rd0243 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0243 [] =
    {
        0x0060, 0x0016, 0x0001, 
    };

    static unsigned long rd0245 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0245 [] =
    {
        0x004f, 0x0011, 0x0003, 
    };

    static unsigned long rd0249 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0249 [] =
    {
        0x0042, 0x000f, 0x0005, 
    };

    static unsigned long rd0250 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0250 [] =
    {
        0x0043, 0x000f, 0x0005, 
    };

    static unsigned long rd0251 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0251 [] =
    {
        0x0045, 0x000f, 0x0005, 
    };

    static unsigned long rd0252 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0252 [] =
    {
        0x0047, 0x000f, 0x0005, 
    };

    static unsigned long rd0253 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0253 [] =
    {
        0x005e, 0x0016, 0x0005, 
    };

    static unsigned long rd0254 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0254 [] =
    {
        0x005a, 0x0014, 0x0004, 
    };

    static unsigned long rd0257 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0257 [] =
    {
        0x0061, 0x0017, 0x0001, 
    };

    static unsigned long rd0258 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0258 [] =
    {
        0x005b, 0x0014, 0x0004, 
    };

    static unsigned long rd0260 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0260 [] =
    {
        0x0058, 0x0013, 0x0004, 
    };

    static unsigned long rd0265 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0265 [] =
    {
        0x0041, 0x000f, 0x0007, 
    };

    static unsigned long rd0266 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0266 [] =
    {
        0x0061, 0x0017, 0x0001, 
    };

    static unsigned long rd0270 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0270 [] =
    {
        0x0055, 0x0013, 0x0008, 
    };

    static unsigned long rd0272 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0272 [] =
    {
        0x0057, 0x0013, 0x0008, 
    };

    static unsigned long rd0274 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0274 [] =
    {
        0x0056, 0x0013, 0x000a, 
    };

    /* Reductions Table */
    static unsigned long *rdtbl [] =
    {
        rd0000, rd0001, ppNULL, ppNULL, ppNULL, ppNULL, rd0006, rd0007, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rd0013, ppNULL, rd0015, 
        ppNULL, ppNULL, ppNULL, rd0019, rd0020, rd0021, rd0022, ppNULL, 
        rd0024, rd0025, rd0026, rd0027, ppNULL, rd0029, ppNULL, rd0031, 
        rd0032, rd0033, rd0034, rd0035, rd0036, rd0037, rd0038, ppNULL, 
        ppNULL, ppNULL, rd0042, rd0043, rd0044, ppNULL, rd0046, ppNULL, 
        rd0048, rd0049, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, rd0074, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, rd0083, ppNULL, rd0085, rd0086, rd0087, 
        rd0088, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rd0125, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rd0150, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rd0159, 
        rd0160, rd0161, rd0162, rd0163, ppNULL, ppNULL, rd0166, rd0167, 
        rd0168, rd0169, rd0170, rd0171, rd0172, rd0173, rd0174, rd0175, 
        rd0176, ppNULL, rd0178, rd0179, rd0180, ppNULL, ppNULL, rd0183, 
        rd0184, ppNULL, rd0186, rd0187, rd0188, rd0189, rd0190, rd0191, 
        rd0192, rd0193, rd0194, rd0195, rd0196, rd0197, rd0198, rd0199, 
        rd0200, rd0201, rd0202, rd0203, rd0204, rd0205, rd0206, rd0207, 
        rd0208, rd0209, ppNULL, ppNULL, rd0212, rd0213, rd0214, rd0215, 
        rd0216, rd0217, rd0218, rd0219, rd0220, rd0221, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, rd0227, rd0228, ppNULL, ppNULL, rd0231, 
        ppNULL, rd0233, ppNULL, ppNULL, ppNULL, rd0237, ppNULL, rd0239, 
        ppNULL, ppNULL, rd0242, rd0243, ppNULL, rd0245, ppNULL, ppNULL, 
        ppNULL, rd0249, rd0250, rd0251, rd0252, rd0253, rd0254, ppNULL, 
        ppNULL, rd0257, rd0258, ppNULL, rd0260, ppNULL, ppNULL, ppNULL, 
        ppNULL, rd0265, rd0266, ppNULL, ppNULL, ppNULL, rd0270, ppNULL, 
        rd0272, ppNULL, rd0274, 
    };

    /* Reductions Parameters Table */
    static unsigned *rptbl [] =
    {
        rp0000, rp0001, ppNULL, ppNULL, ppNULL, ppNULL, rp0006, rp0007, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rp0013, ppNULL, rp0015, 
        ppNULL, ppNULL, ppNULL, rp0019, rp0020, rp0021, rp0022, ppNULL, 
        rp0024, rp0025, rp0026, rp0027, ppNULL, rp0029, ppNULL, rp0031, 
        rp0032, rp0033, rp0034, rp0035, rp0036, rp0037, rp0038, ppNULL, 
        ppNULL, ppNULL, rp0042, rp0043, rp0044, ppNULL, rp0046, ppNULL, 
        rp0048, rp0049, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, rp0074, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, rp0083, ppNULL, rp0085, rp0086, rp0087, 
        rp0088, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rp0125, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rp0150, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rp0159, 
        rp0160, rp0161, rp0162, rp0163, ppNULL, ppNULL, rp0166, rp0167, 
        rp0168, rp0169, rp0170, rp0171, rp0172, rp0173, rp0174, rp0175, 
        rp0176, ppNULL, rp0178, rp0179, rp0180, ppNULL, ppNULL, rp0183, 
        rp0184, ppNULL, rp0186, rp0187, rp0188, rp0189, rp0190, rp0191, 
        rp0192, rp0193, rp0194, rp0195, rp0196, rp0197, rp0198, rp0199, 
        rp0200, rp0201, rp0202, rp0203, rp0204, rp0205, rp0206, rp0207, 
        rp0208, rp0209, ppNULL, ppNULL, rp0212, rp0213, rp0214, rp0215, 
        rp0216, rp0217, rp0218, rp0219, rp0220, rp0221, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, rp0227, rp0228, ppNULL, ppNULL, rp0231, 
        ppNULL, rp0233, ppNULL, ppNULL, ppNULL, rp0237, ppNULL, rp0239, 
        ppNULL, ppNULL, rp0242, rp0243, ppNULL, rp0245, ppNULL, ppNULL, 
        ppNULL, rp0249, rp0250, rp0251, rp0252, rp0253, rp0254, ppNULL, 
        ppNULL, rp0257, rp0258, ppNULL, rp0260, ppNULL, ppNULL, ppNULL, 
        ppNULL, rp0265, rp0266, ppNULL, ppNULL, ppNULL, rp0270, ppNULL, 
        rp0272, ppNULL, rp0274, 
    };

    /* Nonterminals Table */
    static unsigned nttbl [] =
    {
        0x0000, 0x0001, 0x0002, 0x0004, 0x0007, 0x0009, 0x000d, 0x000f, 
        0x0010, 0x0013, 0x0015, 0x0017, 0x0018, 0x001b, 0x001e, 0x0020, 
        0x004a, 0x004e, 0x0050, 0x0055, 0x005a, 0x005d, 0x005e, 0x0061, 
        0x0062, 
    };

    void modifier (unsigned, int, char *[], unsigned []);
    void surface_def (unsigned, int, char *[], unsigned []);
    void append_string (unsigned, int, char *[], unsigned []);
    void set_string (unsigned, int, char *[], unsigned []);
    void destroy_object (unsigned, int, char *[], unsigned []);
    void make_object (unsigned, int, char *[], unsigned []);
    void set_attribute (unsigned, int, char *[], unsigned []);
    void coord_value (unsigned, int, char *[], unsigned []);
    void set_percentile (unsigned, int, char *[], unsigned []);
    void dim_value (unsigned, int, char *[], unsigned []);
    void bg_value (unsigned, int, char *[], unsigned []);
    void border_value (unsigned, int, char *[], unsigned []);
    void color_spec (unsigned, int, char *[], unsigned []);
    void push_expr (unsigned, int, char *[], unsigned []);
    void set_direction (unsigned, int, char *[], unsigned []);

    static void (*fctbl []) (unsigned, int, char *[], unsigned []) =
    {
        NULL, NULL, NULL, NULL, NULL, &modifier, &surface_def, NULL, 
        &append_string, &set_string, NULL, &destroy_object, &make_object, 
        NULL, NULL, &set_attribute, &coord_value, &set_percentile, 
        &dim_value, &bg_value, &border_value, NULL, &color_spec, &push_expr, 
        &set_direction, 
    };


    static unsigned long cbtbl [] =
    {
        0x00000001, 0x00000001, 0x00010004, 0x00000000, 0x00000000, 
        0x000d0006, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00020003, 0x00000000, 
        0x00610001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x000f0002, 0x00130001, 0x00140001, 0x00000000, 0x00040002, 
        0x00050001, 0x00060001, 0x00170002, 0x00090004, 0x00180004, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00100002, 
        0x00000000, 0x00150002, 0x00000000, 0x001b0003, 0x001c0002, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00150002, 0x00000000, 0x001e0003, 0x00000000, 0x001f0001, 
        0x00200003, 0x00210003, 0x00220003, 0x00230003, 0x00240003, 
        0x00250003, 0x00260003, 0x00270003, 0x00280003, 0x00290003, 
        0x002a0003, 0x002b0003, 0x002c0003, 0x002d0003, 0x002f0003, 
        0x00310003, 0x00320003, 0x00330003, 0x00340003, 0x00350003, 
        0x00360003, 0x00370003, 0x00380003, 0x003a0003, 0x003b0001, 
        0x003c0003, 0x003e0003, 0x003f0003, 0x00400003, 0x00430005, 
        0x00450005, 0x00470005, 0x00490003, 0x00000000, 0x005e0005, 
        0x005f0001, 0x00600001, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x005d0003, 
        0x00000000, 0x00200003, 0x00210003, 0x00220003, 0x00230003, 
        0x00240003, 0x00250003, 0x00260003, 0x00270003, 0x00280003, 
        0x00290003, 0x002a0003, 0x002b0003, 0x002c0003, 0x002d0003, 
        0x002f0003, 0x00310003, 0x00320003, 0x00330003, 0x00340003, 
        0x00350003, 0x00360003, 0x00370003, 0x00380003, 0x003a0003, 
        0x003b0001, 0x003c0003, 0x003e0003, 0x003f0003, 0x00400003, 
        0x00430005, 0x00450005, 0x00470005, 0x00490003, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x005a0004, 
        0x005b0004, 0x005c0001, 0x00000000, 0x004a0001, 0x004b0001, 
        0x004c0001, 0x004d0001, 0x00000000, 0x00000000, 0x00500001, 
        0x00510001, 0x00520001, 0x00530002, 0x00540001, 0x00000000, 
        0x00000000, 0x00550008, 0x00580004, 0x00590001, 0x00000000, 
        0x005e0005, 0x005f0001, 0x00600001, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00620001, 0x00630001, 0x00640001, 0x00650001, 
        0x00660001, 0x00670001, 0x00680001, 0x00690001, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00610001, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x005e0005, 0x005f0001, 0x00600001, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x005d0003, 0x00610001, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00610001, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
    };

    static char *mstbl [] =
    {
        "**** syntax error ***", 
    };

    static int catcher (unsigned r, unsigned e, unsigned st)
    {
        __catcher (r, e, st);

        return 0;
    }

