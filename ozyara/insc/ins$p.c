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
        0x0003, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0004, 
    };

    static unsigned gt0001 [] =
    {
        0x0002, 0x0002, 0x0006, 0x0003, 0x0004, 
    };

    static unsigned gt0003 [] =
    {
        0x0002, 0x0003, 0x0008, 0x0004, 0x0007, 
    };

    static unsigned gt0007 [] =
    {
        0x0001, 0x0003, 0x000b, 
    };

    static unsigned gt0009 [] =
    {
        0x0005, 0x0005, 0x000c, 0x0006, 0x000d, 0x0007, 0x000e, 0x0008, 
        0x000f, 0x0009, 0x0011, 
    };

    static unsigned gt0012 [] =
    {
        0x0004, 0x0006, 0x001d, 0x0007, 0x000e, 0x0008, 0x000f, 0x0009, 
        0x0011, 
    };

    static unsigned gt0014 [] =
    {
        0x0002, 0x000a, 0x001e, 0x000b, 0x0020, 
    };

    static unsigned gt0027 [] =
    {
        0x0005, 0x0005, 0x0024, 0x0006, 0x000d, 0x0007, 0x000e, 0x0008, 
        0x000f, 0x0009, 0x0011, 
    };

    static unsigned gt0030 [] =
    {
        0x0001, 0x000b, 0x0025, 
    };

    static unsigned gt0031 [] =
    {
        0x0002, 0x0008, 0x0028, 0x0009, 0x0011, 
    };

    static unsigned gt0036 [] =
    {
        0x0004, 0x0006, 0x001d, 0x0007, 0x000e, 0x0008, 0x000f, 0x0009, 
        0x0011, 
    };

    /* Transitions Table */
    static unsigned *gttbl [] =
    {
        gt0000, gt0001, ppNULL, gt0003, ppNULL, ppNULL, ppNULL, gt0007, 
        ppNULL, gt0009, ppNULL, ppNULL, gt0012, ppNULL, gt0014, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, gt0027, ppNULL, ppNULL, gt0030, gt0031, 
        ppNULL, ppNULL, ppNULL, ppNULL, gt0036, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, 
    };

    static unsigned long sh0000 [] =
    {
        0x00000002, 0x00000028, 0x00000108, 
    };

    static unsigned st0000 [] =
    {
        0x0003, 0x0005, 
    };

    static unsigned long sh0001 [] =
    {
        0x00000002, 0x00000028, 0x00000108, 
    };

    static unsigned st0001 [] =
    {
        0x0003, 0x0005, 
    };

    static unsigned long sh0003 [] =
    {
        0x00000001, 0x00000108, 
    };

    static unsigned st0003 [] =
    {
        0x0005, 
    };

    static unsigned long sh0004 [] =
    {
        0x00000001, 0x0000007b, 
    };

    static unsigned st0004 [] =
    {
        0x0009, 
    };

    static unsigned long sh0007 [] =
    {
        0x00000002, 0x00000029, 0x00000108, 
    };

    static unsigned st0007 [] =
    {
        0x000a, 0x0005, 
    };

    static unsigned long sh0009 [] =
    {
        0x0000000a, 0x00000107, 0x7fff010a, 0x7fff010b, 0x7fff010c, 
        0x7fff010d, 0x7fff010e, 0x7fff010f, 0x7fff0110, 0x7fff0111, 
        0x7fff0112, 
    };

    static unsigned st0009 [] =
    {
        0x0012, 0x0010, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 
        0x0019, 0x001a, 
    };

    static unsigned long sh0010 [] =
    {
        0x00000001, 0x0000007b, 
    };

    static unsigned st0010 [] =
    {
        0x001b, 
    };

    static unsigned long sh0012 [] =
    {
        0x0000000b, 0x0000007d, 0x00000107, 0x7fff010a, 0x7fff010b, 
        0x7fff010c, 0x7fff010d, 0x7fff010e, 0x7fff010f, 0x7fff0110, 
        0x7fff0111, 0x7fff0112, 
    };

    static unsigned st0012 [] =
    {
        0x001c, 0x0012, 0x0010, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
        0x0018, 0x0019, 0x001a, 
    };

    static unsigned long sh0014 [] =
    {
        0x00000003, 0x0000002c, 0x00000106, 0x7fff0109, 
    };

    static unsigned st0014 [] =
    {
        0x001f, 0x0022, 0x0021, 
    };

    static unsigned long sh0016 [] =
    {
        0x00000001, 0x0000002a, 
    };

    static unsigned st0016 [] =
    {
        0x0023, 
    };

    static unsigned long sh0027 [] =
    {
        0x0000000a, 0x00000107, 0x7fff010a, 0x7fff010b, 0x7fff010c, 
        0x7fff010d, 0x7fff010e, 0x7fff010f, 0x7fff0110, 0x7fff0111, 
        0x7fff0112, 
    };

    static unsigned st0027 [] =
    {
        0x0012, 0x0010, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 
        0x0019, 0x001a, 
    };

    static unsigned long sh0030 [] =
    {
        0x00000002, 0x00000106, 0x7fff0109, 
    };

    static unsigned st0030 [] =
    {
        0x0027, 0x0026, 
    };

    static unsigned long sh0031 [] =
    {
        0x0000000a, 0x00000107, 0x7fff010a, 0x7fff010b, 0x7fff010c, 
        0x7fff010d, 0x7fff010e, 0x7fff010f, 0x7fff0110, 0x7fff0111, 
        0x7fff0112, 
    };

    static unsigned st0031 [] =
    {
        0x0012, 0x0010, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 
        0x0019, 0x0029, 
    };

    static unsigned long sh0036 [] =
    {
        0x0000000b, 0x0000007d, 0x00000107, 0x7fff010a, 0x7fff010b, 
        0x7fff010c, 0x7fff010d, 0x7fff010e, 0x7fff010f, 0x7fff0110, 
        0x7fff0111, 0x7fff0112, 
    };

    static unsigned st0036 [] =
    {
        0x002a, 0x0012, 0x0010, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
        0x0018, 0x0019, 0x0029, 
    };

    /* Shifts Table */
    static unsigned long *shtbl [] =
    {
        sh0000, sh0001, ppNULL, sh0003, sh0004, ppNULL, ppNULL, sh0007, 
        ppNULL, sh0009, sh0010, ppNULL, sh0012, ppNULL, sh0014, ppNULL, 
        sh0016, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, sh0027, ppNULL, ppNULL, sh0030, sh0031, 
        ppNULL, ppNULL, ppNULL, ppNULL, sh0036, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, 
    };

    /* Shifts Targets Table */
    static unsigned *sttbl [] =
    {
        st0000, st0001, ppNULL, st0003, st0004, ppNULL, ppNULL, st0007, 
        ppNULL, st0009, st0010, ppNULL, st0012, ppNULL, st0014, ppNULL, 
        st0016, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, st0027, ppNULL, ppNULL, st0030, st0031, 
        ppNULL, ppNULL, ppNULL, ppNULL, st0036, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, 
    };

    static unsigned long rd0001 [] =
    {
        0x00000001, 0x00000101, 
    };

    static unsigned rp0001 [] =
    {
        0x0000, 0x0000, 0x0001, 
    };

    static unsigned long rd0002 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0002 [] =
    {
        0x0002, 0x0001, 0x0001, 
    };

    static unsigned long rd0005 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0005 [] =
    {
        0x0005, 0x0003, 0x0001, 
    };

    static unsigned long rd0006 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0006 [] =
    {
        0x0001, 0x0001, 0x0002, 
    };

    static unsigned long rd0008 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0008 [] =
    {
        0x0007, 0x0004, 0x0001, 
    };

    static unsigned long rd0011 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0011 [] =
    {
        0x0006, 0x0004, 0x0002, 
    };

    static unsigned long rd0013 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0013 [] =
    {
        0x0009, 0x0005, 0x0001, 
    };

    static unsigned long rd0015 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0015 [] =
    {
        0x000c, 0x0007, 0x0001, 
    };

    static unsigned long rd0016 [] =
    {
        0x00000003, 0x0000002c, 0x00000106, 0x7fff0109, 
    };

    static unsigned rp0016 [] =
    {
        0x000e, 0x0008, 0x0001, 0x000e, 0x0008, 0x0001, 0x000e, 0x0008, 
        0x0001, 
    };

    static unsigned long rd0017 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0017 [] =
    {
        0x000f, 0x0008, 0x0001, 
    };

    static unsigned long rd0018 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0018 [] =
    {
        0x0010, 0x0008, 0x0001, 
    };

    static unsigned long rd0019 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0019 [] =
    {
        0x0011, 0x0009, 0x0001, 
    };

    static unsigned long rd0020 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0020 [] =
    {
        0x0012, 0x0009, 0x0001, 
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

    static unsigned long rd0023 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0023 [] =
    {
        0x0015, 0x0009, 0x0001, 
    };

    static unsigned long rd0024 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0024 [] =
    {
        0x0016, 0x0009, 0x0001, 
    };

    static unsigned long rd0025 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0025 [] =
    {
        0x0017, 0x0009, 0x0001, 
    };

    static unsigned long rd0026 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0026 [] =
    {
        0x0018, 0x0009, 0x0001, 
    };

    static unsigned long rd0028 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0028 [] =
    {
        0x0004, 0x0002, 0x0004, 
    };

    static unsigned long rd0029 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0029 [] =
    {
        0x0008, 0x0005, 0x0002, 
    };

    static unsigned long rd0030 [] =
    {
        0x0000000b, 0x0000007d, 0x00000107, 0x7fff010a, 0x7fff010b, 
        0x7fff010c, 0x7fff010d, 0x7fff010e, 0x7fff010f, 0x7fff0110, 
        0x7fff0111, 0x7fff0112, 
    };

    static unsigned rp0030 [] =
    {
        0x000a, 0x0006, 0x0002, 0x000a, 0x0006, 0x0002, 0x000a, 0x0006, 
        0x0002, 0x000a, 0x0006, 0x0002, 0x000a, 0x0006, 0x0002, 0x000a, 
        0x0006, 0x0002, 0x000a, 0x0006, 0x0002, 0x000a, 0x0006, 0x0002, 
        0x000a, 0x0006, 0x0002, 0x000a, 0x0006, 0x0002, 0x000a, 0x0006, 
        0x0002, 
    };

    static unsigned long rd0032 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0032 [] =
    {
        0x001a, 0x000a, 0x0001, 
    };

    static unsigned long rd0033 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0033 [] =
    {
        0x001b, 0x000b, 0x0001, 
    };

    static unsigned long rd0034 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0034 [] =
    {
        0x001c, 0x000b, 0x0001, 
    };

    static unsigned long rd0035 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0035 [] =
    {
        0x000d, 0x0008, 0x0002, 
    };

    static unsigned long rd0037 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0037 [] =
    {
        0x0019, 0x000a, 0x0002, 
    };

    static unsigned long rd0038 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0038 [] =
    {
        0x001b, 0x000b, 0x0001, 
    };

    static unsigned long rd0039 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0039 [] =
    {
        0x001c, 0x000b, 0x0001, 
    };

    static unsigned long rd0040 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0040 [] =
    {
        0x000b, 0x0007, 0x0003, 
    };

    static unsigned long rd0041 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0041 [] =
    {
        0x0018, 0x0009, 0x0001, 
    };

    static unsigned long rd0042 [] =
    {
        0x00000001, 0x00000000, 
    };

    static unsigned rp0042 [] =
    {
        0x0003, 0x0002, 0x0006, 
    };

    /* Reductions Table */
    static unsigned long *rdtbl [] =
    {
        ppNULL, rd0001, rd0002, ppNULL, ppNULL, rd0005, rd0006, ppNULL, 
        rd0008, ppNULL, ppNULL, rd0011, ppNULL, rd0013, ppNULL, rd0015, 
        rd0016, rd0017, rd0018, rd0019, rd0020, rd0021, rd0022, rd0023, 
        rd0024, rd0025, rd0026, ppNULL, rd0028, rd0029, rd0030, ppNULL, 
        rd0032, rd0033, rd0034, rd0035, ppNULL, rd0037, rd0038, rd0039, 
        rd0040, rd0041, rd0042, 
    };

    /* Reductions Parameters Table */
    static unsigned *rptbl [] =
    {
        ppNULL, rp0001, rp0002, ppNULL, ppNULL, rp0005, rp0006, ppNULL, 
        rp0008, ppNULL, ppNULL, rp0011, ppNULL, rp0013, ppNULL, rp0015, 
        rp0016, rp0017, rp0018, rp0019, rp0020, rp0021, rp0022, rp0023, 
        rp0024, rp0025, rp0026, ppNULL, rp0028, rp0029, rp0030, ppNULL, 
        rp0032, rp0033, rp0034, rp0035, ppNULL, rp0037, rp0038, rp0039, 
        rp0040, rp0041, rp0042, 
    };

    /* Nonterminals Table */
    static unsigned nttbl [] =
    {
        0x0000, 0x0001, 0x0003, 0x0005, 0x0006, 0x0008, 0x000a, 0x000b, 
        0x000d, 0x0011, 0x0019, 0x001b, 
    };

    void instruction_definition (unsigned, int, char *[], unsigned []);
    void instruction_name (unsigned, int, char *[], unsigned []);
    void inc_formcount (unsigned, int, char *[], unsigned []);
    void operand (unsigned, int, char *[], unsigned []);
    void explicit_operand (unsigned, int, char *[], unsigned []);
    void opcode_specifier (unsigned, int, char *[], unsigned []);

    static void (*fctbl []) (unsigned, int, char *[], unsigned []) =
    {
        NULL, NULL, &instruction_definition, &instruction_name, NULL, 
        &inc_formcount, NULL, NULL, &operand, &explicit_operand, NULL, 
        &opcode_specifier, 
    };


    static unsigned long cbtbl [] =
    {
        0x00000001, 0x00000001, 0x00020001, 0x00030006, 0x00040004, 
        0x00050001, 0x00000000, 0x00060002, 0x00070001, 0x00000000, 
        0x00000000, 0x00000000, 0x00080002, 0x00090001, 0x000a0002, 
        0x000c0001, 0x000d0002, 0x000f0001, 0x00100001, 0x00110001, 
        0x00120001, 0x00130001, 0x00140001, 0x00150001, 0x00160001, 
        0x00170001, 0x00180001, 0x00000000, 0x00000000, 0x00000000, 
        0x00190002, 0x00000000, 0x001a0001, 0x001b0001, 0x001c0001, 
        0x00000000, 0x00080002, 0x00000000, 0x001b0001, 0x001c0001, 
        0x00000000, 0x00180001, 0x00000000, 
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

