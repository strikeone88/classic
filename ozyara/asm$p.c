/*
    PRSR-R-S.TEM

    Source code template file for the RedStar Paige Parser Generator
    Version 0.17, part of the Pegasus Project.

    Copyright (C) 2007-2008 RedStar Technologies (May 16 2008)
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <string.h>

    /* Simply shifts an empty symbol to take a stack space. */
    #define eshift() args [top++] = buftop; *buftop++ = '\0'

    /* Pushes the given value on the stack. */
    #define push(x) stack [sp++] = x

    /* Maximum stack depth. */
    #define MAX_DEPTH   1024

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
    static const unsigned long * const shtbl [];

    /* Shift targets table. */
    static const unsigned * const sttbl [];

    /* Reductions table. */
    static const unsigned long * const rdtbl [];

    /* Transitions table. */
    static const unsigned * const gttbl [];

    /* Reductions parameters table. */
    static const unsigned * const rptbl [];

    /* Function calls table. */
    static void (*fctbl []) (unsigned, int, char *[], unsigned []);

    /* Nonterminals table. */
    static const unsigned nttbl [];

    /* Catcher base table. */
    static const unsigned long cbtbl [];

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
    static unsigned srch_key (unsigned count, unsigned long const *lb,
                              unsigned long key)
    {
        unsigned cnt = count, damper = 0;
        register unsigned long elem;
        unsigned long const *p = lb;
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
    static unsigned srch_target (unsigned count, unsigned const *lb,
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
        register unsigned long const *lptr;
        register unsigned long symbol;
        register unsigned const *iptr;
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

    static const unsigned gt0000 [] =
    {
        0x0003, 0x0001, 0x0001, 0x0002, 0x0002, 0x0009, 0x0004, 
    };

    static const unsigned gt0002 [] =
    {
        0x0002, 0x0003, 0x0006, 0x000a, 0x0007, 
    };

    static const unsigned gt0003 [] =
    {
        0x0018, 0x0012, 0x0009, 0x0013, 0x000a, 0x0014, 0x000b, 0x0015, 
        0x0010, 0x0016, 0x001f, 0x0017, 0x0011, 0x0018, 0x001b, 0x001a, 
        0x001c, 0x001c, 0x0012, 0x001d, 0x0013, 0x001e, 0x0015, 0x001f, 
        0x0014, 0x0020, 0x0016, 0x0021, 0x0017, 0x0022, 0x0018, 0x0023, 
        0x0019, 0x0024, 0x001a, 0x0025, 0x001d, 0x0026, 0x002a, 0x0027, 
        0x002b, 0x002a, 0x001e, 0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 
        0x002f, 
    };

    static const unsigned gt0006 [] =
    {
        0x0002, 0x0004, 0x0033, 0x000a, 0x0034, 
    };

    static const unsigned gt0007 [] =
    {
        0x0001, 0x0009, 0x0035, 
    };

    static const unsigned gt0008 [] =
    {
        0x0001, 0x000b, 0x0036, 
    };

    static const unsigned gt0009 [] =
    {
        0x0017, 0x0013, 0x0038, 0x0014, 0x000b, 0x0015, 0x0010, 0x0016, 
        0x001f, 0x0017, 0x0011, 0x0018, 0x001b, 0x001a, 0x001c, 0x001c, 
        0x0012, 0x001d, 0x0013, 0x001e, 0x0015, 0x001f, 0x0014, 0x0020, 
        0x0016, 0x0021, 0x0017, 0x0022, 0x0018, 0x0023, 0x0019, 0x0024, 
        0x001a, 0x0025, 0x001d, 0x0026, 0x002a, 0x0027, 0x002b, 0x002a, 
        0x001e, 0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 0x002f, 
    };

    static const unsigned gt0013 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x003a, 
    };

    static const unsigned gt0021 [] =
    {
        0x0016, 0x0014, 0x0053, 0x0015, 0x0010, 0x0016, 0x001f, 0x0017, 
        0x0011, 0x0018, 0x001b, 0x001a, 0x001c, 0x001c, 0x0012, 0x001d, 
        0x0013, 0x001e, 0x0015, 0x001f, 0x0014, 0x0020, 0x0016, 0x0021, 
        0x0017, 0x0022, 0x0018, 0x0023, 0x0019, 0x0024, 0x001a, 0x0025, 
        0x001d, 0x0026, 0x002a, 0x0027, 0x002b, 0x002a, 0x001e, 0x002b, 
        0x002c, 0x002c, 0x002d, 0x002d, 0x002f, 
    };

    static const unsigned gt0033 [] =
    {
        0x0001, 0x000b, 0x0055, 
    };

    static const unsigned gt0035 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x005a, 
    };

    static const unsigned gt0036 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x005b, 
    };

    static const unsigned gt0037 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x005c, 
    };

    static const unsigned gt0040 [] =
    {
        0x0001, 0x0019, 0x005f, 
    };

    static const unsigned gt0041 [] =
    {
        0x0001, 0x001b, 0x0061, 
    };

    static const unsigned gt0042 [] =
    {
        0x0001, 0x0027, 0x0063, 
    };

    static const unsigned gt0043 [] =
    {
        0x0009, 0x0028, 0x0064, 0x0029, 0x0065, 0x003d, 0x0040, 0x003e, 
        0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 0x003c, 0x0042, 
        0x003b, 0x0043, 0x0066, 
    };

    static const unsigned gt0044 [] =
    {
        0x0002, 0x002c, 0x0068, 0x002d, 0x0069, 
    };

    static const unsigned gt0047 [] =
    {
        0x0012, 0x002e, 0x006a, 0x002f, 0x006b, 0x0030, 0x006c, 0x0031, 
        0x006e, 0x0032, 0x006f, 0x0033, 0x006d, 0x0034, 0x0071, 0x0035, 
        0x0081, 0x0036, 0x0080, 0x0037, 0x0072, 0x0038, 0x0073, 0x003d, 
        0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 
        0x003c, 0x0042, 0x003b, 0x0043, 0x0070, 
    };

    static const unsigned gt0051 [] =
    {
        0x0002, 0x0005, 0x0085, 0x0010, 0x0086, 
    };

    static const unsigned gt0052 [] =
    {
        0x0001, 0x0009, 0x0088, 
    };

    static const unsigned gt0054 [] =
    {
        0x0002, 0x000e, 0x0089, 0x000f, 0x008a, 
    };

    static const unsigned gt0065 [] =
    {
        0x0002, 0x003d, 0x0040, 0x003e, 0x009e, 
    };

    static const unsigned gt0066 [] =
    {
        0x0002, 0x003d, 0x0040, 0x003e, 0x009f, 
    };

    static const unsigned gt0067 [] =
    {
        0x0002, 0x003d, 0x0040, 0x003e, 0x00a0, 
    };

    static const unsigned gt0068 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00a1, 
    };

    static const unsigned gt0089 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00ad, 
    };

    static const unsigned gt0099 [] =
    {
        0x0009, 0x0028, 0x00b1, 0x0029, 0x0065, 0x003d, 0x0040, 0x003e, 
        0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 0x003c, 0x0042, 
        0x003b, 0x0043, 0x0066, 
    };

    static const unsigned gt0105 [] =
    {
        0x0012, 0x002e, 0x006a, 0x002f, 0x006b, 0x0030, 0x006c, 0x0031, 
        0x006e, 0x0032, 0x006f, 0x0033, 0x006d, 0x0034, 0x0071, 0x0035, 
        0x0081, 0x0036, 0x0080, 0x0037, 0x0072, 0x0038, 0x0073, 0x003d, 
        0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 
        0x003c, 0x0042, 0x003b, 0x0043, 0x0070, 
    };

    static const unsigned gt0113 [] =
    {
        0x0002, 0x0037, 0x00b6, 0x0038, 0x00b7, 
    };

    static const unsigned gt0114 [] =
    {
        0x0001, 0x0038, 0x00b9, 
    };

    static const unsigned gt0116 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00ba, 
    };

    static const unsigned gt0117 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00bc, 
    };

    static const unsigned gt0130 [] =
    {
        0x000b, 0x0039, 0x00be, 0x003a, 0x00c6, 0x003b, 0x00bf, 0x003c, 
        0x00c0, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00c7, 
    };

    static const unsigned gt0133 [] =
    {
        0x0004, 0x0006, 0x00ca, 0x0007, 0x00cb, 0x0008, 0x00cc, 0x000d, 
        0x00c9, 
    };

    static const unsigned gt0134 [] =
    {
        0x0001, 0x0009, 0x00cf, 
    };

    static const unsigned gt0135 [] =
    {
        0x0001, 0x000c, 0x00d0, 
    };

    static const unsigned gt0137 [] =
    {
        0x0001, 0x000f, 0x00d2, 
    };

    static const unsigned gt0141 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00d3, 
    };

    static const unsigned gt0148 [] =
    {
        0x0005, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x00d4, 
    };

    static const unsigned gt0149 [] =
    {
        0x0005, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x00d5, 
    };

    static const unsigned gt0150 [] =
    {
        0x0005, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x00d6, 
    };

    static const unsigned gt0151 [] =
    {
        0x0004, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x00d7, 
    };

    static const unsigned gt0152 [] =
    {
        0x0004, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x00d8, 
    };

    static const unsigned gt0153 [] =
    {
        0x0003, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x00d9, 
    };

    static const unsigned gt0154 [] =
    {
        0x0003, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x00da, 
    };

    static const unsigned gt0155 [] =
    {
        0x0002, 0x003d, 0x0040, 0x003e, 0x00db, 
    };

    static const unsigned gt0156 [] =
    {
        0x0002, 0x003d, 0x0040, 0x003e, 0x00dc, 
    };

    static const unsigned gt0157 [] =
    {
        0x0002, 0x003d, 0x0040, 0x003e, 0x00dd, 
    };

    static const unsigned gt0174 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00e2, 
    };

    static const unsigned gt0178 [] =
    {
        0x0008, 0x0029, 0x00e5, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 
        0x003e, 0x0040, 0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 
        0x0066, 
    };

    static const unsigned gt0180 [] =
    {
        0x0011, 0x002f, 0x00e7, 0x0030, 0x006c, 0x0031, 0x006e, 0x0032, 
        0x006f, 0x0033, 0x006d, 0x0034, 0x0071, 0x0035, 0x0081, 0x0036, 
        0x0080, 0x0037, 0x0072, 0x0038, 0x0073, 0x003d, 0x0040, 0x003e, 
        0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 0x003c, 0x0042, 
        0x003b, 0x0043, 0x0070, 
    };

    static const unsigned gt0181 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00e8, 
    };

    static const unsigned gt0182 [] =
    {
        0x0001, 0x0038, 0x00e9, 
    };

    static const unsigned gt0201 [] =
    {
        0x0018, 0x0012, 0x00f1, 0x0013, 0x000a, 0x0014, 0x000b, 0x0015, 
        0x0010, 0x0016, 0x001f, 0x0017, 0x0011, 0x0018, 0x001b, 0x001a, 
        0x001c, 0x001c, 0x0012, 0x001d, 0x0013, 0x001e, 0x0015, 0x001f, 
        0x0014, 0x0020, 0x0016, 0x0021, 0x0017, 0x0022, 0x0018, 0x0023, 
        0x0019, 0x0024, 0x001a, 0x0025, 0x001d, 0x0026, 0x002a, 0x0027, 
        0x002b, 0x002a, 0x001e, 0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 
        0x002f, 
    };

    static const unsigned gt0202 [] =
    {
        0x0001, 0x0009, 0x00f2, 
    };

    static const unsigned gt0208 [] =
    {
        0x0001, 0x0011, 0x00f5, 
    };

    static const unsigned gt0223 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00f7, 
    };

    static const unsigned gt0224 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00f8, 
    };

    static const unsigned gt0225 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00f9, 
    };

    static const unsigned gt0230 [] =
    {
        0x0007, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 
        0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 0x00fa, 
    };

    static const unsigned gt0234 [] =
    {
        0x000a, 0x0039, 0x00fb, 0x003a, 0x00c6, 0x003b, 0x00fc, 0x003d, 
        0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 
        0x003c, 0x0042, 0x003b, 0x0043, 0x00c7, 
    };

    static const unsigned gt0235 [] =
    {
        0x0008, 0x003a, 0x00fd, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 
        0x003e, 0x0040, 0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 
        0x00c7, 
    };

    static const unsigned gt0237 [] =
    {
        0x000a, 0x0039, 0x00ff, 0x003a, 0x00c6, 0x003b, 0x0100, 0x003d, 
        0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 
        0x003c, 0x0042, 0x003b, 0x0043, 0x00c7, 
    };

    static const unsigned gt0238 [] =
    {
        0x0008, 0x003a, 0x0101, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 
        0x003e, 0x0040, 0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 
        0x00c7, 
    };

    static const unsigned gt0241 [] =
    {
        0x0017, 0x0013, 0x0038, 0x0014, 0x000b, 0x0015, 0x0010, 0x0016, 
        0x001f, 0x0017, 0x0011, 0x0018, 0x001b, 0x001a, 0x001c, 0x001c, 
        0x0012, 0x001d, 0x0013, 0x001e, 0x0015, 0x001f, 0x0014, 0x0020, 
        0x0016, 0x0021, 0x0017, 0x0022, 0x0018, 0x0023, 0x0019, 0x0024, 
        0x001a, 0x0025, 0x001d, 0x0026, 0x002a, 0x0027, 0x002b, 0x002a, 
        0x001e, 0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 0x002f, 
    };

    static const unsigned gt0254 [] =
    {
        0x000a, 0x0039, 0x010d, 0x003a, 0x00c6, 0x003b, 0x010c, 0x003d, 
        0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 
        0x003c, 0x0042, 0x003b, 0x0043, 0x00c7, 
    };

    static const unsigned gt0258 [] =
    {
        0x000a, 0x0039, 0x010d, 0x003a, 0x00c6, 0x003b, 0x0112, 0x003d, 
        0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 
        0x003c, 0x0042, 0x003b, 0x0043, 0x00c7, 
    };

    static const unsigned gt0264 [] =
    {
        0x000a, 0x0039, 0x010d, 0x003a, 0x00c6, 0x003b, 0x0116, 0x003d, 
        0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 
        0x003c, 0x0042, 0x003b, 0x0043, 0x00c7, 
    };

    static const unsigned gt0265 [] =
    {
        0x0008, 0x003a, 0x0117, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 
        0x003e, 0x0040, 0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 
        0x00c7, 
    };

    static const unsigned gt0270 [] =
    {
        0x000a, 0x0039, 0x010d, 0x003a, 0x00c6, 0x003b, 0x0119, 0x003d, 
        0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 
        0x003c, 0x0042, 0x003b, 0x0043, 0x00c7, 
    };

    static const unsigned gt0271 [] =
    {
        0x0008, 0x003a, 0x011a, 0x003d, 0x0040, 0x003e, 0x003f, 0x003f, 
        0x003e, 0x0040, 0x003d, 0x0041, 0x003c, 0x0042, 0x003b, 0x0043, 
        0x00c7, 
    };

    static const unsigned gt0286 [] =
    {
        0x000a, 0x0039, 0x010d, 0x003a, 0x00c6, 0x003b, 0x0122, 0x003d, 
        0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 
        0x003c, 0x0042, 0x003b, 0x0043, 0x00c7, 
    };

    static const unsigned gt0289 [] =
    {
        0x000a, 0x0039, 0x010d, 0x003a, 0x00c6, 0x003b, 0x0123, 0x003d, 
        0x0040, 0x003e, 0x003f, 0x003f, 0x003e, 0x0040, 0x003d, 0x0041, 
        0x003c, 0x0042, 0x003b, 0x0043, 0x00c7, 
    };

    /* Transitions Table */
    static const unsigned * const gttbl [] =
    {
        gt0000, ppNULL, gt0002, gt0003, ppNULL, ppNULL, gt0006, gt0007, 
        gt0008, gt0009, ppNULL, ppNULL, ppNULL, gt0013, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, gt0021, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, gt0033, ppNULL, gt0035, gt0036, gt0037, ppNULL, ppNULL, 
        gt0040, gt0041, gt0042, gt0043, gt0044, ppNULL, ppNULL, gt0047, 
        ppNULL, ppNULL, ppNULL, gt0051, gt0052, ppNULL, gt0054, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, gt0065, gt0066, gt0067, gt0068, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, gt0089, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, gt0099, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, gt0105, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, gt0113, gt0114, ppNULL, gt0116, gt0117, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, gt0130, ppNULL, ppNULL, gt0133, gt0134, gt0135, 
        ppNULL, gt0137, ppNULL, ppNULL, ppNULL, gt0141, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, gt0148, gt0149, gt0150, gt0151, 
        gt0152, gt0153, gt0154, gt0155, gt0156, gt0157, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, gt0174, ppNULL, 
        ppNULL, ppNULL, gt0178, ppNULL, gt0180, gt0181, gt0182, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, gt0201, gt0202, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        gt0208, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, gt0223, 
        gt0224, gt0225, ppNULL, ppNULL, ppNULL, ppNULL, gt0230, ppNULL, 
        ppNULL, ppNULL, gt0234, gt0235, ppNULL, gt0237, gt0238, ppNULL, 
        ppNULL, gt0241, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, gt0254, ppNULL, 
        ppNULL, ppNULL, gt0258, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        gt0264, gt0265, ppNULL, ppNULL, ppNULL, ppNULL, gt0270, gt0271, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, gt0286, ppNULL, 
        ppNULL, gt0289, ppNULL, ppNULL, ppNULL, ppNULL, 
    };

    static const unsigned long sh0000 [] =
    {
        0x00000002, 0x00000001, 0x0000010b, 
    };

    static const unsigned st0000 [] =
    {
        0x0003, 0x0005, 
    };

    static const unsigned long sh0002 [] =
    {
        0x00000001, 0x00000111, 
    };

    static const unsigned st0002 [] =
    {
        0x0008, 
    };

    static const unsigned long sh0003 [] =
    {
        0x00000011, 0x0000005b, 0x00000108, 0x0000010a, 0x0000010b, 
        0x00000112, 0x00040111, 0x00050111, 0x00070111, 0x000b0111, 
        0x000c0111, 0x000d0111, 0x000e0111, 0x000f0111, 0x001b0111, 
        0x7fff0117, 0x7fff0118, 0x7fff0123, 
    };

    static const unsigned st0003 [] =
    {
        0x0021, 0x0022, 0x000f, 0x000c, 0x0028, 0x0023, 0x0029, 0x0025, 
        0x0026, 0x0027, 0x000d, 0x000e, 0x0024, 0x0020, 0x002e, 0x0030, 
        0x0031, 
    };

    static const unsigned long sh0004 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned st0004 [] =
    {
        0x0032, 
    };

    static const unsigned long sh0006 [] =
    {
        0x00000001, 0x00000111, 
    };

    static const unsigned st0006 [] =
    {
        0x0008, 
    };

    static const unsigned long sh0007 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned st0007 [] =
    {
        0x0005, 
    };

    static const unsigned long sh0008 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0008 [] =
    {
        0x0037, 
    };

    static const unsigned long sh0009 [] =
    {
        0x00000011, 0x0000005b, 0x00000108, 0x0000010a, 0x0000010b, 
        0x00000112, 0x00040111, 0x00050111, 0x00070111, 0x000b0111, 
        0x000c0111, 0x000d0111, 0x000e0111, 0x000f0111, 0x001b0111, 
        0x7fff0117, 0x7fff0118, 0x7fff0123, 
    };

    static const unsigned st0009 [] =
    {
        0x0021, 0x0022, 0x000f, 0x000c, 0x0028, 0x0023, 0x0029, 0x0025, 
        0x0026, 0x0027, 0x000d, 0x000e, 0x0024, 0x0020, 0x002e, 0x0030, 
        0x0031, 
    };

    static const unsigned long sh0011 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned st0011 [] =
    {
        0x0039, 
    };

    static const unsigned long sh0013 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0013 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0021 [] =
    {
        0x00000010, 0x0000005b, 0x00000108, 0x0000010a, 0x00000112, 
        0x00040111, 0x00050111, 0x00070111, 0x000b0111, 0x000c0111, 
        0x000d0111, 0x000e0111, 0x000f0111, 0x001b0111, 0x7fff0117, 
        0x7fff0118, 0x7fff0123, 
    };

    static const unsigned st0021 [] =
    {
        0x0021, 0x0022, 0x000f, 0x0028, 0x0023, 0x0029, 0x0025, 0x0026, 
        0x0027, 0x000d, 0x000e, 0x0024, 0x0020, 0x002e, 0x0030, 0x0031, 
    };

    static const unsigned long sh0032 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0032 [] =
    {
        0x0054, 
    };

    static const unsigned long sh0033 [] =
    {
        0x00000002, 0x00000108, 0x00030111, 
    };

    static const unsigned st0033 [] =
    {
        0x0037, 0x0056, 
    };

    static const unsigned long sh0034 [] =
    {
        0x00000003, 0x0000003a, 0x0000003d, 0x001a0111, 
    };

    static const unsigned st0034 [] =
    {
        0x0058, 0x0059, 0x0057, 
    };

    static const unsigned long sh0035 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0035 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0036 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0036 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0037 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0037 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0038 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0038 [] =
    {
        0x005d, 
    };

    static const unsigned long sh0039 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0039 [] =
    {
        0x005e, 
    };

    static const unsigned long sh0040 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0040 [] =
    {
        0x0060, 
    };

    static const unsigned long sh0041 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0041 [] =
    {
        0x0062, 
    };

    static const unsigned long sh0042 [] =
    {
        0x00000001, 0x7fff0117, 
    };

    static const unsigned st0042 [] =
    {
        0x002e, 
    };

    static const unsigned long sh0043 [] =
    {
        0x00000013, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000106, 0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 
        0x0000010f, 0x00000110, 0x00000111, 0x00020118, 0x00080111, 
        0x00090111, 0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0043 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x0067, 0x004b, 0x0046, 0x0047, 
        0x0048, 0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 
        0x0050, 0x0051, 0x0052, 
    };

    static const unsigned long sh0044 [] =
    {
        0x00000002, 0x7fff0118, 0x7fff0123, 
    };

    static const unsigned st0044 [] =
    {
        0x0030, 0x0031, 
    };

    static const unsigned long sh0047 [] =
    {
        0x00000021, 0x00000028, 0x0000002b, 0x0000002d, 0x0000005b, 
        0x00000105, 0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 
        0x0000010f, 0x00000110, 0x00000111, 0x00020118, 0x00080111, 
        0x00090111, 0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
        0x7fff0113, 0x7fff0114, 0x7fff0115, 0x7fff0116, 0x7fff0119, 
        0x7fff011a, 0x7fff011b, 0x7fff011c, 0x7fff011d, 0x7fff011e, 
        0x7fff011f, 0x7fff0120, 0x7fff0121, 0x7fff0122, 
    };

    static const unsigned st0047 [] =
    {
        0x0044, 0x0042, 0x0043, 0x0082, 0x004a, 0x004b, 0x0046, 0x0047, 
        0x0048, 0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 
        0x0050, 0x0051, 0x0052, 0x0074, 0x0075, 0x0084, 0x0083, 0x0076, 
        0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 
        0x007f, 
    };

    static const unsigned long sh0051 [] =
    {
        0x00000001, 0x00010111, 
    };

    static const unsigned st0051 [] =
    {
        0x0087, 
    };

    static const unsigned long sh0052 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned st0052 [] =
    {
        0x0005, 
    };

    static const unsigned long sh0053 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned st0053 [] =
    {
        0x0032, 
    };

    static const unsigned long sh0054 [] =
    {
        0x00000009, 0x00000105, 0x00000113, 0x00010113, 0x00020113, 
        0x00040111, 0x00060113, 0x00100111, 0x00110111, 0x7fff0112, 
    };

    static const unsigned st0054 [] =
    {
        0x008c, 0x008e, 0x008f, 0x0090, 0x008d, 0x0091, 0x0092, 0x0093, 
        0x008b, 
    };

    static const unsigned long sh0059 [] =
    {
        0x00000003, 0x00030118, 0x00040118, 0x00050118, 
    };

    static const unsigned st0059 [] =
    {
        0x0094, 0x0095, 0x0096, 
    };

    static const unsigned long sh0060 [] =
    {
        0x00000002, 0x00000118, 0x00010118, 
    };

    static const unsigned st0060 [] =
    {
        0x0097, 0x0098, 
    };

    static const unsigned long sh0061 [] =
    {
        0x00000002, 0x0000002b, 0x0000002d, 
    };

    static const unsigned st0061 [] =
    {
        0x0099, 0x009a, 
    };

    static const unsigned long sh0062 [] =
    {
        0x00000003, 0x0000002a, 0x0000002f, 0x00180111, 
    };

    static const unsigned st0062 [] =
    {
        0x009b, 0x009c, 0x009d, 
    };

    static const unsigned long sh0065 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0065 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0066 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0066 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0067 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0067 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0068 [] =
    {
        0x00000015, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00010114, 0x00020118, 0x00080111, 
        0x00090111, 0x000a0111, 0x00160111, 0x00170111, 0x00190111, 
        0x001c0111, 0x001d0111, 
    };

    static const unsigned st0068 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x00a4, 0x0041, 0x004d, 0x004e, 0x004f, 
        0x00a2, 0x00a3, 0x0050, 0x0051, 0x0052, 
    };

    static const unsigned long sh0076 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0076 [] =
    {
        0x00a5, 
    };

    static const unsigned long sh0077 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0077 [] =
    {
        0x00a6, 
    };

    static const unsigned long sh0078 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0078 [] =
    {
        0x00a7, 
    };

    static const unsigned long sh0079 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0079 [] =
    {
        0x00a8, 
    };

    static const unsigned long sh0080 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0080 [] =
    {
        0x00a9, 
    };

    static const unsigned long sh0085 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0085 [] =
    {
        0x00aa, 
    };

    static const unsigned long sh0086 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0086 [] =
    {
        0x00ab, 
    };

    static const unsigned long sh0087 [] =
    {
        0x00000001, 0x7fff0113, 
    };

    static const unsigned st0087 [] =
    {
        0x00ac, 
    };

    static const unsigned long sh0089 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0089 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0091 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static const unsigned st0091 [] =
    {
        0x00ae, 
    };

    static const unsigned long sh0095 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static const unsigned st0095 [] =
    {
        0x00af, 
    };

    static const unsigned long sh0097 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static const unsigned st0097 [] =
    {
        0x00b0, 
    };

    static const unsigned long sh0099 [] =
    {
        0x00000013, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000106, 0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 
        0x0000010f, 0x00000110, 0x00000111, 0x00020118, 0x00080111, 
        0x00090111, 0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0099 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x0067, 0x004b, 0x0046, 0x0047, 
        0x0048, 0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 
        0x0050, 0x0051, 0x0052, 
    };

    static const unsigned long sh0100 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static const unsigned st0100 [] =
    {
        0x00b2, 
    };

    static const unsigned long sh0102 [] =
    {
        0x00000001, 0x00060111, 
    };

    static const unsigned st0102 [] =
    {
        0x00b3, 
    };

    static const unsigned long sh0105 [] =
    {
        0x00000021, 0x00000028, 0x0000002b, 0x0000002d, 0x0000005b, 
        0x00000105, 0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 
        0x0000010f, 0x00000110, 0x00000111, 0x00020118, 0x00080111, 
        0x00090111, 0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
        0x7fff0113, 0x7fff0114, 0x7fff0115, 0x7fff0116, 0x7fff0119, 
        0x7fff011a, 0x7fff011b, 0x7fff011c, 0x7fff011d, 0x7fff011e, 
        0x7fff011f, 0x7fff0120, 0x7fff0121, 0x7fff0122, 
    };

    static const unsigned st0105 [] =
    {
        0x0044, 0x0042, 0x0043, 0x0082, 0x004a, 0x004b, 0x0046, 0x0047, 
        0x0048, 0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 
        0x0050, 0x0051, 0x0052, 0x0074, 0x0075, 0x0084, 0x0083, 0x0076, 
        0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 
        0x007f, 
    };

    static const unsigned long sh0106 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static const unsigned st0106 [] =
    {
        0x00b4, 
    };

    static const unsigned long sh0112 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static const unsigned st0112 [] =
    {
        0x00b5, 
    };

    static const unsigned long sh0113 [] =
    {
        0x00000002, 0x0000005b, 0x7fff011c, 
    };

    static const unsigned st0113 [] =
    {
        0x0082, 0x00b8, 
    };

    static const unsigned long sh0114 [] =
    {
        0x00000001, 0x0000005b, 
    };

    static const unsigned st0114 [] =
    {
        0x0082, 
    };

    static const unsigned long sh0116 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0116 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x00bb, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0117 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0117 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0121 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static const unsigned st0121 [] =
    {
        0x00bd, 
    };

    static const unsigned long sh0130 [] =
    {
        0x00000017, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x0003011a, 0x0005011a, 
        0x0006011a, 0x0007011a, 0x00080111, 0x00090111, 0x000a0111, 
        0x00190111, 0x001c0111, 0x001d0111, 0x7fff011b, 
    };

    static const unsigned st0130 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 
        0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x00c1, 
    };

    static const unsigned long sh0132 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static const unsigned st0132 [] =
    {
        0x00c8, 
    };

    static const unsigned long sh0133 [] =
    {
        0x00000002, 0x00120111, 0x00130111, 
    };

    static const unsigned st0133 [] =
    {
        0x00cd, 0x00ce, 
    };

    static const unsigned long sh0134 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned st0134 [] =
    {
        0x0005, 
    };

    static const unsigned long sh0135 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0135 [] =
    {
        0x00d1, 
    };

    static const unsigned long sh0136 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned st0136 [] =
    {
        0x0032, 
    };

    static const unsigned long sh0137 [] =
    {
        0x00000009, 0x00000105, 0x00000113, 0x00010113, 0x00020113, 
        0x00040111, 0x00060113, 0x00100111, 0x00110111, 0x7fff0112, 
    };

    static const unsigned st0137 [] =
    {
        0x008c, 0x008e, 0x008f, 0x0090, 0x008d, 0x0091, 0x0092, 0x0093, 
        0x008b, 
    };

    static const unsigned long sh0141 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0141 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0148 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0148 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0149 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0149 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0150 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0150 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0151 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0151 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0152 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0152 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0153 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0153 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0154 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0154 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0155 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0155 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0156 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0156 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0157 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0157 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0161 [] =
    {
        0x00000001, 0x00000029, 
    };

    static const unsigned st0161 [] =
    {
        0x00de, 
    };

    static const unsigned long sh0162 [] =
    {
        0x00000001, 0x00000029, 
    };

    static const unsigned st0162 [] =
    {
        0x00df, 
    };

    static const unsigned long sh0163 [] =
    {
        0x00000001, 0x00000029, 
    };

    static const unsigned st0163 [] =
    {
        0x00e0, 
    };

    static const unsigned long sh0164 [] =
    {
        0x00000001, 0x00000029, 
    };

    static const unsigned st0164 [] =
    {
        0x00e1, 
    };

    static const unsigned long sh0174 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0174 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0175 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0175 [] =
    {
        0x00e3, 
    };

    static const unsigned long sh0176 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0176 [] =
    {
        0x00e4, 
    };

    static const unsigned long sh0177 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static const unsigned st0177 [] =
    {
        0x00b2, 
    };

    static const unsigned long sh0178 [] =
    {
        0x00000013, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000106, 0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 
        0x0000010f, 0x00000110, 0x00000111, 0x00020118, 0x00080111, 
        0x00090111, 0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0178 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x0067, 0x004b, 0x0046, 0x0047, 
        0x0048, 0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 
        0x0050, 0x0051, 0x0052, 
    };

    static const unsigned long sh0179 [] =
    {
        0x00000001, 0x00000028, 
    };

    static const unsigned st0179 [] =
    {
        0x00e6, 
    };

    static const unsigned long sh0180 [] =
    {
        0x00000021, 0x00000028, 0x0000002b, 0x0000002d, 0x0000005b, 
        0x00000105, 0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 
        0x0000010f, 0x00000110, 0x00000111, 0x00020118, 0x00080111, 
        0x00090111, 0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
        0x7fff0113, 0x7fff0114, 0x7fff0115, 0x7fff0116, 0x7fff0119, 
        0x7fff011a, 0x7fff011b, 0x7fff011c, 0x7fff011d, 0x7fff011e, 
        0x7fff011f, 0x7fff0120, 0x7fff0121, 0x7fff0122, 
    };

    static const unsigned st0180 [] =
    {
        0x0044, 0x0042, 0x0043, 0x0082, 0x004a, 0x004b, 0x0046, 0x0047, 
        0x0048, 0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 
        0x0050, 0x0051, 0x0052, 0x0074, 0x0075, 0x0084, 0x0083, 0x0076, 
        0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 
        0x007f, 
    };

    static const unsigned long sh0181 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0181 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0182 [] =
    {
        0x00000001, 0x0000005b, 
    };

    static const unsigned st0182 [] =
    {
        0x0082, 
    };

    static const unsigned long sh0184 [] =
    {
        0x00000001, 0x0000003a, 
    };

    static const unsigned st0184 [] =
    {
        0x00bd, 
    };

    static const unsigned long sh0187 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0187 [] =
    {
        0x00a9, 
    };

    static const unsigned long sh0190 [] =
    {
        0x00000002, 0x0000002b, 0x0000002d, 
    };

    static const unsigned st0190 [] =
    {
        0x00ea, 0x00eb, 
    };

    static const unsigned long sh0191 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0191 [] =
    {
        0x00ec, 
    };

    static const unsigned long sh0192 [] =
    {
        0x00000003, 0x0000002b, 0x0000002d, 0x0000005d, 
    };

    static const unsigned st0192 [] =
    {
        0x00ed, 0x00ee, 0x00ef, 
    };

    static const unsigned long sh0193 [] =
    {
        0x00000001, 0x0000002a, 
    };

    static const unsigned st0193 [] =
    {
        0x00f0, 
    };

    static const unsigned long sh0201 [] =
    {
        0x00000011, 0x0000005b, 0x00000108, 0x0000010a, 0x0000010b, 
        0x00000112, 0x00040111, 0x00050111, 0x00070111, 0x000b0111, 
        0x000c0111, 0x000d0111, 0x000e0111, 0x000f0111, 0x001b0111, 
        0x7fff0117, 0x7fff0118, 0x7fff0123, 
    };

    static const unsigned st0201 [] =
    {
        0x0021, 0x0022, 0x000f, 0x000c, 0x0028, 0x0023, 0x0029, 0x0025, 
        0x0026, 0x0027, 0x000d, 0x000e, 0x0024, 0x0020, 0x002e, 0x0030, 
        0x0031, 
    };

    static const unsigned long sh0202 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned st0202 [] =
    {
        0x0005, 
    };

    static const unsigned long sh0205 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0205 [] =
    {
        0x00f3, 
    };

    static const unsigned long sh0206 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0206 [] =
    {
        0x00f4, 
    };

    static const unsigned long sh0207 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned st0207 [] =
    {
        0x0032, 
    };

    static const unsigned long sh0208 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0208 [] =
    {
        0x00f6, 
    };

    static const unsigned long sh0212 [] =
    {
        0x00000002, 0x00000118, 0x00010118, 
    };

    static const unsigned st0212 [] =
    {
        0x0097, 0x0098, 
    };

    static const unsigned long sh0213 [] =
    {
        0x00000002, 0x00000118, 0x00010118, 
    };

    static const unsigned st0213 [] =
    {
        0x0097, 0x0098, 
    };

    static const unsigned long sh0214 [] =
    {
        0x00000002, 0x00000118, 0x00010118, 
    };

    static const unsigned st0214 [] =
    {
        0x0097, 0x0098, 
    };

    static const unsigned long sh0215 [] =
    {
        0x00000002, 0x0000002b, 0x0000002d, 
    };

    static const unsigned st0215 [] =
    {
        0x0099, 0x009a, 
    };

    static const unsigned long sh0216 [] =
    {
        0x00000002, 0x0000002b, 0x0000002d, 
    };

    static const unsigned st0216 [] =
    {
        0x0099, 0x009a, 
    };

    static const unsigned long sh0217 [] =
    {
        0x00000003, 0x0000002a, 0x0000002f, 0x00180111, 
    };

    static const unsigned st0217 [] =
    {
        0x009b, 0x009c, 0x009d, 
    };

    static const unsigned long sh0218 [] =
    {
        0x00000003, 0x0000002a, 0x0000002f, 0x00180111, 
    };

    static const unsigned st0218 [] =
    {
        0x009b, 0x009c, 0x009d, 
    };

    static const unsigned long sh0223 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0223 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0224 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0224 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0225 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0225 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0230 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0230 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0234 [] =
    {
        0x00000017, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x0003011a, 0x0005011a, 
        0x0006011a, 0x0007011a, 0x00080111, 0x00090111, 0x000a0111, 
        0x00190111, 0x001c0111, 0x001d0111, 0x7fff011b, 
    };

    static const unsigned st0234 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 
        0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x00c1, 
    };

    static const unsigned long sh0235 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0235 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0236 [] =
    {
        0x00000001, 0x0000005b, 
    };

    static const unsigned st0236 [] =
    {
        0x00fe, 
    };

    static const unsigned long sh0237 [] =
    {
        0x00000017, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x0003011a, 0x0005011a, 
        0x0006011a, 0x0007011a, 0x00080111, 0x00090111, 0x000a0111, 
        0x00190111, 0x001c0111, 0x001d0111, 0x7fff011b, 
    };

    static const unsigned st0237 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 
        0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x00c1, 
    };

    static const unsigned long sh0238 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0238 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0239 [] =
    {
        0x00000001, 0x0000005b, 
    };

    static const unsigned st0239 [] =
    {
        0x0102, 
    };

    static const unsigned long sh0240 [] =
    {
        0x00000001, 0x0000010d, 
    };

    static const unsigned st0240 [] =
    {
        0x0103, 
    };

    static const unsigned long sh0241 [] =
    {
        0x00000011, 0x0000005b, 0x00000108, 0x0000010a, 0x0000010b, 
        0x00000112, 0x00040111, 0x00050111, 0x00070111, 0x000b0111, 
        0x000c0111, 0x000d0111, 0x000e0111, 0x000f0111, 0x001b0111, 
        0x7fff0117, 0x7fff0118, 0x7fff0123, 
    };

    static const unsigned st0241 [] =
    {
        0x0021, 0x0022, 0x000f, 0x000c, 0x0028, 0x0023, 0x0029, 0x0025, 
        0x0026, 0x0027, 0x000d, 0x000e, 0x0024, 0x0020, 0x002e, 0x0030, 
        0x0031, 
    };

    static const unsigned long sh0242 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned st0242 [] =
    {
        0x0032, 
    };

    static const unsigned long sh0243 [] =
    {
        0x00000001, 0x00140111, 
    };

    static const unsigned st0243 [] =
    {
        0x0104, 
    };

    static const unsigned long sh0244 [] =
    {
        0x00000001, 0x00150111, 
    };

    static const unsigned st0244 [] =
    {
        0x0105, 
    };

    static const unsigned long sh0245 [] =
    {
        0x00000001, 0x0000002c, 
    };

    static const unsigned st0245 [] =
    {
        0x0106, 
    };

    static const unsigned long sh0250 [] =
    {
        0x00000001, 0x00000029, 
    };

    static const unsigned st0250 [] =
    {
        0x0107, 
    };

    static const unsigned long sh0251 [] =
    {
        0x00000002, 0x0000002b, 0x0000002d, 
    };

    static const unsigned st0251 [] =
    {
        0x0108, 0x0109, 
    };

    static const unsigned long sh0252 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0252 [] =
    {
        0x010a, 
    };

    static const unsigned long sh0253 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0253 [] =
    {
        0x010b, 
    };

    static const unsigned long sh0254 [] =
    {
        0x00000017, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x0003011a, 0x0005011a, 
        0x0006011a, 0x0007011a, 0x00080111, 0x00090111, 0x000a0111, 
        0x00190111, 0x001c0111, 0x001d0111, 0x7fff011b, 
    };

    static const unsigned st0254 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 
        0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x00c1, 
    };

    static const unsigned long sh0255 [] =
    {
        0x00000002, 0x0000002b, 0x0000002d, 
    };

    static const unsigned st0255 [] =
    {
        0x010e, 0x010f, 
    };

    static const unsigned long sh0256 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0256 [] =
    {
        0x0110, 
    };

    static const unsigned long sh0257 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0257 [] =
    {
        0x0111, 
    };

    static const unsigned long sh0258 [] =
    {
        0x00000017, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x0003011a, 0x0005011a, 
        0x0006011a, 0x0007011a, 0x00080111, 0x00090111, 0x000a0111, 
        0x00190111, 0x001c0111, 0x001d0111, 0x7fff011b, 
    };

    static const unsigned st0258 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 
        0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x00c1, 
    };

    static const unsigned long sh0260 [] =
    {
        0x00000001, 0x00000106, 
    };

    static const unsigned st0260 [] =
    {
        0x0113, 
    };

    static const unsigned long sh0261 [] =
    {
        0x00000001, 0x00000106, 
    };

    static const unsigned st0261 [] =
    {
        0x0114, 
    };

    static const unsigned long sh0262 [] =
    {
        0x00000001, 0x00000108, 
    };

    static const unsigned st0262 [] =
    {
        0x0115, 
    };

    static const unsigned long sh0264 [] =
    {
        0x00000017, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x0003011a, 0x0005011a, 
        0x0006011a, 0x0007011a, 0x00080111, 0x00090111, 0x000a0111, 
        0x00190111, 0x001c0111, 0x001d0111, 0x7fff011b, 
    };

    static const unsigned st0264 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 
        0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x00c1, 
    };

    static const unsigned long sh0265 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0265 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0268 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0268 [] =
    {
        0x0118, 
    };

    static const unsigned long sh0270 [] =
    {
        0x00000017, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x0003011a, 0x0005011a, 
        0x0006011a, 0x0007011a, 0x00080111, 0x00090111, 0x000a0111, 
        0x00190111, 0x001c0111, 0x001d0111, 0x7fff011b, 
    };

    static const unsigned st0270 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 
        0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x00c1, 
    };

    static const unsigned long sh0271 [] =
    {
        0x00000012, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x00080111, 0x00090111, 
        0x000a0111, 0x00190111, 0x001c0111, 0x001d0111, 
    };

    static const unsigned st0271 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x004d, 0x004e, 0x004f, 0x0050, 
        0x0051, 0x0052, 
    };

    static const unsigned long sh0274 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0274 [] =
    {
        0x011b, 
    };

    static const unsigned long sh0278 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0278 [] =
    {
        0x011c, 
    };

    static const unsigned long sh0279 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0279 [] =
    {
        0x011d, 
    };

    static const unsigned long sh0280 [] =
    {
        0x00000001, 0x0000005b, 
    };

    static const unsigned st0280 [] =
    {
        0x011e, 
    };

    static const unsigned long sh0281 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0281 [] =
    {
        0x011f, 
    };

    static const unsigned long sh0282 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0282 [] =
    {
        0x0120, 
    };

    static const unsigned long sh0283 [] =
    {
        0x00000001, 0x0000005b, 
    };

    static const unsigned st0283 [] =
    {
        0x0121, 
    };

    static const unsigned long sh0286 [] =
    {
        0x00000017, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x0003011a, 0x0005011a, 
        0x0006011a, 0x0007011a, 0x00080111, 0x00090111, 0x000a0111, 
        0x00190111, 0x001c0111, 0x001d0111, 0x7fff011b, 
    };

    static const unsigned st0286 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 
        0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x00c1, 
    };

    static const unsigned long sh0289 [] =
    {
        0x00000017, 0x00000028, 0x0000002b, 0x0000002d, 0x00000105, 
        0x00000108, 0x0000010c, 0x0000010d, 0x0000010e, 0x0000010f, 
        0x00000110, 0x00000111, 0x00020118, 0x0003011a, 0x0005011a, 
        0x0006011a, 0x0007011a, 0x00080111, 0x00090111, 0x000a0111, 
        0x00190111, 0x001c0111, 0x001d0111, 0x7fff011b, 
    };

    static const unsigned st0289 [] =
    {
        0x0044, 0x0042, 0x0043, 0x004a, 0x004b, 0x0046, 0x0047, 0x0048, 
        0x0049, 0x0045, 0x004c, 0x0041, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 
        0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x00c1, 
    };

    static const unsigned long sh0290 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0290 [] =
    {
        0x0124, 
    };

    static const unsigned long sh0291 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned st0291 [] =
    {
        0x0125, 
    };

    /* Shifts Table */
    static const unsigned long * const shtbl [] =
    {
        sh0000, ppNULL, sh0002, sh0003, sh0004, ppNULL, sh0006, sh0007, 
        sh0008, sh0009, ppNULL, sh0011, ppNULL, sh0013, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, sh0021, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        sh0032, sh0033, sh0034, sh0035, sh0036, sh0037, sh0038, sh0039, 
        sh0040, sh0041, sh0042, sh0043, sh0044, ppNULL, ppNULL, sh0047, 
        ppNULL, ppNULL, ppNULL, sh0051, sh0052, sh0053, sh0054, ppNULL, 
        ppNULL, ppNULL, ppNULL, sh0059, sh0060, sh0061, sh0062, ppNULL, 
        ppNULL, sh0065, sh0066, sh0067, sh0068, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, sh0076, sh0077, sh0078, sh0079, 
        sh0080, ppNULL, ppNULL, ppNULL, ppNULL, sh0085, sh0086, sh0087, 
        ppNULL, sh0089, ppNULL, sh0091, ppNULL, ppNULL, ppNULL, sh0095, 
        ppNULL, sh0097, ppNULL, sh0099, sh0100, ppNULL, sh0102, ppNULL, 
        ppNULL, sh0105, sh0106, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        sh0112, sh0113, sh0114, ppNULL, sh0116, sh0117, ppNULL, ppNULL, 
        ppNULL, sh0121, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, sh0130, ppNULL, sh0132, sh0133, sh0134, sh0135, 
        sh0136, sh0137, ppNULL, ppNULL, ppNULL, sh0141, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, sh0148, sh0149, sh0150, sh0151, 
        sh0152, sh0153, sh0154, sh0155, sh0156, sh0157, ppNULL, ppNULL, 
        ppNULL, sh0161, sh0162, sh0163, sh0164, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, sh0174, sh0175, 
        sh0176, sh0177, sh0178, sh0179, sh0180, sh0181, sh0182, ppNULL, 
        sh0184, ppNULL, ppNULL, sh0187, ppNULL, ppNULL, sh0190, sh0191, 
        sh0192, sh0193, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, sh0201, sh0202, ppNULL, ppNULL, sh0205, sh0206, sh0207, 
        sh0208, ppNULL, ppNULL, ppNULL, sh0212, sh0213, sh0214, sh0215, 
        sh0216, sh0217, sh0218, ppNULL, ppNULL, ppNULL, ppNULL, sh0223, 
        sh0224, sh0225, ppNULL, ppNULL, ppNULL, ppNULL, sh0230, ppNULL, 
        ppNULL, ppNULL, sh0234, sh0235, sh0236, sh0237, sh0238, sh0239, 
        sh0240, sh0241, sh0242, sh0243, sh0244, sh0245, ppNULL, ppNULL, 
        ppNULL, ppNULL, sh0250, sh0251, sh0252, sh0253, sh0254, sh0255, 
        sh0256, sh0257, sh0258, ppNULL, sh0260, sh0261, sh0262, ppNULL, 
        sh0264, sh0265, ppNULL, ppNULL, sh0268, ppNULL, sh0270, sh0271, 
        ppNULL, ppNULL, sh0274, ppNULL, ppNULL, ppNULL, sh0278, sh0279, 
        sh0280, sh0281, sh0282, sh0283, ppNULL, ppNULL, sh0286, ppNULL, 
        ppNULL, sh0289, sh0290, sh0291, ppNULL, ppNULL, 
    };

    /* Shifts Targets Table */
    static const unsigned * const sttbl [] =
    {
        st0000, ppNULL, st0002, st0003, st0004, ppNULL, st0006, st0007, 
        st0008, st0009, ppNULL, st0011, ppNULL, st0013, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, st0021, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        st0032, st0033, st0034, st0035, st0036, st0037, st0038, st0039, 
        st0040, st0041, st0042, st0043, st0044, ppNULL, ppNULL, st0047, 
        ppNULL, ppNULL, ppNULL, st0051, st0052, st0053, st0054, ppNULL, 
        ppNULL, ppNULL, ppNULL, st0059, st0060, st0061, st0062, ppNULL, 
        ppNULL, st0065, st0066, st0067, st0068, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, st0076, st0077, st0078, st0079, 
        st0080, ppNULL, ppNULL, ppNULL, ppNULL, st0085, st0086, st0087, 
        ppNULL, st0089, ppNULL, st0091, ppNULL, ppNULL, ppNULL, st0095, 
        ppNULL, st0097, ppNULL, st0099, st0100, ppNULL, st0102, ppNULL, 
        ppNULL, st0105, st0106, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        st0112, st0113, st0114, ppNULL, st0116, st0117, ppNULL, ppNULL, 
        ppNULL, st0121, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, st0130, ppNULL, st0132, st0133, st0134, st0135, 
        st0136, st0137, ppNULL, ppNULL, ppNULL, st0141, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, st0148, st0149, st0150, st0151, 
        st0152, st0153, st0154, st0155, st0156, st0157, ppNULL, ppNULL, 
        ppNULL, st0161, st0162, st0163, st0164, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, st0174, st0175, 
        st0176, st0177, st0178, st0179, st0180, st0181, st0182, ppNULL, 
        st0184, ppNULL, ppNULL, st0187, ppNULL, ppNULL, st0190, st0191, 
        st0192, st0193, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, st0201, st0202, ppNULL, ppNULL, st0205, st0206, st0207, 
        st0208, ppNULL, ppNULL, ppNULL, st0212, st0213, st0214, st0215, 
        st0216, st0217, st0218, ppNULL, ppNULL, ppNULL, ppNULL, st0223, 
        st0224, st0225, ppNULL, ppNULL, ppNULL, ppNULL, st0230, ppNULL, 
        ppNULL, ppNULL, st0234, st0235, st0236, st0237, st0238, st0239, 
        st0240, st0241, st0242, st0243, st0244, st0245, ppNULL, ppNULL, 
        ppNULL, ppNULL, st0250, st0251, st0252, st0253, st0254, st0255, 
        st0256, st0257, st0258, ppNULL, st0260, st0261, st0262, ppNULL, 
        st0264, st0265, ppNULL, ppNULL, st0268, ppNULL, st0270, st0271, 
        ppNULL, ppNULL, st0274, ppNULL, ppNULL, ppNULL, st0278, st0279, 
        st0280, st0281, st0282, st0283, ppNULL, ppNULL, st0286, ppNULL, 
        ppNULL, st0289, st0290, st0291, ppNULL, ppNULL, 
    };

    static const unsigned long rd0000 [] =
    {
        0x00000001, 0x00000111, 
    };

    static const unsigned rp0000 [] =
    {
        0x0003, 0x0002, 0x0000, 
    };

    static const unsigned long rd0001 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0001 [] =
    {
        0x0000, 0x0000, 0x0001, 
    };

    static const unsigned long rd0004 [] =
    {
        0x00000001, 0x00000111, 
    };

    static const unsigned rp0004 [] =
    {
        0x0002, 0x0002, 0x0001, 
    };

    static const unsigned long rd0005 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0005 [] =
    {
        0x0010, 0x0009, 0x0001, 
    };

    static const unsigned long rd0006 [] =
    {
        0x00000014, 0x0000005b, 0x00000108, 0x0000010a, 0x0000010b, 
        0x00000112, 0x00010111, 0x00040111, 0x00050111, 0x00070111, 
        0x000b0111, 0x000c0111, 0x000d0111, 0x000e0111, 0x000f0111, 
        0x00120111, 0x00130111, 0x001b0111, 0x7fff0117, 0x7fff0118, 
        0x7fff0123, 
    };

    static const unsigned rp0006 [] =
    {
        0x0008, 0x0004, 0x0000, 0x0008, 0x0004, 0x0000, 0x0008, 0x0004, 
        0x0000, 0x0008, 0x0004, 0x0000, 0x0008, 0x0004, 0x0000, 0x0008, 
        0x0004, 0x0000, 0x0008, 0x0004, 0x0000, 0x0008, 0x0004, 0x0000, 
        0x0008, 0x0004, 0x0000, 0x0008, 0x0004, 0x0000, 0x0008, 0x0004, 
        0x0000, 0x0008, 0x0004, 0x0000, 0x0008, 0x0004, 0x0000, 0x0008, 
        0x0004, 0x0000, 0x0008, 0x0004, 0x0000, 0x0008, 0x0004, 0x0000, 
        0x0008, 0x0004, 0x0000, 0x0008, 0x0004, 0x0000, 0x0008, 0x0004, 
        0x0000, 0x0008, 0x0004, 0x0000, 
    };

    static const unsigned long rd0009 [] =
    {
        0x00000001, 0x00000101, 
    };

    static const unsigned rp0009 [] =
    {
        0x0004, 0x0001, 0x0002, 
    };

    static const unsigned long rd0010 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0010 [] =
    {
        0x0026, 0x0012, 0x0001, 
    };

    static const unsigned long rd0012 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0012 [] =
    {
        0x0028, 0x0013, 0x0001, 
    };

    static const unsigned long rd0014 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0014 [] =
    {
        0x002a, 0x0014, 0x0001, 
    };

    static const unsigned long rd0015 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0015 [] =
    {
        0x002b, 0x0014, 0x0001, 
    };

    static const unsigned long rd0016 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0016 [] =
    {
        0x002c, 0x0014, 0x0001, 
    };

    static const unsigned long rd0017 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0017 [] =
    {
        0x002d, 0x0014, 0x0001, 
    };

    static const unsigned long rd0018 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0018 [] =
    {
        0x002e, 0x0014, 0x0001, 
    };

    static const unsigned long rd0019 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0019 [] =
    {
        0x002f, 0x0014, 0x0001, 
    };

    static const unsigned long rd0020 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0020 [] =
    {
        0x0030, 0x0014, 0x0001, 
    };

    static const unsigned long rd0021 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0021 [] =
    {
        0x0032, 0x0014, 0x0001, 
    };

    static const unsigned long rd0022 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0022 [] =
    {
        0x0033, 0x0014, 0x0001, 
    };

    static const unsigned long rd0023 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0023 [] =
    {
        0x0034, 0x0014, 0x0001, 
    };

    static const unsigned long rd0024 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0024 [] =
    {
        0x0035, 0x0014, 0x0001, 
    };

    static const unsigned long rd0025 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0025 [] =
    {
        0x0036, 0x0014, 0x0001, 
    };

    static const unsigned long rd0026 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0026 [] =
    {
        0x0037, 0x0014, 0x0001, 
    };

    static const unsigned long rd0027 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0027 [] =
    {
        0x0038, 0x0014, 0x0001, 
    };

    static const unsigned long rd0028 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0028 [] =
    {
        0x0039, 0x0014, 0x0001, 
    };

    static const unsigned long rd0029 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0029 [] =
    {
        0x003a, 0x0014, 0x0001, 
    };

    static const unsigned long rd0030 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0030 [] =
    {
        0x003b, 0x0014, 0x0001, 
    };

    static const unsigned long rd0031 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0031 [] =
    {
        0x003c, 0x0015, 0x0001, 
    };

    static const unsigned long rd0034 [] =
    {
        0x00000001, 0x7fff0117, 
    };

    static const unsigned rp0034 [] =
    {
        0x0052, 0x0026, 0x0001, 
    };

    static const unsigned long rd0045 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0045 [] =
    {
        0x005a, 0x002a, 0x0001, 
    };

    static const unsigned long rd0046 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0046 [] =
    {
        0x0053, 0x0027, 0x0001, 
    };

    static const unsigned long rd0047 [] =
    {
        0x00000003, 0x0000010b, 0x7fff0118, 0x7fff0123, 
    };

    static const unsigned rp0047 [] =
    {
        0x005d, 0x002c, 0x0001, 0x005b, 0x002b, 0x0001, 0x005b, 0x002b, 
        0x0001, 
    };

    static const unsigned long rd0048 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0048 [] =
    {
        0x005e, 0x002d, 0x0001, 
    };

    static const unsigned long rd0049 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0049 [] =
    {
        0x005f, 0x002d, 0x0001, 
    };

    static const unsigned long rd0050 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0050 [] =
    {
        0x000f, 0x0009, 0x0002, 
    };

    static const unsigned long rd0051 [] =
    {
        0x00000013, 0x0000005b, 0x00000108, 0x0000010a, 0x0000010b, 
        0x00000112, 0x00040111, 0x00050111, 0x00070111, 0x000b0111, 
        0x000c0111, 0x000d0111, 0x000e0111, 0x000f0111, 0x00120111, 
        0x00130111, 0x001b0111, 0x7fff0117, 0x7fff0118, 0x7fff0123, 
    };

    static const unsigned rp0051 [] =
    {
        0x000a, 0x0005, 0x0000, 0x000a, 0x0005, 0x0000, 0x000a, 0x0005, 
        0x0000, 0x000a, 0x0005, 0x0000, 0x000a, 0x0005, 0x0000, 0x000a, 
        0x0005, 0x0000, 0x000a, 0x0005, 0x0000, 0x000a, 0x0005, 0x0000, 
        0x000a, 0x0005, 0x0000, 0x000a, 0x0005, 0x0000, 0x000a, 0x0005, 
        0x0000, 0x000a, 0x0005, 0x0000, 0x000a, 0x0005, 0x0000, 0x000a, 
        0x0005, 0x0000, 0x000a, 0x0005, 0x0000, 0x000a, 0x0005, 0x0000, 
        0x000a, 0x0005, 0x0000, 0x000a, 0x0005, 0x0000, 0x000a, 0x0005, 
        0x0000, 
    };

    static const unsigned long rd0053 [] =
    {
        0x00000014, 0x0000005b, 0x00000108, 0x0000010a, 0x00000111, 
        0x00000112, 0x00010111, 0x00040111, 0x00050111, 0x00070111, 
        0x000b0111, 0x000c0111, 0x000d0111, 0x000e0111, 0x000f0111, 
        0x00120111, 0x00130111, 0x001b0111, 0x7fff0117, 0x7fff0118, 
        0x7fff0123, 
    };

    static const unsigned rp0053 [] =
    {
        0x0006, 0x0003, 0x0002, 0x0006, 0x0003, 0x0002, 0x0006, 0x0003, 
        0x0002, 0x0006, 0x0003, 0x0002, 0x0006, 0x0003, 0x0002, 0x0006, 
        0x0003, 0x0002, 0x0006, 0x0003, 0x0002, 0x0006, 0x0003, 0x0002, 
        0x0006, 0x0003, 0x0002, 0x0006, 0x0003, 0x0002, 0x0006, 0x0003, 
        0x0002, 0x0006, 0x0003, 0x0002, 0x0006, 0x0003, 0x0002, 0x0006, 
        0x0003, 0x0002, 0x0006, 0x0003, 0x0002, 0x0006, 0x0003, 0x0002, 
        0x0006, 0x0003, 0x0002, 0x0006, 0x0003, 0x0002, 0x0006, 0x0003, 
        0x0002, 0x0006, 0x0003, 0x0002, 
    };

    static const unsigned long rd0054 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0054 [] =
    {
        0x0012, 0x000a, 0x0002, 
    };

    static const unsigned long rd0055 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0055 [] =
    {
        0x0013, 0x000b, 0x0001, 
    };

    static const unsigned long rd0056 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0056 [] =
    {
        0x0025, 0x0012, 0x0002, 
    };

    static const unsigned long rd0057 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0057 [] =
    {
        0x0027, 0x0013, 0x0002, 
    };

    static const unsigned long rd0058 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0058 [] =
    {
        0x0029, 0x0014, 0x0002, 
    };

    static const unsigned long rd0059 [] =
    {
        0x0000000f, 0x00000029, 0x0000002c, 0x0000003a, 0x0000005d, 
        0x00000105, 0x0000010b, 0x00000113, 0x00010113, 0x00020113, 
        0x00040111, 0x00060111, 0x00060113, 0x00100111, 0x00110111, 
        0x7fff0112, 
    };

    static const unsigned rp0059 [] =
    {
        0x00ba, 0x0043, 0x0001, 0x00ba, 0x0043, 0x0001, 0x00ba, 0x0043, 
        0x0001, 0x00ba, 0x0043, 0x0001, 0x00ba, 0x0043, 0x0001, 0x00ba, 
        0x0043, 0x0001, 0x00ba, 0x0043, 0x0001, 0x00ba, 0x0043, 0x0001, 
        0x00ba, 0x0043, 0x0001, 0x00ba, 0x0043, 0x0001, 0x00ba, 0x0043, 
        0x0001, 0x00ba, 0x0043, 0x0001, 0x00ba, 0x0043, 0x0001, 0x00ba, 
        0x0043, 0x0001, 0x00ba, 0x0043, 0x0001, 
    };

    static const unsigned long rd0060 [] =
    {
        0x00000012, 0x00000029, 0x0000002c, 0x0000003a, 0x0000005d, 
        0x00000105, 0x0000010b, 0x00000113, 0x00010113, 0x00020113, 
        0x00030118, 0x00040111, 0x00040118, 0x00050118, 0x00060111, 
        0x00060113, 0x00100111, 0x00110111, 0x7fff0112, 
    };

    static const unsigned rp0060 [] =
    {
        0x00b6, 0x0042, 0x0001, 0x00b6, 0x0042, 0x0001, 0x00b6, 0x0042, 
        0x0001, 0x00b6, 0x0042, 0x0001, 0x00b6, 0x0042, 0x0001, 0x00b6, 
        0x0042, 0x0001, 0x00b6, 0x0042, 0x0001, 0x00b6, 0x0042, 0x0001, 
        0x00b6, 0x0042, 0x0001, 0x00b6, 0x0042, 0x0001, 0x00b6, 0x0042, 
        0x0001, 0x00b6, 0x0042, 0x0001, 0x00b6, 0x0042, 0x0001, 0x00b6, 
        0x0042, 0x0001, 0x00b6, 0x0042, 0x0001, 0x00b6, 0x0042, 0x0001, 
        0x00b6, 0x0042, 0x0001, 0x00b6, 0x0042, 0x0001, 
    };

    static const unsigned long rd0061 [] =
    {
        0x00000014, 0x00000029, 0x0000002c, 0x0000003a, 0x0000005d, 
        0x00000105, 0x0000010b, 0x00000113, 0x00000118, 0x00010113, 
        0x00010118, 0x00020113, 0x00030118, 0x00040111, 0x00040118, 
        0x00050118, 0x00060111, 0x00060113, 0x00100111, 0x00110111, 
        0x7fff0112, 
    };

    static const unsigned rp0061 [] =
    {
        0x00b3, 0x0041, 0x0001, 0x00b3, 0x0041, 0x0001, 0x00b3, 0x0041, 
        0x0001, 0x00b3, 0x0041, 0x0001, 0x00b3, 0x0041, 0x0001, 0x00b3, 
        0x0041, 0x0001, 0x00b3, 0x0041, 0x0001, 0x00b3, 0x0041, 0x0001, 
        0x00b3, 0x0041, 0x0001, 0x00b3, 0x0041, 0x0001, 0x00b3, 0x0041, 
        0x0001, 0x00b3, 0x0041, 0x0001, 0x00b3, 0x0041, 0x0001, 0x00b3, 
        0x0041, 0x0001, 0x00b3, 0x0041, 0x0001, 0x00b3, 0x0041, 0x0001, 
        0x00b3, 0x0041, 0x0001, 0x00b3, 0x0041, 0x0001, 0x00b3, 0x0041, 
        0x0001, 0x00b3, 0x0041, 0x0001, 
    };

    static const unsigned long rd0062 [] =
    {
        0x00000016, 0x00000029, 0x0000002b, 0x0000002c, 0x0000002d, 
        0x0000003a, 0x0000005d, 0x00000105, 0x0000010b, 0x00000113, 
        0x00000118, 0x00010113, 0x00010118, 0x00020113, 0x00030118, 
        0x00040111, 0x00040118, 0x00050118, 0x00060111, 0x00060113, 
        0x00100111, 0x00110111, 0x7fff0112, 
    };

    static const unsigned rp0062 [] =
    {
        0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 
        0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 
        0x0040, 0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 0x0001, 
        0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 
        0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 
        0x0040, 0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 0x0001, 
        0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 
        0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 0x0040, 0x0001, 0x00b0, 
        0x0040, 0x0001, 
    };

    static const unsigned long rd0063 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0063 [] =
    {
        0x00ac, 0x003f, 0x0001, 
    };

    static const unsigned long rd0064 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0064 [] =
    {
        0x00a8, 0x003e, 0x0001, 
    };

    static const unsigned long rd0069 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0069 [] =
    {
        0x0097, 0x003d, 0x0001, 
    };

    static const unsigned long rd0070 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0070 [] =
    {
        0x0098, 0x003d, 0x0001, 
    };

    static const unsigned long rd0071 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0071 [] =
    {
        0x0099, 0x003d, 0x0001, 
    };

    static const unsigned long rd0072 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0072 [] =
    {
        0x009a, 0x003d, 0x0001, 
    };

    static const unsigned long rd0073 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0073 [] =
    {
        0x009b, 0x003d, 0x0001, 
    };

    static const unsigned long rd0074 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0074 [] =
    {
        0x009c, 0x003d, 0x0001, 
    };

    static const unsigned long rd0075 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0075 [] =
    {
        0x009d, 0x003d, 0x0001, 
    };

    static const unsigned long rd0081 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0081 [] =
    {
        0x00a3, 0x003d, 0x0001, 
    };

    static const unsigned long rd0082 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0082 [] =
    {
        0x00a4, 0x003d, 0x0001, 
    };

    static const unsigned long rd0083 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0083 [] =
    {
        0x0031, 0x0014, 0x0002, 
    };

    static const unsigned long rd0084 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0084 [] =
    {
        0x003e, 0x0017, 0x0002, 
    };

    static const unsigned long rd0087 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0087 [] =
    {
        0x0049, 0x001f, 0x0002, 
    };

    static const unsigned long rd0088 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0088 [] =
    {
        0x0047, 0x001e, 0x0002, 
    };

    static const unsigned long rd0090 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0090 [] =
    {
        0x004a, 0x0020, 0x0002, 
    };

    static const unsigned long rd0091 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0091 [] =
    {
        0x004c, 0x0021, 0x0002, 
    };

    static const unsigned long rd0092 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0092 [] =
    {
        0x004d, 0x0022, 0x0002, 
    };

    static const unsigned long rd0093 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0093 [] =
    {
        0x004e, 0x0023, 0x0002, 
    };

    static const unsigned long rd0094 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0094 [] =
    {
        0x004f, 0x0024, 0x0002, 
    };

    static const unsigned long rd0095 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0095 [] =
    {
        0x003f, 0x0018, 0x0002, 
    };

    static const unsigned long rd0096 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0096 [] =
    {
        0x0041, 0x0019, 0x0001, 
    };

    static const unsigned long rd0097 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0097 [] =
    {
        0x0042, 0x001a, 0x0002, 
    };

    static const unsigned long rd0098 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0098 [] =
    {
        0x0044, 0x001b, 0x0001, 
    };

    static const unsigned long rd0100 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0100 [] =
    {
        0x0051, 0x0025, 0x0002, 
    };

    static const unsigned long rd0101 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0101 [] =
    {
        0x0055, 0x0028, 0x0001, 
    };

    static const unsigned long rd0102 [] =
    {
        0x00000002, 0x0000002c, 0x0000010b, 
    };

    static const unsigned rp0102 [] =
    {
        0x0058, 0x0029, 0x0001, 0x0058, 0x0029, 0x0001, 
    };

    static const unsigned long rd0103 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0103 [] =
    {
        0x0057, 0x0029, 0x0001, 
    };

    static const unsigned long rd0104 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0104 [] =
    {
        0x0059, 0x002a, 0x0002, 
    };

    static const unsigned long rd0105 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0105 [] =
    {
        0x005d, 0x002c, 0x0001, 
    };

    static const unsigned long rd0106 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0106 [] =
    {
        0x005c, 0x002c, 0x0002, 
    };

    static const unsigned long rd0107 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0107 [] =
    {
        0x0061, 0x002e, 0x0001, 
    };

    static const unsigned long rd0108 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0108 [] =
    {
        0x0062, 0x002f, 0x0001, 
    };

    static const unsigned long rd0109 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0109 [] =
    {
        0x0063, 0x002f, 0x0001, 
    };

    static const unsigned long rd0110 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0110 [] =
    {
        0x0064, 0x002f, 0x0001, 
    };

    static const unsigned long rd0111 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0111 [] =
    {
        0x0065, 0x002f, 0x0001, 
    };

    static const unsigned long rd0112 [] =
    {
        0x00000002, 0x0000002c, 0x0000010b, 
    };

    static const unsigned rp0112 [] =
    {
        0x0069, 0x0031, 0x0001, 0x0069, 0x0031, 0x0001, 
    };

    static const unsigned long rd0115 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0115 [] =
    {
        0x0077, 0x0033, 0x0001, 
    };

    static const unsigned long rd0118 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0118 [] =
    {
        0x006a, 0x0032, 0x0001, 
    };

    static const unsigned long rd0119 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0119 [] =
    {
        0x006b, 0x0032, 0x0001, 
    };

    static const unsigned long rd0120 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0120 [] =
    {
        0x006c, 0x0032, 0x0001, 
    };

    static const unsigned long rd0121 [] =
    {
        0x00000002, 0x0000002c, 0x0000010b, 
    };

    static const unsigned rp0121 [] =
    {
        0x006d, 0x0032, 0x0001, 0x006d, 0x0032, 0x0001, 
    };

    static const unsigned long rd0122 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0122 [] =
    {
        0x006e, 0x0032, 0x0001, 
    };

    static const unsigned long rd0123 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0123 [] =
    {
        0x006f, 0x0032, 0x0001, 
    };

    static const unsigned long rd0124 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0124 [] =
    {
        0x0070, 0x0032, 0x0001, 
    };

    static const unsigned long rd0125 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0125 [] =
    {
        0x0071, 0x0032, 0x0001, 
    };

    static const unsigned long rd0126 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0126 [] =
    {
        0x0072, 0x0032, 0x0001, 
    };

    static const unsigned long rd0127 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0127 [] =
    {
        0x0073, 0x0032, 0x0001, 
    };

    static const unsigned long rd0128 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0128 [] =
    {
        0x0078, 0x0034, 0x0001, 
    };

    static const unsigned long rd0129 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0129 [] =
    {
        0x0079, 0x0034, 0x0001, 
    };

    static const unsigned long rd0131 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0131 [] =
    {
        0x007d, 0x0036, 0x0001, 
    };

    static const unsigned long rd0132 [] =
    {
        0x00000002, 0x0000005b, 0x7fff011c, 
    };

    static const unsigned rp0132 [] =
    {
        0x007c, 0x0035, 0x0001, 0x007c, 0x0035, 0x0001, 
    };

    static const unsigned long rd0133 [] =
    {
        0x00000011, 0x0000005b, 0x00000108, 0x0000010a, 0x0000010b, 
        0x00000112, 0x00040111, 0x00050111, 0x00070111, 0x000b0111, 
        0x000c0111, 0x000d0111, 0x000e0111, 0x000f0111, 0x001b0111, 
        0x7fff0117, 0x7fff0118, 0x7fff0123, 
    };

    static const unsigned rp0133 [] =
    {
        0x0015, 0x000d, 0x0000, 0x0015, 0x000d, 0x0000, 0x0015, 0x000d, 
        0x0000, 0x0015, 0x000d, 0x0000, 0x0015, 0x000d, 0x0000, 0x0015, 
        0x000d, 0x0000, 0x0015, 0x000d, 0x0000, 0x0015, 0x000d, 0x0000, 
        0x0015, 0x000d, 0x0000, 0x0015, 0x000d, 0x0000, 0x0015, 0x000d, 
        0x0000, 0x0015, 0x000d, 0x0000, 0x0015, 0x000d, 0x0000, 0x0015, 
        0x000d, 0x0000, 0x0015, 0x000d, 0x0000, 0x0015, 0x000d, 0x0000, 
        0x0015, 0x000d, 0x0000, 
    };

    static const unsigned long rd0136 [] =
    {
        0x00000014, 0x0000005b, 0x00000108, 0x0000010a, 0x00000111, 
        0x00000112, 0x00010111, 0x00040111, 0x00050111, 0x00070111, 
        0x000b0111, 0x000c0111, 0x000d0111, 0x000e0111, 0x000f0111, 
        0x00120111, 0x00130111, 0x001b0111, 0x7fff0117, 0x7fff0118, 
        0x7fff0123, 
    };

    static const unsigned rp0136 [] =
    {
        0x0005, 0x0003, 0x0003, 0x0005, 0x0003, 0x0003, 0x0005, 0x0003, 
        0x0003, 0x0005, 0x0003, 0x0003, 0x0005, 0x0003, 0x0003, 0x0005, 
        0x0003, 0x0003, 0x0005, 0x0003, 0x0003, 0x0005, 0x0003, 0x0003, 
        0x0005, 0x0003, 0x0003, 0x0005, 0x0003, 0x0003, 0x0005, 0x0003, 
        0x0003, 0x0005, 0x0003, 0x0003, 0x0005, 0x0003, 0x0003, 0x0005, 
        0x0003, 0x0003, 0x0005, 0x0003, 0x0003, 0x0005, 0x0003, 0x0003, 
        0x0005, 0x0003, 0x0003, 0x0005, 0x0003, 0x0003, 0x0005, 0x0003, 
        0x0003, 0x0005, 0x0003, 0x0003, 
    };

    static const unsigned long rd0137 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0137 [] =
    {
        0x0011, 0x000a, 0x0003, 
    };

    static const unsigned long rd0138 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0138 [] =
    {
        0x0017, 0x000e, 0x0001, 
    };

    static const unsigned long rd0139 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0139 [] =
    {
        0x0018, 0x000f, 0x0001, 
    };

    static const unsigned long rd0140 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0140 [] =
    {
        0x0019, 0x000f, 0x0001, 
    };

    static const unsigned long rd0142 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0142 [] =
    {
        0x001b, 0x000f, 0x0001, 
    };

    static const unsigned long rd0143 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0143 [] =
    {
        0x001c, 0x000f, 0x0001, 
    };

    static const unsigned long rd0144 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0144 [] =
    {
        0x001d, 0x000f, 0x0001, 
    };

    static const unsigned long rd0145 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0145 [] =
    {
        0x001e, 0x000f, 0x0001, 
    };

    static const unsigned long rd0146 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0146 [] =
    {
        0x001f, 0x000f, 0x0001, 
    };

    static const unsigned long rd0147 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0147 [] =
    {
        0x0020, 0x000f, 0x0001, 
    };

    static const unsigned long rd0158 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0158 [] =
    {
        0x00a9, 0x003e, 0x0002, 
    };

    static const unsigned long rd0159 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0159 [] =
    {
        0x00aa, 0x003e, 0x0002, 
    };

    static const unsigned long rd0160 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0160 [] =
    {
        0x00ab, 0x003e, 0x0002, 
    };

    static const unsigned long rd0165 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0165 [] =
    {
        0x009e, 0x003d, 0x0002, 
    };

    static const unsigned long rd0166 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0166 [] =
    {
        0x009f, 0x003d, 0x0002, 
    };

    static const unsigned long rd0167 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0167 [] =
    {
        0x00a0, 0x003d, 0x0002, 
    };

    static const unsigned long rd0168 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0168 [] =
    {
        0x00a1, 0x003d, 0x0002, 
    };

    static const unsigned long rd0169 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0169 [] =
    {
        0x00a2, 0x003d, 0x0002, 
    };

    static const unsigned long rd0170 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0170 [] =
    {
        0x0045, 0x001c, 0x0003, 
    };

    static const unsigned long rd0171 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0171 [] =
    {
        0x0046, 0x001d, 0x0003, 
    };

    static const unsigned long rd0172 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0172 [] =
    {
        0x0048, 0x001f, 0x0003, 
    };

    static const unsigned long rd0173 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0173 [] =
    {
        0x003d, 0x0016, 0x0003, 
    };

    static const unsigned long rd0177 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0177 [] =
    {
        0x0050, 0x0025, 0x0003, 
    };

    static const unsigned long rd0183 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0183 [] =
    {
        0x0075, 0x0033, 0x0002, 
    };

    static const unsigned long rd0185 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0185 [] =
    {
        0x0076, 0x0033, 0x0002, 
    };

    static const unsigned long rd0186 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0186 [] =
    {
        0x0067, 0x0031, 0x0002, 
    };

    static const unsigned long rd0187 [] =
    {
        0x00000002, 0x0000005b, 0x7fff011c, 
    };

    static const unsigned rp0187 [] =
    {
        0x007a, 0x0035, 0x0002, 0x007a, 0x0035, 0x0002, 
    };

    static const unsigned long rd0188 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0188 [] =
    {
        0x0068, 0x0031, 0x0002, 
    };

    static const unsigned long rd0189 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0189 [] =
    {
        0x007e, 0x0037, 0x0002, 
    };

    static const unsigned long rd0190 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned rp0190 [] =
    {
        0x0093, 0x003b, 0x0001, 
    };

    static const unsigned long rd0193 [] =
    {
        0x00000003, 0x0000002b, 0x0000002d, 0x0000005d, 
    };

    static const unsigned rp0193 [] =
    {
        0x008d, 0x0039, 0x0001, 0x008d, 0x0039, 0x0001, 0x008d, 0x0039, 
        0x0001, 
    };

    static const unsigned long rd0194 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0194 [] =
    {
        0x008e, 0x0039, 0x0001, 
    };

    static const unsigned long rd0195 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0195 [] =
    {
        0x008f, 0x0039, 0x0001, 
    };

    static const unsigned long rd0196 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0196 [] =
    {
        0x0090, 0x0039, 0x0001, 
    };

    static const unsigned long rd0197 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0197 [] =
    {
        0x0091, 0x0039, 0x0001, 
    };

    static const unsigned long rd0198 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0198 [] =
    {
        0x0094, 0x003b, 0x0001, 
    };

    static const unsigned long rd0199 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0199 [] =
    {
        0x0092, 0x003a, 0x0001, 
    };

    static const unsigned long rd0200 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0200 [] =
    {
        0x007b, 0x0035, 0x0002, 
    };

    static const unsigned long rd0203 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0203 [] =
    {
        0x000b, 0x0006, 0x0001, 
    };

    static const unsigned long rd0204 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0204 [] =
    {
        0x000c, 0x0006, 0x0001, 
    };

    static const unsigned long rd0207 [] =
    {
        0x00000013, 0x0000005b, 0x00000108, 0x0000010a, 0x00000112, 
        0x00010111, 0x00040111, 0x00050111, 0x00070111, 0x000b0111, 
        0x000c0111, 0x000d0111, 0x000e0111, 0x000f0111, 0x00120111, 
        0x00130111, 0x001b0111, 0x7fff0117, 0x7fff0118, 0x7fff0123, 
    };

    static const unsigned rp0207 [] =
    {
        0x0007, 0x0004, 0x0003, 0x0007, 0x0004, 0x0003, 0x0007, 0x0004, 
        0x0003, 0x0007, 0x0004, 0x0003, 0x0007, 0x0004, 0x0003, 0x0007, 
        0x0004, 0x0003, 0x0007, 0x0004, 0x0003, 0x0007, 0x0004, 0x0003, 
        0x0007, 0x0004, 0x0003, 0x0007, 0x0004, 0x0003, 0x0007, 0x0004, 
        0x0003, 0x0007, 0x0004, 0x0003, 0x0007, 0x0004, 0x0003, 0x0007, 
        0x0004, 0x0003, 0x0007, 0x0004, 0x0003, 0x0007, 0x0004, 0x0003, 
        0x0007, 0x0004, 0x0003, 0x0007, 0x0004, 0x0003, 0x0007, 0x0004, 
        0x0003, 
    };

    static const unsigned long rd0208 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0208 [] =
    {
        0x0022, 0x0010, 0x0002, 
    };

    static const unsigned long rd0209 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0209 [] =
    {
        0x0014, 0x000c, 0x0001, 
    };

    static const unsigned long rd0210 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0210 [] =
    {
        0x0016, 0x000e, 0x0002, 
    };

    static const unsigned long rd0211 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0211 [] =
    {
        0x001a, 0x000f, 0x0002, 
    };

    static const unsigned long rd0212 [] =
    {
        0x00000012, 0x00000029, 0x0000002c, 0x0000003a, 0x0000005d, 
        0x00000105, 0x0000010b, 0x00000113, 0x00010113, 0x00020113, 
        0x00030118, 0x00040111, 0x00040118, 0x00050118, 0x00060111, 
        0x00060113, 0x00100111, 0x00110111, 0x7fff0112, 
    };

    static const unsigned rp0212 [] =
    {
        0x00b7, 0x0042, 0x0003, 0x00b7, 0x0042, 0x0003, 0x00b7, 0x0042, 
        0x0003, 0x00b7, 0x0042, 0x0003, 0x00b7, 0x0042, 0x0003, 0x00b7, 
        0x0042, 0x0003, 0x00b7, 0x0042, 0x0003, 0x00b7, 0x0042, 0x0003, 
        0x00b7, 0x0042, 0x0003, 0x00b7, 0x0042, 0x0003, 0x00b7, 0x0042, 
        0x0003, 0x00b7, 0x0042, 0x0003, 0x00b7, 0x0042, 0x0003, 0x00b7, 
        0x0042, 0x0003, 0x00b7, 0x0042, 0x0003, 0x00b7, 0x0042, 0x0003, 
        0x00b7, 0x0042, 0x0003, 0x00b7, 0x0042, 0x0003, 
    };

    static const unsigned long rd0213 [] =
    {
        0x00000012, 0x00000029, 0x0000002c, 0x0000003a, 0x0000005d, 
        0x00000105, 0x0000010b, 0x00000113, 0x00010113, 0x00020113, 
        0x00030118, 0x00040111, 0x00040118, 0x00050118, 0x00060111, 
        0x00060113, 0x00100111, 0x00110111, 0x7fff0112, 
    };

    static const unsigned rp0213 [] =
    {
        0x00b8, 0x0042, 0x0003, 0x00b8, 0x0042, 0x0003, 0x00b8, 0x0042, 
        0x0003, 0x00b8, 0x0042, 0x0003, 0x00b8, 0x0042, 0x0003, 0x00b8, 
        0x0042, 0x0003, 0x00b8, 0x0042, 0x0003, 0x00b8, 0x0042, 0x0003, 
        0x00b8, 0x0042, 0x0003, 0x00b8, 0x0042, 0x0003, 0x00b8, 0x0042, 
        0x0003, 0x00b8, 0x0042, 0x0003, 0x00b8, 0x0042, 0x0003, 0x00b8, 
        0x0042, 0x0003, 0x00b8, 0x0042, 0x0003, 0x00b8, 0x0042, 0x0003, 
        0x00b8, 0x0042, 0x0003, 0x00b8, 0x0042, 0x0003, 
    };

    static const unsigned long rd0214 [] =
    {
        0x00000012, 0x00000029, 0x0000002c, 0x0000003a, 0x0000005d, 
        0x00000105, 0x0000010b, 0x00000113, 0x00010113, 0x00020113, 
        0x00030118, 0x00040111, 0x00040118, 0x00050118, 0x00060111, 
        0x00060113, 0x00100111, 0x00110111, 0x7fff0112, 
    };

    static const unsigned rp0214 [] =
    {
        0x00b9, 0x0042, 0x0003, 0x00b9, 0x0042, 0x0003, 0x00b9, 0x0042, 
        0x0003, 0x00b9, 0x0042, 0x0003, 0x00b9, 0x0042, 0x0003, 0x00b9, 
        0x0042, 0x0003, 0x00b9, 0x0042, 0x0003, 0x00b9, 0x0042, 0x0003, 
        0x00b9, 0x0042, 0x0003, 0x00b9, 0x0042, 0x0003, 0x00b9, 0x0042, 
        0x0003, 0x00b9, 0x0042, 0x0003, 0x00b9, 0x0042, 0x0003, 0x00b9, 
        0x0042, 0x0003, 0x00b9, 0x0042, 0x0003, 0x00b9, 0x0042, 0x0003, 
        0x00b9, 0x0042, 0x0003, 0x00b9, 0x0042, 0x0003, 
    };

    static const unsigned long rd0215 [] =
    {
        0x00000014, 0x00000029, 0x0000002c, 0x0000003a, 0x0000005d, 
        0x00000105, 0x0000010b, 0x00000113, 0x00000118, 0x00010113, 
        0x00010118, 0x00020113, 0x00030118, 0x00040111, 0x00040118, 
        0x00050118, 0x00060111, 0x00060113, 0x00100111, 0x00110111, 
        0x7fff0112, 
    };

    static const unsigned rp0215 [] =
    {
        0x00b4, 0x0041, 0x0003, 0x00b4, 0x0041, 0x0003, 0x00b4, 0x0041, 
        0x0003, 0x00b4, 0x0041, 0x0003, 0x00b4, 0x0041, 0x0003, 0x00b4, 
        0x0041, 0x0003, 0x00b4, 0x0041, 0x0003, 0x00b4, 0x0041, 0x0003, 
        0x00b4, 0x0041, 0x0003, 0x00b4, 0x0041, 0x0003, 0x00b4, 0x0041, 
        0x0003, 0x00b4, 0x0041, 0x0003, 0x00b4, 0x0041, 0x0003, 0x00b4, 
        0x0041, 0x0003, 0x00b4, 0x0041, 0x0003, 0x00b4, 0x0041, 0x0003, 
        0x00b4, 0x0041, 0x0003, 0x00b4, 0x0041, 0x0003, 0x00b4, 0x0041, 
        0x0003, 0x00b4, 0x0041, 0x0003, 
    };

    static const unsigned long rd0216 [] =
    {
        0x00000014, 0x00000029, 0x0000002c, 0x0000003a, 0x0000005d, 
        0x00000105, 0x0000010b, 0x00000113, 0x00000118, 0x00010113, 
        0x00010118, 0x00020113, 0x00030118, 0x00040111, 0x00040118, 
        0x00050118, 0x00060111, 0x00060113, 0x00100111, 0x00110111, 
        0x7fff0112, 
    };

    static const unsigned rp0216 [] =
    {
        0x00b5, 0x0041, 0x0003, 0x00b5, 0x0041, 0x0003, 0x00b5, 0x0041, 
        0x0003, 0x00b5, 0x0041, 0x0003, 0x00b5, 0x0041, 0x0003, 0x00b5, 
        0x0041, 0x0003, 0x00b5, 0x0041, 0x0003, 0x00b5, 0x0041, 0x0003, 
        0x00b5, 0x0041, 0x0003, 0x00b5, 0x0041, 0x0003, 0x00b5, 0x0041, 
        0x0003, 0x00b5, 0x0041, 0x0003, 0x00b5, 0x0041, 0x0003, 0x00b5, 
        0x0041, 0x0003, 0x00b5, 0x0041, 0x0003, 0x00b5, 0x0041, 0x0003, 
        0x00b5, 0x0041, 0x0003, 0x00b5, 0x0041, 0x0003, 0x00b5, 0x0041, 
        0x0003, 0x00b5, 0x0041, 0x0003, 
    };

    static const unsigned long rd0217 [] =
    {
        0x00000016, 0x00000029, 0x0000002b, 0x0000002c, 0x0000002d, 
        0x0000003a, 0x0000005d, 0x00000105, 0x0000010b, 0x00000113, 
        0x00000118, 0x00010113, 0x00010118, 0x00020113, 0x00030118, 
        0x00040111, 0x00040118, 0x00050118, 0x00060111, 0x00060113, 
        0x00100111, 0x00110111, 0x7fff0112, 
    };

    static const unsigned rp0217 [] =
    {
        0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 
        0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 
        0x0040, 0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 0x0003, 
        0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 
        0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 
        0x0040, 0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 0x0003, 
        0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 
        0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 0x0040, 0x0003, 0x00b1, 
        0x0040, 0x0003, 
    };

    static const unsigned long rd0218 [] =
    {
        0x00000016, 0x00000029, 0x0000002b, 0x0000002c, 0x0000002d, 
        0x0000003a, 0x0000005d, 0x00000105, 0x0000010b, 0x00000113, 
        0x00000118, 0x00010113, 0x00010118, 0x00020113, 0x00030118, 
        0x00040111, 0x00040118, 0x00050118, 0x00060111, 0x00060113, 
        0x00100111, 0x00110111, 0x7fff0112, 
    };

    static const unsigned rp0218 [] =
    {
        0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 
        0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 
        0x0040, 0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 0x0003, 
        0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 
        0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 
        0x0040, 0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 0x0003, 
        0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 
        0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 0x0040, 0x0003, 0x00b2, 
        0x0040, 0x0003, 
    };

    static const unsigned long rd0219 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0219 [] =
    {
        0x00ad, 0x003f, 0x0003, 
    };

    static const unsigned long rd0220 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0220 [] =
    {
        0x00ae, 0x003f, 0x0003, 
    };

    static const unsigned long rd0221 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0221 [] =
    {
        0x00af, 0x003f, 0x0003, 
    };

    static const unsigned long rd0222 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0222 [] =
    {
        0x0096, 0x003d, 0x0003, 
    };

    static const unsigned long rd0226 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0226 [] =
    {
        0x004b, 0x0021, 0x0004, 
    };

    static const unsigned long rd0227 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0227 [] =
    {
        0x0040, 0x0019, 0x0003, 
    };

    static const unsigned long rd0228 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0228 [] =
    {
        0x0043, 0x001b, 0x0003, 
    };

    static const unsigned long rd0229 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0229 [] =
    {
        0x0054, 0x0028, 0x0003, 
    };

    static const unsigned long rd0231 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0231 [] =
    {
        0x0060, 0x002e, 0x0003, 
    };

    static const unsigned long rd0232 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0232 [] =
    {
        0x0066, 0x0030, 0x0003, 
    };

    static const unsigned long rd0233 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0233 [] =
    {
        0x0074, 0x0033, 0x0003, 
    };

    static const unsigned long rd0236 [] =
    {
        0x00000002, 0x0000002c, 0x0000010b, 
    };

    static const unsigned rp0236 [] =
    {
        0x008b, 0x0038, 0x0003, 0x008b, 0x0038, 0x0003, 
    };

    static const unsigned long rd0239 [] =
    {
        0x00000002, 0x0000002c, 0x0000010b, 
    };

    static const unsigned rp0239 [] =
    {
        0x008c, 0x0038, 0x0003, 0x008c, 0x0038, 0x0003, 
    };

    static const unsigned long rd0241 [] =
    {
        0x00000001, 0x00000101, 
    };

    static const unsigned rp0241 [] =
    {
        0x0001, 0x0001, 0x0006, 
    };

    static const unsigned long rd0242 [] =
    {
        0x00000012, 0x0000005b, 0x00000108, 0x0000010a, 0x00000112, 
        0x00040111, 0x00050111, 0x00070111, 0x000b0111, 0x000c0111, 
        0x000d0111, 0x000e0111, 0x000f0111, 0x00120111, 0x00130111, 
        0x001b0111, 0x7fff0117, 0x7fff0118, 0x7fff0123, 
    };

    static const unsigned rp0242 [] =
    {
        0x0009, 0x0005, 0x0003, 0x0009, 0x0005, 0x0003, 0x0009, 0x0005, 
        0x0003, 0x0009, 0x0005, 0x0003, 0x0009, 0x0005, 0x0003, 0x0009, 
        0x0005, 0x0003, 0x0009, 0x0005, 0x0003, 0x0009, 0x0005, 0x0003, 
        0x0009, 0x0005, 0x0003, 0x0009, 0x0005, 0x0003, 0x0009, 0x0005, 
        0x0003, 0x0009, 0x0005, 0x0003, 0x0009, 0x0005, 0x0003, 0x0009, 
        0x0005, 0x0003, 0x0009, 0x0005, 0x0003, 0x0009, 0x0005, 0x0003, 
        0x0009, 0x0005, 0x0003, 0x0009, 0x0005, 0x0003, 
    };

    static const unsigned long rd0245 [] =
    {
        0x00000001, 0x0000010b, 
    };

    static const unsigned rp0245 [] =
    {
        0x0021, 0x0010, 0x0003, 
    };

    static const unsigned long rd0246 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0246 [] =
    {
        0x0024, 0x0011, 0x0001, 
    };

    static const unsigned long rd0247 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0247 [] =
    {
        0x00a5, 0x003d, 0x0004, 
    };

    static const unsigned long rd0248 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0248 [] =
    {
        0x00a6, 0x003d, 0x0004, 
    };

    static const unsigned long rd0249 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0249 [] =
    {
        0x00a7, 0x003d, 0x0004, 
    };

    static const unsigned long rd0251 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned rp0251 [] =
    {
        0x0093, 0x003b, 0x0001, 
    };

    static const unsigned long rd0255 [] =
    {
        0x00000001, 0x0000005d, 
    };

    static const unsigned rp0255 [] =
    {
        0x0093, 0x003b, 0x0001, 
    };

    static const unsigned long rd0259 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0259 [] =
    {
        0x0095, 0x003c, 0x0003, 
    };

    static const unsigned long rd0263 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0263 [] =
    {
        0x0056, 0x0029, 0x0005, 
    };

    static const unsigned long rd0266 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0266 [] =
    {
        0x0081, 0x0038, 0x0005, 
    };

    static const unsigned long rd0267 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0267 [] =
    {
        0x0082, 0x0038, 0x0005, 
    };

    static const unsigned long rd0269 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0269 [] =
    {
        0x0093, 0x003b, 0x0001, 
    };

    static const unsigned long rd0272 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0272 [] =
    {
        0x0087, 0x0038, 0x0005, 
    };

    static const unsigned long rd0273 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0273 [] =
    {
        0x0088, 0x0038, 0x0005, 
    };

    static const unsigned long rd0275 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0275 [] =
    {
        0x000d, 0x0007, 0x0004, 
    };

    static const unsigned long rd0276 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0276 [] =
    {
        0x000e, 0x0008, 0x0004, 
    };

    static const unsigned long rd0277 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0277 [] =
    {
        0x0023, 0x0011, 0x0003, 
    };

    static const unsigned long rd0280 [] =
    {
        0x00000002, 0x0000002c, 0x0000010b, 
    };

    static const unsigned rp0280 [] =
    {
        0x0084, 0x0038, 0x0006, 0x0084, 0x0038, 0x0006, 
    };

    static const unsigned long rd0283 [] =
    {
        0x00000002, 0x0000002c, 0x0000010b, 
    };

    static const unsigned rp0283 [] =
    {
        0x008a, 0x0038, 0x0006, 0x008a, 0x0038, 0x0006, 
    };

    static const unsigned long rd0284 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0284 [] =
    {
        0x007f, 0x0038, 0x0007, 
    };

    static const unsigned long rd0285 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0285 [] =
    {
        0x0080, 0x0038, 0x0007, 
    };

    static const unsigned long rd0287 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0287 [] =
    {
        0x0085, 0x0038, 0x0007, 
    };

    static const unsigned long rd0288 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0288 [] =
    {
        0x0086, 0x0038, 0x0007, 
    };

    static const unsigned long rd0292 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0292 [] =
    {
        0x0083, 0x0038, 0x0009, 
    };

    static const unsigned long rd0293 [] =
    {
        0x00000001, 0x00000000, 
    };

    static const unsigned rp0293 [] =
    {
        0x0089, 0x0038, 0x0009, 
    };

    /* Reductions Table */
    static const unsigned long * const rdtbl [] =
    {
        rd0000, rd0001, ppNULL, ppNULL, rd0004, rd0005, rd0006, ppNULL, 
        ppNULL, rd0009, rd0010, ppNULL, rd0012, ppNULL, rd0014, rd0015, 
        rd0016, rd0017, rd0018, rd0019, rd0020, rd0021, rd0022, rd0023, 
        rd0024, rd0025, rd0026, rd0027, rd0028, rd0029, rd0030, rd0031, 
        ppNULL, ppNULL, rd0034, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rd0045, rd0046, rd0047, 
        rd0048, rd0049, rd0050, rd0051, ppNULL, rd0053, rd0054, rd0055, 
        rd0056, rd0057, rd0058, rd0059, rd0060, rd0061, rd0062, rd0063, 
        rd0064, ppNULL, ppNULL, ppNULL, ppNULL, rd0069, rd0070, rd0071, 
        rd0072, rd0073, rd0074, rd0075, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, rd0081, rd0082, rd0083, rd0084, ppNULL, ppNULL, rd0087, 
        rd0088, ppNULL, rd0090, rd0091, rd0092, rd0093, rd0094, rd0095, 
        rd0096, rd0097, rd0098, ppNULL, rd0100, rd0101, rd0102, rd0103, 
        rd0104, rd0105, rd0106, rd0107, rd0108, rd0109, rd0110, rd0111, 
        rd0112, ppNULL, ppNULL, rd0115, ppNULL, ppNULL, rd0118, rd0119, 
        rd0120, rd0121, rd0122, rd0123, rd0124, rd0125, rd0126, rd0127, 
        rd0128, rd0129, ppNULL, rd0131, rd0132, rd0133, ppNULL, ppNULL, 
        rd0136, rd0137, rd0138, rd0139, rd0140, ppNULL, rd0142, rd0143, 
        rd0144, rd0145, rd0146, rd0147, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rd0158, rd0159, 
        rd0160, ppNULL, ppNULL, ppNULL, ppNULL, rd0165, rd0166, rd0167, 
        rd0168, rd0169, rd0170, rd0171, rd0172, rd0173, ppNULL, ppNULL, 
        ppNULL, rd0177, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rd0183, 
        ppNULL, rd0185, rd0186, rd0187, rd0188, rd0189, rd0190, ppNULL, 
        ppNULL, rd0193, rd0194, rd0195, rd0196, rd0197, rd0198, rd0199, 
        rd0200, ppNULL, ppNULL, rd0203, rd0204, ppNULL, ppNULL, rd0207, 
        rd0208, rd0209, rd0210, rd0211, rd0212, rd0213, rd0214, rd0215, 
        rd0216, rd0217, rd0218, rd0219, rd0220, rd0221, rd0222, ppNULL, 
        ppNULL, ppNULL, rd0226, rd0227, rd0228, rd0229, ppNULL, rd0231, 
        rd0232, rd0233, ppNULL, ppNULL, rd0236, ppNULL, ppNULL, rd0239, 
        ppNULL, rd0241, rd0242, ppNULL, ppNULL, rd0245, rd0246, rd0247, 
        rd0248, rd0249, ppNULL, rd0251, ppNULL, ppNULL, ppNULL, rd0255, 
        ppNULL, ppNULL, ppNULL, rd0259, ppNULL, ppNULL, ppNULL, rd0263, 
        ppNULL, ppNULL, rd0266, rd0267, ppNULL, rd0269, ppNULL, ppNULL, 
        rd0272, rd0273, ppNULL, rd0275, rd0276, rd0277, ppNULL, ppNULL, 
        rd0280, ppNULL, ppNULL, rd0283, rd0284, rd0285, ppNULL, rd0287, 
        rd0288, ppNULL, ppNULL, ppNULL, rd0292, rd0293, 
    };

    /* Reductions Parameters Table */
    static const unsigned * const rptbl [] =
    {
        rp0000, rp0001, ppNULL, ppNULL, rp0004, rp0005, rp0006, ppNULL, 
        ppNULL, rp0009, rp0010, ppNULL, rp0012, ppNULL, rp0014, rp0015, 
        rp0016, rp0017, rp0018, rp0019, rp0020, rp0021, rp0022, rp0023, 
        rp0024, rp0025, rp0026, rp0027, rp0028, rp0029, rp0030, rp0031, 
        ppNULL, ppNULL, rp0034, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rp0045, rp0046, rp0047, 
        rp0048, rp0049, rp0050, rp0051, ppNULL, rp0053, rp0054, rp0055, 
        rp0056, rp0057, rp0058, rp0059, rp0060, rp0061, rp0062, rp0063, 
        rp0064, ppNULL, ppNULL, ppNULL, ppNULL, rp0069, rp0070, rp0071, 
        rp0072, rp0073, rp0074, rp0075, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, rp0081, rp0082, rp0083, rp0084, ppNULL, ppNULL, rp0087, 
        rp0088, ppNULL, rp0090, rp0091, rp0092, rp0093, rp0094, rp0095, 
        rp0096, rp0097, rp0098, ppNULL, rp0100, rp0101, rp0102, rp0103, 
        rp0104, rp0105, rp0106, rp0107, rp0108, rp0109, rp0110, rp0111, 
        rp0112, ppNULL, ppNULL, rp0115, ppNULL, ppNULL, rp0118, rp0119, 
        rp0120, rp0121, rp0122, rp0123, rp0124, rp0125, rp0126, rp0127, 
        rp0128, rp0129, ppNULL, rp0131, rp0132, rp0133, ppNULL, ppNULL, 
        rp0136, rp0137, rp0138, rp0139, rp0140, ppNULL, rp0142, rp0143, 
        rp0144, rp0145, rp0146, rp0147, ppNULL, ppNULL, ppNULL, ppNULL, 
        ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rp0158, rp0159, 
        rp0160, ppNULL, ppNULL, ppNULL, ppNULL, rp0165, rp0166, rp0167, 
        rp0168, rp0169, rp0170, rp0171, rp0172, rp0173, ppNULL, ppNULL, 
        ppNULL, rp0177, ppNULL, ppNULL, ppNULL, ppNULL, ppNULL, rp0183, 
        ppNULL, rp0185, rp0186, rp0187, rp0188, rp0189, rp0190, ppNULL, 
        ppNULL, rp0193, rp0194, rp0195, rp0196, rp0197, rp0198, rp0199, 
        rp0200, ppNULL, ppNULL, rp0203, rp0204, ppNULL, ppNULL, rp0207, 
        rp0208, rp0209, rp0210, rp0211, rp0212, rp0213, rp0214, rp0215, 
        rp0216, rp0217, rp0218, rp0219, rp0220, rp0221, rp0222, ppNULL, 
        ppNULL, ppNULL, rp0226, rp0227, rp0228, rp0229, ppNULL, rp0231, 
        rp0232, rp0233, ppNULL, ppNULL, rp0236, ppNULL, ppNULL, rp0239, 
        ppNULL, rp0241, rp0242, ppNULL, ppNULL, rp0245, rp0246, rp0247, 
        rp0248, rp0249, ppNULL, rp0251, ppNULL, ppNULL, ppNULL, rp0255, 
        ppNULL, ppNULL, ppNULL, rp0259, ppNULL, ppNULL, ppNULL, rp0263, 
        ppNULL, ppNULL, rp0266, rp0267, ppNULL, rp0269, ppNULL, ppNULL, 
        rp0272, rp0273, ppNULL, rp0275, rp0276, rp0277, ppNULL, ppNULL, 
        rp0280, ppNULL, ppNULL, rp0283, rp0284, rp0285, ppNULL, rp0287, 
        rp0288, ppNULL, ppNULL, ppNULL, rp0292, rp0293, 
    };

    /* Nonterminals Table */
    static const unsigned nttbl [] =
    {
        0x0000, 0x0001, 0x0002, 0x0005, 0x0007, 0x0009, 0x000b, 0x000d, 
        0x000e, 0x000f, 0x0011, 0x0013, 0x0014, 0x0015, 0x0016, 0x0018, 
        0x0021, 0x0023, 0x0025, 0x0027, 0x0029, 0x003c, 0x003d, 0x003e, 
        0x003f, 0x0040, 0x0042, 0x0043, 0x0045, 0x0046, 0x0047, 0x0048, 
        0x004a, 0x004b, 0x004d, 0x004e, 0x004f, 0x0050, 0x0052, 0x0053, 
        0x0054, 0x0056, 0x0059, 0x005b, 0x005c, 0x005e, 0x0060, 0x0062, 
        0x0066, 0x0067, 0x006a, 0x0074, 0x0078, 0x007a, 0x007d, 0x007e, 
        0x007f, 0x008d, 0x0092, 0x0093, 0x0095, 0x0096, 0x00a8, 0x00ac, 
        0x00b0, 0x00b3, 0x00b6, 0x00ba, 
    };

    void __export_statement (unsigned, int, char *[], unsigned []);
    void __import_statement (unsigned, int, char *[], unsigned []);
    void __section_def (unsigned, int, char *[], unsigned []);
    void __temp_name (unsigned, int, char *[], unsigned []);
    void __group_name (unsigned, int, char *[], unsigned []);
    void __stmt_start (unsigned, int, char *[], unsigned []);
    void __section_attr (unsigned, int, char *[], unsigned []);
    void __group_member_list (unsigned, int, char *[], unsigned []);
    void __statement (unsigned, int, char *[], unsigned []);
    void __variable_value (unsigned, int, char *[], unsigned []);
    void __entry_statement (unsigned, int, char *[], unsigned []);
    void __public_symbol (unsigned, int, char *[], unsigned []);
    void __extern_symbol (unsigned, int, char *[], unsigned []);
    void __section_start (unsigned, int, char *[], unsigned []);
    void __section_end (unsigned, int, char *[], unsigned []);
    void __label_definition (unsigned, int, char *[], unsigned []);
    void __label_statement (unsigned, int, char *[], unsigned []);
    void __origin_statement (unsigned, int, char *[], unsigned []);
    void __align_statement (unsigned, int, char *[], unsigned []);
    void __bits_statement (unsigned, int, char *[], unsigned []);
    void __publics_statement (unsigned, int, char *[], unsigned []);
    void __externs_statement (unsigned, int, char *[], unsigned []);
    void __label_definition (unsigned, int, char *[], unsigned []);
    void __data_length_short_spec (unsigned, int, char *[], unsigned []);
    void __data_element (unsigned, int, char *[], unsigned []);
    void __instruction (unsigned, int, char *[], unsigned []);
    void __instruction (unsigned, int, char *[], unsigned []);
    void __mnemonic (unsigned, int, char *[], unsigned []);
    void __epointer (unsigned, int, char *[], unsigned []);
    void __immediate (unsigned, int, char *[], unsigned []);
    void __reg (unsigned, int, char *[], unsigned []);
    void __explicit_size_specification (unsigned, int, char *[], unsigned []);
    void __explicit_mem_type_specification (unsigned, int, char *[], unsigned []);
    void __segment_override (unsigned, int, char *[], unsigned []);
    void __x32mem (unsigned, int, char *[], unsigned []);
    void __x32memOpA (unsigned, int, char *[], unsigned []);
    void __x32memOpB (unsigned, int, char *[], unsigned []);
    void __x32scale (unsigned, int, char *[], unsigned []);
    void pushRule (unsigned, int, char *[], unsigned []);
    void pushRule (unsigned, int, char *[], unsigned []);
    void pushRule (unsigned, int, char *[], unsigned []);
    void pushRule (unsigned, int, char *[], unsigned []);
    void pushRule (unsigned, int, char *[], unsigned []);
    void pushRule (unsigned, int, char *[], unsigned []);
    void pushRule (unsigned, int, char *[], unsigned []);

    static void (*fctbl []) (unsigned, int, char *[], unsigned []) =
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, &__export_statement, 
        &__import_statement, NULL, &__section_def, &__temp_name, 
        &__group_name, &__stmt_start, NULL, &__section_attr, NULL, 
        &__group_member_list, NULL, NULL, &__statement, NULL, 
        &__variable_value, &__entry_statement, NULL, &__public_symbol, NULL, 
        &__extern_symbol, &__section_start, &__section_end, 
        &__label_definition, &__label_statement, &__origin_statement, 
        &__align_statement, &__bits_statement, &__publics_statement, 
        &__externs_statement, NULL, &__label_definition, 
        &__data_length_short_spec, NULL, &__data_element, NULL, 
        &__instruction, &__instruction, &__mnemonic, NULL, NULL, &__epointer, 
        &__immediate, &__reg, NULL, NULL, &__explicit_size_specification, 
        &__explicit_mem_type_specification, &__segment_override, &__x32mem, 
        &__x32memOpA, &__x32memOpB, NULL, &__x32scale, &pushRule, &pushRule, 
        &pushRule, &pushRule, &pushRule, &pushRule, &pushRule, 
    };


    static const unsigned long cbtbl [] =
    {
        0x00000001, 0x00000001, 0x00010006, 0x00040002, 0x00020001, 
        0x00100001, 0x00050003, 0x00060002, 0x00110003, 0x00250002, 
        0x00260001, 0x00270002, 0x00280001, 0x00290002, 0x002a0001, 
        0x002b0001, 0x002c0001, 0x002d0001, 0x002e0001, 0x002f0001, 
        0x00300001, 0x00310002, 0x00330001, 0x00340001, 0x00350001, 
        0x00360001, 0x00370001, 0x00380001, 0x00390001, 0x003a0001, 
        0x003b0001, 0x003c0001, 0x003e0002, 0x00450003, 0x00480003, 
        0x004a0002, 0x004b0004, 0x004d0002, 0x004e0002, 0x004f0002, 
        0x003f0002, 0x00420002, 0x00500003, 0x00510002, 0x00590002, 
        0x005a0001, 0x00530001, 0x005b0001, 0x005e0001, 0x005f0001, 
        0x00000000, 0x00070003, 0x00000000, 0x000f0002, 0x00000000, 
        0x00130001, 0x00000000, 0x00000000, 0x00000000, 0x00ba0001, 
        0x00b60001, 0x00b30001, 0x00b00001, 0x00ac0001, 0x00a80001, 
        0x00a90002, 0x00aa0002, 0x00ab0002, 0x00960003, 0x00970001, 
        0x00980001, 0x00990001, 0x009a0001, 0x009b0001, 0x009c0001, 
        0x009d0001, 0x009e0002, 0x009f0002, 0x00a00002, 0x00a10002, 
        0x00a20002, 0x00a30001, 0x00a40001, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00400003, 0x00410001, 0x00430003, 0x00440001, 0x00000000, 
        0x00540003, 0x00550001, 0x00560005, 0x00570001, 0x00000000, 
        0x005c0002, 0x00600003, 0x00610001, 0x00620001, 0x00630001, 
        0x00640001, 0x00650001, 0x00660003, 0x00740003, 0x00760002, 
        0x00770001, 0x00670002, 0x00680002, 0x006a0001, 0x006b0001, 
        0x006c0001, 0x006d0001, 0x006e0001, 0x006f0001, 0x00700001, 
        0x00710001, 0x00720001, 0x00730001, 0x00780001, 0x00790001, 
        0x007f0007, 0x007d0001, 0x007b0002, 0x00090003, 0x00000000, 
        0x00210003, 0x000f0002, 0x00160002, 0x00170001, 0x00180001, 
        0x00190001, 0x001a0002, 0x001b0001, 0x001c0001, 0x001d0001, 
        0x001e0001, 0x001f0001, 0x00200001, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00540003, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x007e0002, 
        0x00000000, 0x00000000, 0x00a20002, 0x00000000, 0x00000000, 
        0x00930001, 0x00000000, 0x00000000, 0x008d0001, 0x008e0001, 
        0x008f0001, 0x00900001, 0x00910001, 0x00940001, 0x00920001, 
        0x00000000, 0x00000000, 0x00000000, 0x000b0001, 0x000c0001, 
        0x000d0004, 0x000e0004, 0x000f0002, 0x00000000, 0x00140001, 
        0x00000000, 0x00000000, 0x00b40003, 0x00b40003, 0x00b40003, 
        0x00b10003, 0x00b10003, 0x00ad0003, 0x00ad0003, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00250002, 0x000f0002, 0x00000000, 0x00000000, 
        0x00230003, 0x00240001, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00930001, 0x00000000, 0x00000000, 0x00000000, 
        0x00930001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00930001, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 
    };

    static const char *mstbl [] =
    {
        "**** syntax error ***", 
    };

    static int catcher (unsigned r, unsigned e, unsigned st)
    {
        __catcher (r, e, st);

        return 0;
    }

