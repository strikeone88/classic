/*
    SCANNER.TEM

    Source code template file for the RedStar Phoebe Scanner Generator
    Version 0.14, part of the Pegasus Project.

    Copyright (C) 2007-2010 RedStar Technologies (May 25 2010)
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>

    /* Shifts the given character into the symbol string. */
    #define shift(ch)       if (len < STR_LEN - 1) symbolstr [len++] = ch

    /* Pushes the given character back into the stack. */
    #define push_back(ch)   stack [top--] = ch

    /* This is used ONLY to make the above macro fit in one line. */
    #define len             symbollen

    /* Pops a character off of the stack. */
    #define pop()           (__colnum++, stack [++top])

    /* Depth of the "shifted symbols" list (must be in base two). */
    #ifndef LIST_DEPTH
    #define LIST_DEPTH      8
    #endif

    /* Depth of the character and auxiliary stack. */
    #ifndef STACK_DEPTH
    #define STACK_DEPTH     4096
    #endif

    #ifndef AUX_DEPTH
    #define AUX_DEPTH       128
    #endif

    /* Length of the symbol string. */
    #ifndef STR_LEN
    #define STR_LEN         4096
    #endif

    /* Special scanner codes. */
    #define c__empty        256
    #define c__eof          257

    /* The scanner context structure. */
    typedef struct
    {
        /* The character stack, an auxiliary stack and a temporal queue. */
        int top, stack [STACK_DEPTH], aux [AUX_DEPTH];
        int temp [STACK_DEPTH];

        /* Current input stream (set using setinputstream). */
        FILE *input;

        /* This is the symbol string and its length, both are public. */
        char symbolstr [STR_LEN];
        int symbollen;

        /* Current symbol code (and index) to avoid unwanted scan. */
        unsigned s__symbol;
        unsigned long s__index;

        /* Line and col number of the symbol that was just scanned. */
        unsigned linenum, colnum;

        /* Count of invisible lexemes before the current. */
        int s__count;

        /* Same as above, but for the NEXT symbol. */
        unsigned __linenum, __colnum;

        /* List of the last LIST_DEPTH shifted symbols. */
        unsigned long s__shifted [LIST_DEPTH];
        unsigned s__bot, s__cnt;

        /* Function to compare strings, set to strcmp at first. */
        int (*__strcmp) (const char *, const char *);

        /* Current position within the file. */
        unsigned long offset;
    }
    context_t;

    /* The character stack, an auxiliary stack and a temporal queue. */
    static int top, stack [STACK_DEPTH], aux [AUX_DEPTH];
    static int temp [STACK_DEPTH];

    /* Current input stream (set using setinputstream). */
    static FILE *input;

    /* This is the symbol string and its length, both are public. */
    char symbolstr [STR_LEN];
    int symbollen;

    /* Current symbol code (and index) to avoid unwanted scan. */
    unsigned long s__index;
    unsigned s__symbol;

    /* Line and col number of the symbol that was just scanned. */
    unsigned linenum, colnum;

    /* Count of invisible lexemes before the current. */
    int s__count;

    /* Same as above, but for the NEXT symbol. */
    static unsigned __linenum, __colnum;

    /* List of the last LIST_DEPTH shifted symbols. */
    static unsigned long s__shifted [LIST_DEPTH];
    static unsigned s__bot, s__cnt;

    /* Prototype of scansymbol. */
    unsigned scansymbol (void);

    /* Function to compare strings, set to strcmp at first. */
    int (*__strcmp) (const char *, const char *);

    /* Prints a message on the screen. */
    void pmsg (const char *, ...);

    /* Sets the internal input stream and resets a few things. */
    void setinputstream (FILE *_input)
    {
            s__bot = s__cnt = 0;

            top = STACK_DEPTH - 1;

            s__symbol = c__empty;
            __strcmp = strcmp;

            input = _input;

            __linenum = 1;
            __colnum = 1;
    }

    /* Fills up the character stack as long as the current level is
       below or equal to the aux depth. */

    static int *fillup (void)
    {
        int c, n = 0, i = 0, left = top + 1;

            if ((c = STACK_DEPTH - left) > AUX_DEPTH)
                return &stack [top + 1];

            while (i < c) aux [i++] = stack [++top];

            while (left--)
            {
                i = getc (input);
                if (i == EOF) break;

                temp [n++] = i;
            }

            if ((i = n + c) < AUX_DEPTH)
            {
                i = AUX_DEPTH - i;
                while (i--) push_back (c__eof);
            }

            while (n--) push_back (temp [n]);

            while (c--) push_back (aux [c]);

            return &stack [top + 1];
    }

    /* Given a list of strings, it tries to match symbolstr, if found
       returns the index + 1 or zero if not found. */

    static int __bsearch (char *s [], unsigned n [], unsigned c)
    {
        int i, mid, base = 0;

            while (c)
            {
                mid = base + c / 2;

                i = __strcmp (symbolstr, s [mid]);
                if (!i) return 1 + n [mid];

                if (i > 0)
                {
                    c -= mid - base + 1;
                    base = mid + 1;
                }
                else
                    c = mid - base;
            }

            return 0;
    }

    /* Returns the current symbol code (scans if necessary). */
    unsigned long readsymbol (void)
    {
            if (s__symbol == c__empty) s__symbol = scansymbol ();

            return s__index | s__symbol;
    }

    /* Pops a character off of the stack. */
    int s__pop (void)
    {
            fillup ();
            return pop ();
    }

    /* Pushes the given character back into the stack. */
    void s__push_back (int ch)
    {
            push_back (ch);
            __colnum--;
    }

    /* Returns the N-th previous shifted symbol code (if any). */
    unsigned long s__peek (int n)
    {
            if (n > s__cnt || n < 1) return 0;

            return s__shifted [s__bot - n + LIST_DEPTH & LIST_DEPTH - 1];
    }

    /* Puts a symbol code in the shifted list. */
    void s__put (unsigned long x)
    {
            if (++s__cnt > LIST_DEPTH) s__cnt = LIST_DEPTH;

            s__shifted [s__bot++] = x;

            s__bot &= LIST_DEPTH - 1;
    }

    /* Shifts the current symbol, this will cause to force to scan
       another symbol on next call to readsymbol. */

    void shiftsymbol (void)
    {
            if (s__symbol != c__eof)
            {
                s__put (s__index | s__symbol);
                s__symbol = c__empty;
            }
    }

    /* Saves the context of the scanner and returns a pointer as void
       to the structure, returns NULL if can't save the context. */

    void *s__savectx (void)
    {
        context_t *c = malloc (sizeof (context_t));

            if (c == NULL) return NULL;

            /* Save all the single variables. */
            c->symbollen = symbollen;
            c->s__symbol = s__symbol;
            c->__linenum = __linenum;
            c->s__index = s__index;
            c->s__count = s__count;
            c->__colnum = __colnum;
            c->__strcmp = __strcmp;
            c->linenum = linenum;
            c->colnum = colnum;
            c->s__bot = s__bot;
            c->s__cnt = s__cnt;
            c->input = input;
            c->top = top;

            /* Save all the arrays. */
            memcpy (c->s__shifted, s__shifted, sizeof (s__shifted));
            memcpy (c->symbolstr, symbolstr, sizeof (symbolstr));
            memcpy (c->stack, stack, sizeof (stack));
            memcpy (c->temp, temp, sizeof (temp));
            memcpy (c->aux, aux, sizeof (aux));

            /* Save offset. */
            c->offset = ftell (input);

            return (void *)c;
    }

    /* Loads the context of the scanner. */
    void s__loadctx (context_t *c)
    {
            if (c == NULL) return;

            /* Restore all the single variables. */
            symbollen = c->symbollen;
            s__symbol = c->s__symbol;
            __linenum = c->__linenum;
            s__index = c->s__index;
            s__count = c->s__count;
            __colnum = c->__colnum;
            __strcmp = c->__strcmp;
            linenum = c->linenum;
            colnum = c->colnum;
            s__bot = c->s__bot;
            s__cnt = c->s__cnt;
            input = c->input;
            top = c->top;

            /* Restore all the arrays. */
            memcpy (s__shifted, c->s__shifted, sizeof (s__shifted));
            memcpy (symbolstr, c->symbolstr, sizeof (symbolstr));
            memcpy (stack, c->stack, sizeof (stack));
            memcpy (temp, c->temp, sizeof (temp));
            memcpy (aux, c->aux, sizeof (aux));

            /* Restore offset. */
            fseek (input, c->offset, SEEK_SET);

            free (c);
    }

/****************************************************************************/

    static char *S0 [] =
    {
        "+rb", "+rd", "+rw", "/0", "/1", "/2", "/3", "/4", "/5", "/6", "/7", 
        "/d", "/r", "cb", "cd", "cw", "db", "dd", "dw", "ib", "id", "idd", 
        "idw", "iw", "pd", "pf", "pm", "pq", "rm", "xm", 
    };

    static unsigned I0 [] =
    {
        13, 15, 14, 0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 19, 21, 20, 16, 18, 17, 10, 
        12, 29, 28, 11, 25, 26, 23, 27, 22, 24, 
    };

    static char *S1 [] =
    {
        "cr", "dr", "eptr32", "eptr48", "far32", "far48", "far64", "fpreg", 
        "i16moffs", "i32moffs", "imm16", "imm32", "imm8", "m", "mem128", 
        "mem16", "mem32", "mem48", "mem64", "mem8", "mem80", "mmxreg", 
        "moffs16", "moffs32", "moffs8", "none", "ptr32", "ptr48", "ptr64", 
        "r/m128", "r/m16", "r/m32", "r/m64", "r/m8", "r/m80", "r128/m32", 
        "r128/m64", "r16", "r32", "r32/m16", "r8", "rel16", "rel32", "rel8", 
        "sreg", "tr", "xmmreg", 
    };

    static unsigned I1 [] =
    {
        30, 32, 42, 43, 36, 37, 38, 44, 25, 26, 11, 12, 10, 18, 24, 20, 21, 
        22, 23, 19, 45, 8, 28, 29, 27, 0, 39, 40, 41, 17, 14, 15, 16, 13, 46, 
        34, 35, 5, 6, 33, 4, 2, 3, 1, 7, 31, 9, 
    };

    static char *S2 [] =
    {
        "AH", "AL", "BH", "BL", "CH", "CL", "DH", "DL", 
    };

    static unsigned I2 [] =
    {
        4, 0, 7, 3, 5, 1, 6, 2, 
    };

    static char *S3 [] =
    {
        "AX", "BP", "BX", "CX", "DI", "DX", "SI", "SP", 
    };

    static unsigned I3 [] =
    {
        0, 5, 3, 1, 7, 2, 6, 4, 
    };

    static char *S4 [] =
    {
        "EAX", "EBP", "EBX", "ECX", "EDI", "EDX", "ESI", "ESP", 
    };

    static unsigned I4 [] =
    {
        0, 5, 3, 1, 7, 2, 6, 4, 
    };

    static char *S5 [] =
    {
        "CS", "DS", "ES", "FS", "GS", "SS", 
    };

    static unsigned I5 [] =
    {
        1, 3, 0, 4, 5, 2, 
    };

    static char *S6 [] =
    {
        "CR0", "CR1", "CR2", "CR3", "CR4", "CR5", "CR6", "CR7", 
    };

    static unsigned I6 [] =
    {
        0, 1, 2, 3, 4, 5, 6, 7, 
    };

    static char *S7 [] =
    {
        "DR0", "DR1", "DR2", "DR3", "DR4", "DR5", "DR6", "DR7", 
    };

    static unsigned I7 [] =
    {
        0, 1, 2, 3, 4, 5, 6, 7, 
    };

    static char *S8 [] =
    {
        "TR0", "TR1", "TR2", "TR3", "TR4", "TR5", "TR6", "TR7", 
    };

    static unsigned I8 [] =
    {
        0, 1, 2, 3, 4, 5, 6, 7, 
    };

    static char *S9 [] =
    {
        "ST0", "ST1", "ST2", "ST3", "ST4", "ST5", "ST6", "ST7", 
    };

    static unsigned I9 [] =
    {
        0, 1, 2, 3, 4, 5, 6, 7, 
    };

    unsigned scansymbol (void)
    {
        register int ch, c, d;
        int *p;

        s__count = -1;

        begin: symbollen = s__index = 0;

        colnum = __colnum;
        linenum = __linenum;

        s__count++;

        p = fillup ();

        switch (p [0])
        {
            case 9:    case 10:    case 11:    case 12:    case 13:
            case ' ':
                c = 1;
                d = 1;
                goto L0;

            case '(':
                switch (p [1])
                {
                    case '*':
                        c = 2;
                        d = 2;
                        goto L0;
                }

                break;

            case '{':
                switch (p [1])
                {
                    case '*':
                        c = 2;
                        d = 3;
                        goto L0;
                }

                break;

            case '/':
                switch (p [1])
                {
                    case '/':
                        c = 2;
                        d = 4;
                        goto L0;
                }

                c = 1;
                d = 7;
                goto L0;

            case '0':
                switch (p [1])
                {
                    case 'x':
                        c = 2;
                        d = 5;
                        goto L0;
                }
    case '1':    case '2':    case '3':    case '4':
            case '5':    case '6':    case '7':    case '8':    case '9':
                c = 1;
                d = 6;
                goto L0;

            case '+':    case 'A':    case 'B':    case 'C':    case 'D':
            case 'E':    case 'F':    case 'G':    case 'H':    case 'I':
            case 'J':    case 'K':    case 'L':    case 'M':    case 'N':
            case 'O':    case 'P':    case 'Q':    case 'R':    case 'S':
            case 'T':    case 'U':    case 'V':    case 'W':    case 'X':
            case 'Y':    case 'Z':    case 'a':    case 'b':    case 'c':
            case 'd':    case 'e':    case 'f':    case 'g':    case 'h':
            case 'i':    case 'j':    case 'k':    case 'l':    case 'm':
            case 'n':    case 'o':    case 'p':    case 'q':    case 'r':
            case 's':    case 't':    case 'u':    case 'v':    case 'w':
            case 'x':    case 'y':    case 'z':
                c = 1;
                d = 7;
                goto L0;
        }

        ch = pop ();
        shift (ch);

        return ch;

        L0: while (c--)
        {
            ch = pop (); shift (ch);
            if (ch == '\n') __linenum++, __colnum = 1;
        }

        switch (d)
        {
            case 1:
                L1: p = fillup ();

                switch (p [0])
                {
                    case 9:    case 10:    case 11:    case 12:    case 13:
                    case ' ':
                        c = 1;
                        goto L2;
                }

                goto begin;

                L2: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto L1;

            case 2:
                L3: p = fillup ();

                switch (p [0])
                {
                    case '*':
                        switch (p [1])
                        {
                            case ')':
                                c = 2;
                                goto L4;
                        }

                        break;
                }

                if (*p != c__eof) { c = 1; goto L5; }

                pmsg ("*unable to locate end of %s", "comment1");

                goto begin;

                L5: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto L3;

                L4: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto begin;

            case 3:
                L6: p = fillup ();

                switch (p [0])
                {
                    case '*':
                        switch (p [1])
                        {
                            case '}':
                                c = 2;
                                goto L7;
                        }

                        break;
                }

                if (*p != c__eof) { c = 1; goto L8; }

                pmsg ("*unable to locate end of %s", "comment2");

                goto begin;

                L8: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto L6;

                L7: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto begin;

            case 4:
                L9: p = fillup ();

                switch (p [0])
                {
                    case 10:
                        c = 1;
                        goto L10;
                }

                if (*p != c__eof) { c = 1; goto L11; }

                pmsg ("*unable to locate end of %s", "comment3");

                goto begin;

                L11: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto L9;

                L10: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto begin;

            case 5:
                L12: p = fillup ();

                switch (p [0])
                {
                    case '0':    case '1':    case '2':    case '3':    case '4':
                    case '5':    case '6':    case '7':    case '8':    case '9':
                    case 'A':    case 'B':    case 'C':    case 'D':    case 'E':
                    case 'F':    case 'a':    case 'b':    case 'c':    case 'd':
                    case 'e':    case 'f':
                        c = 1;
                        goto L13;
                }

                return 262;

                L13: while (c--) { ch = pop (); shift (ch); }
                goto L12;

            case 6:
                L14: p = fillup ();

                switch (p [0])
                {
                    case '0':    case '1':    case '2':    case '3':    case '4':
                    case '5':    case '6':    case '7':    case '8':    case '9':
                        c = 1;
                        goto L15;
                }

                return 263;

                L15: while (c--) { ch = pop (); shift (ch); }
                goto L14;

            case 7:
                L16: p = fillup ();

                switch (p [0])
                {
                    case '/':    case '0':    case '1':    case '2':    case '3':
                    case '4':    case '5':    case '6':    case '7':    case '8':
                    case '9':    case 'A':    case 'B':    case 'C':    case 'D':
                    case 'E':    case 'F':    case 'G':    case 'H':    case 'I':
                    case 'J':    case 'K':    case 'L':    case 'M':    case 'N':
                    case 'O':    case 'P':    case 'Q':    case 'R':    case 'S':
                    case 'T':    case 'U':    case 'V':    case 'W':    case 'X':
                    case 'Y':    case 'Z':    case 'a':    case 'b':    case 'c':
                    case 'd':    case 'e':    case 'f':    case 'g':    case 'h':
                    case 'i':    case 'j':    case 'k':    case 'l':    case 'm':
                    case 'n':    case 'o':    case 'p':    case 'q':    case 'r':
                    case 's':    case 't':    case 'u':    case 'v':    case 'w':
                    case 'x':    case 'y':    case 'z':
                        c = 1;
                        goto L17;
                }

                symbolstr [symbollen] = 0;

                if ((c = __bsearch (S0, I0, 30)) != 0)
                {
                    s__index = ((unsigned long)(c - 1)) << 16;
                    return 265;
                }

                if ((c = __bsearch (S1, I1, 47)) != 0)
                {
                    s__index = ((unsigned long)(c - 1)) << 16;
                    return 266;
                }

                if ((c = __bsearch (S2, I2, 8)) != 0)
                {
                    s__index = ((unsigned long)(c - 1)) << 16;
                    return 267;
                }

                if ((c = __bsearch (S3, I3, 8)) != 0)
                {
                    s__index = ((unsigned long)(c - 1)) << 16;
                    return 268;
                }

                if ((c = __bsearch (S4, I4, 8)) != 0)
                {
                    s__index = ((unsigned long)(c - 1)) << 16;
                    return 269;
                }

                if ((c = __bsearch (S5, I5, 6)) != 0)
                {
                    s__index = ((unsigned long)(c - 1)) << 16;
                    return 270;
                }

                if ((c = __bsearch (S6, I6, 8)) != 0)
                {
                    s__index = ((unsigned long)(c - 1)) << 16;
                    return 271;
                }

                if ((c = __bsearch (S7, I7, 8)) != 0)
                {
                    s__index = ((unsigned long)(c - 1)) << 16;
                    return 272;
                }

                if ((c = __bsearch (S8, I8, 8)) != 0)
                {
                    s__index = ((unsigned long)(c - 1)) << 16;
                    return 273;
                }

                if ((c = __bsearch (S9, I9, 8)) != 0)
                {
                    s__index = ((unsigned long)(c - 1)) << 16;
                    return 274;
                }

                return 264;

                L17: while (c--) { ch = pop (); shift (ch); }
                goto L16;
        }

        return c__eof;
    }