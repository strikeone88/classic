/*
    AGSCAN.TEM

    Source code template file for the RedStar Phoebe Scanner Generator
    Version 0.14, part of the Pegasus Project.

    NOTE: This template has support for Aurora dictionary engine, used
          to speed up the symbol matching, you'll need Aurora 0.01+
          and Glink 0.15 or later.

    Copyright (C) 2007-2008 RedStar Technologies (June 9 2008)
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <aurora.h>
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
    #define LIST_DEPTH      8

    /* Depth of the character and auxiliary stack. */
    #define STACK_DEPTH     256
    #define AUX_DEPTH       64

    /* Length of the symbol string. */
    #define STR_LEN         512

    /* Special scanner codes. */
    #define c__empty        256
    #define c__eof          257

    /* A node of the strings dictionary. */
    typedef struct /* direct cast: strn_t */
    {
        linkable_t  link;
        char        *val;

        unsigned    index;
        unsigned    block;
    }
    qnode_t;

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
    }
    context_t;

    /* The character stack, an auxiliary stack and a temporal queue. */
    static int top, stack [STACK_DEPTH], aux [AUX_DEPTH];
    static int temp [STACK_DEPTH];

    /* Current input stream (set using setinputstream). */
    static FILE *input;

    /* Indicates scanner case-sensitiveness in symbol matching. */
    static int ssensitive;

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

    /* Scanner strings dictionary. */
    static list_t dictionary;

    /* Prototype of scansymbol. */
    unsigned scansymbol (void);

    /* Prints a message on the screen. */
    void pmsg (const char *, ...);

    /* Converts a string to lowercase. */
    static void lwrcase (char *s)
    {
        int ch;

            for (; (ch = *s) != '\0'; s++)
                if ('A' <= ch && ch <= 'Z') *s = ch | 0x20;
    }

    /* Adds a string to the strings dictionary. */
    static void addString (unsigned block, unsigned index, char *val)
    {
        qnode_t *qnode = new (qnode_t);

            qnode->index = index;
            qnode->block = block;

            qnode->val = val;

            if (!ssensitive) lwrcase (val);

            dict__add (&dictionary, qnode);
    }

    /* Sets the internal input stream and resets a few things. */
    void setinputstream (FILE *_input)
    {
            s__bot = s__cnt = 0;

            top = STACK_DEPTH - 1;

            s__symbol = c__empty;

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
                i = s__getc (input);
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

            free (c);
    }

/****************************************************************************/

    void s__initialize (int case_sensitive)
    {
        static int initialized = 0;

            if (initialized) return;

            ssensitive = case_sensitive;
            initialized = 1;

            addString (0, 28, "$");
            addString (0, 29, "$$");
            addString (0, 27, "ENTRY");
            addString (0, 26, "LABEL");
            addString (0, 24, "MOD");
            addString (0, 25, "PTR");
            addString (0, 15, "align");
            addString (0, 22, "bfloat");
            addString (0, 7, "bits");
            addString (0, 2, "class");
            addString (0, 6, "dup");
            addString (0, 14, "endm");
            addString (0, 3, "ends");
            addString (0, 18, "export");
            addString (0, 5, "extern");
            addString (0, 12, "externs");
            addString (0, 23, "float");
            addString (0, 21, "from");
            addString (0, 1, "group");
            addString (0, 19, "import");
            addString (0, 4, "org");
            addString (0, 17, "page");
            addString (0, 16, "page256");
            addString (0, 8, "ptr32");
            addString (0, 9, "ptr48");
            addString (0, 10, "ptr64");
            addString (0, 11, "publics");
            addString (0, 13, "repeat");
            addString (0, 0, "section");
            addString (0, 20, "to");
            addString (1, 1, "private");
            addString (1, 0, "public");
            addString (2, 0, "byte");
            addString (2, 2, "dword");
            addString (2, 3, "fword");
            addString (2, 6, "para");
            addString (2, 4, "qword");
            addString (2, 5, "tbyte");
            addString (2, 1, "word");
            addString (3, 1, "long");
            addString (3, 0, "short");
            addString (4, 0, "b");
            addString (4, 2, "d");
            addString (4, 3, "f");
            addString (4, 6, "p");
            addString (4, 4, "q");
            addString (4, 5, "t");
            addString (4, 1, "w");
            addString (5, 0, "far32");
            addString (5, 1, "far48");
            addString (5, 2, "far64");
            addString (6, 0, "db");
            addString (6, 2, "dd");
            addString (6, 6, "ddq");
            addString (6, 3, "df");
            addString (6, 4, "dq");
            addString (6, 5, "dt");
            addString (6, 1, "dw");
            addString (7, 3, "AND");
            addString (7, 2, "NOT");
            addString (7, 4, "OR");
            addString (7, 0, "SHL");
            addString (7, 1, "SHR");
            addString (7, 5, "XOR");
            addString (8, 4, "AH");
            addString (8, 0, "AL");
            addString (8, 7, "BH");
            addString (8, 3, "BL");
            addString (8, 5, "CH");
            addString (8, 1, "CL");
            addString (8, 6, "DH");
            addString (8, 2, "DL");
            addString (9, 0, "AX");
            addString (9, 5, "BP");
            addString (9, 3, "BX");
            addString (9, 1, "CX");
            addString (9, 7, "DI");
            addString (9, 2, "DX");
            addString (9, 6, "SI");
            addString (9, 4, "SP");
            addString (10, 0, "EAX");
            addString (10, 5, "EBP");
            addString (10, 3, "EBX");
            addString (10, 1, "ECX");
            addString (10, 7, "EDI");
            addString (10, 2, "EDX");
            addString (10, 6, "ESI");
            addString (10, 4, "ESP");
            addString (11, 1, "CS");
            addString (11, 3, "DS");
            addString (11, 0, "ES");
            addString (11, 4, "FS");
            addString (11, 5, "GS");
            addString (11, 2, "SS");
            addString (12, 0, "CR0");
            addString (12, 1, "CR1");
            addString (12, 2, "CR2");
            addString (12, 3, "CR3");
            addString (12, 4, "CR4");
            addString (12, 5, "CR5");
            addString (12, 6, "CR6");
            addString (12, 7, "CR7");
            addString (13, 0, "DR0");
            addString (13, 1, "DR1");
            addString (13, 2, "DR2");
            addString (13, 3, "DR3");
            addString (13, 4, "DR4");
            addString (13, 5, "DR5");
            addString (13, 6, "DR6");
            addString (13, 7, "DR7");
            addString (14, 0, "TR0");
            addString (14, 1, "TR1");
            addString (14, 2, "TR2");
            addString (14, 3, "TR3");
            addString (14, 4, "TR4");
            addString (14, 5, "TR5");
            addString (14, 6, "TR6");
            addString (14, 7, "TR7");
            addString (15, 0, "MM0");
            addString (15, 1, "MM1");
            addString (15, 2, "MM2");
            addString (15, 3, "MM3");
            addString (15, 4, "MM4");
            addString (15, 5, "MM5");
            addString (15, 6, "MM6");
            addString (15, 7, "MM7");
            addString (16, 0, "XMM0");
            addString (16, 1, "XMM1");
            addString (16, 2, "XMM2");
            addString (16, 3, "XMM3");
            addString (16, 4, "XMM4");
            addString (16, 5, "XMM5");
            addString (16, 6, "XMM6");
            addString (16, 7, "XMM7");
            addString (17, 0, "ST0");
            addString (17, 1, "ST1");
            addString (17, 2, "ST2");
            addString (17, 3, "ST3");
            addString (17, 4, "ST4");
            addString (17, 5, "ST5");
            addString (17, 6, "ST6");
            addString (17, 7, "ST7");
    }

    unsigned scansymbol (void)
    {
        register int ch, c, d;
        qnode_t *qnode;
        int *p;

        s__count = -1;

        begin: symbollen = s__index = 0;

        colnum = __colnum;
        linenum = __linenum;

        s__count++;

        p = fillup ();

        switch (p [0])
        {
            case '/':
                switch (p [1])
                {
                    case '/':
                        c = 2;
                        d = 1;
                        goto L0;

                    case '*':
                        c = 2;
                        d = 2;
                        goto L0;
                }

                break;

            case ';':
                c = 1;
                d = 1;
                goto L0;

            case 9:    case 11:    case 12:    case 13:    case ' ':
                c = 1;
                d = 3;
                goto L0;

            case 39:
                c = 1;
                d = 4;
                goto L0;

            case '"':
                c = 1;
                d = 5;
                goto L0;

            case '#':
                c = 1;
                d = 6;
                goto L0;

            case '$':    case '&':    case '.':    case '?':    case '@':
            case 'A':    case 'B':    case 'C':    case 'D':    case 'E':
            case 'F':    case 'G':    case 'H':    case 'I':    case 'J':
            case 'K':    case 'L':    case 'M':    case 'N':    case 'O':
            case 'P':    case 'Q':    case 'R':    case 'S':    case 'T':
            case 'U':    case 'V':    case 'W':    case 'X':    case 'Y':
            case 'Z':    case '_':    case 'a':    case 'b':    case 'c':
            case 'd':    case 'e':    case 'f':    case 'g':    case 'h':
            case 'i':    case 'j':    case 'k':    case 'l':    case 'm':
            case 'n':    case 'o':    case 'p':    case 'q':    case 'r':
            case 's':    case 't':    case 'u':    case 'v':    case 'w':
            case 'x':    case 'y':    case 'z':
                c = 1;
                d = 7;
                goto L0;

            case '0':    case '1':    case '2':    case '3':    case '4':
            case '5':    case '6':    case '7':    case '8':    case '9':
                c = 1;
                d = 8;
                goto L0;

            case 92:
                c = 1;
                d = 9;
                goto L0;

            case 10:
                c = 1;
                d = 10;
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
                    case 1:    case 2:    case 3:    case 4:    case 5:
                    case 6:    case 7:    case 8:    case 9:    case 11:
                    case 12:    case 13:    case 14:    case 15:    case 16:
                    case 17:    case 18:    case 19:    case 20:    case 21:
                    case 22:    case 23:    case 24:    case 25:    case 26:
                    case 27:    case 28:    case 29:    case 30:    case 31:
                    case ' ':    case '!':    case '"':    case '#':    case '$':
                    case '%':    case '&':    case 39:    case '(':    case ')':
                    case '*':    case '+':    case ',':    case '-':    case '.':
                    case '/':    case '0':    case '1':    case '2':    case '3':
                    case '4':    case '5':    case '6':    case '7':    case '8':
                    case '9':    case ':':    case ';':    case '<':    case '=':
                    case '>':    case '?':    case '@':    case 'A':    case 'B':
                    case 'C':    case 'D':    case 'E':    case 'F':    case 'G':
                    case 'H':    case 'I':    case 'J':    case 'K':    case 'L':
                    case 'M':    case 'N':    case 'O':    case 'P':    case 'Q':
                    case 'R':    case 'S':    case 'T':    case 'U':    case 'V':
                    case 'W':    case 'X':    case 'Y':    case 'Z':    case '[':
                    case 92:    case ']':    case '^':    case '_':    case '`':
                    case 'a':    case 'b':    case 'c':    case 'd':    case 'e':
                    case 'f':    case 'g':    case 'h':    case 'i':    case 'j':
                    case 'k':    case 'l':    case 'm':    case 'n':    case 'o':
                    case 'p':    case 'q':    case 'r':    case 's':    case 't':
                    case 'u':    case 'v':    case 'w':    case 'x':    case 'y':
                    case 'z':    case '{':    case '|':    case '}':    case '~':
                    case 127:    case 128:    case 129:    case 130:    case 131:
                    case 132:    case 133:    case 134:    case 135:    case 136:
                    case 137:    case 138:    case 139:    case 140:    case 141:
                    case 142:    case 143:    case 144:    case 145:    case 146:
                    case 147:    case 148:    case 149:    case 150:    case 151:
                    case 152:    case 153:    case 154:    case 155:    case 156:
                    case 157:    case 158:    case 159:    case 160:    case 161:
                    case 162:    case 163:    case 164:    case 165:    case 166:
                    case 167:    case 168:    case 169:    case 170:    case 171:
                    case 172:    case 173:    case 174:    case 175:    case 176:
                    case 177:    case 178:    case 179:    case 180:    case 181:
                    case 182:    case 183:    case 184:    case 185:    case 186:
                    case 187:    case 188:    case 189:    case 190:    case 191:
                    case 192:    case 193:    case 194:    case 195:    case 196:
                    case 197:    case 198:    case 199:    case 200:    case 201:
                    case 202:    case 203:    case 204:    case 205:    case 206:
                    case 207:    case 208:    case 209:    case 210:    case 211:
                    case 212:    case 213:    case 214:    case 215:    case 216:
                    case 217:    case 218:    case 219:    case 220:    case 221:
                    case 222:    case 223:    case 224:    case 225:    case 226:
                    case 227:    case 228:    case 229:    case 230:    case 231:
                    case 232:    case 233:    case 234:    case 235:    case 236:
                    case 237:    case 238:    case 239:    case 240:    case 241:
                    case 242:    case 243:    case 244:    case 245:    case 246:
                    case 247:    case 248:    case 249:    case 250:    case 251:
                    case 252:    case 253:    case 254:    case 255:
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
                            case '/':
                                c = 2;
                                goto L4;
                        }

                        break;
                }

                if (*p != c__eof) { c = 1; goto L5; }

                pmsg ("*unable to locate end of %s", "comment2");

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
                    case 9:    case 11:    case 12:    case 13:    case ' ':
                        c = 1;
                        goto L7;
                }

                goto begin;

                L7: while (c--) { ch = pop (); shift (ch); }
                goto L6;

            case 4:
                L8: p = fillup ();

                switch (p [0])
                {
                    case 39:
                        c = 1;
                        goto L9;
                }

                if (*p != c__eof) { c = 1; goto L10; }

                pmsg ("*unable to locate end of %s", "character-literal");

                goto begin;

                L10: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto L8;

                L9: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                return 261;

            case 5:
                L11: p = fillup ();

                switch (p [0])
                {
                    case '"':
                        c = 1;
                        goto L12;
                }

                if (*p != c__eof) { c = 1; goto L13; }

                pmsg ("*unable to locate end of %s", "string-literal");

                goto begin;

                L13: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto L11;

                L12: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                return 262;

            case 6:
                L14: p = fillup ();

                switch (p [0])
                {
                    case '#':
                        c = 1;
                        goto L15;
                }

                if (*p != c__eof) { c = 1; goto L16; }

                __mident (0);
                goto begin;

                L16: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto L14;

                L15: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                if ((c = __mident (1)) != 0) return c;

                return 263;

            case 7:
                L17: p = fillup ();

                switch (p [0])
                {
                    case '$':    case '&':    case '.':    case '0':    case '1':
                    case '2':    case '3':    case '4':    case '5':    case '6':
                    case '7':    case '8':    case '9':    case '?':    case '@':
                    case 'A':    case 'B':    case 'C':    case 'D':    case 'E':
                    case 'F':    case 'G':    case 'H':    case 'I':    case 'J':
                    case 'K':    case 'L':    case 'M':    case 'N':    case 'O':
                    case 'P':    case 'Q':    case 'R':    case 'S':    case 'T':
                    case 'U':    case 'V':    case 'W':    case 'X':    case 'Y':
                    case 'Z':    case '_':    case 'a':    case 'b':    case 'c':
                    case 'd':    case 'e':    case 'f':    case 'g':    case 'h':
                    case 'i':    case 'j':    case 'k':    case 'l':    case 'm':
                    case 'n':    case 'o':    case 'p':    case 'q':    case 'r':
                    case 's':    case 't':    case 'u':    case 'v':    case 'w':
                    case 'x':    case 'y':    case 'z':
                        c = 1;
                        goto L18;
                }

                if ((c = __ident (1)) != 0) return c;

                symbolstr [symbollen] = 0;

                if (ssensitive)
                    qnode = dict__srch (&dictionary, symbolstr);
                else
                    qnode = dict__isrch (&dictionary, symbolstr);

                if (!qnode) goto SL_0;

                s__index = ((unsigned long)qnode->index) << 16;

                switch (qnode->block)
                {
                    case 0x0000: return 273;
                    case 0x0001: return 274;
                    case 0x0002: return 275;
                    case 0x0003: return 276;
                    case 0x0004: return 277;
                    case 0x0005: return 278;
                    case 0x0006: return 279;
                    case 0x0007: return 280;
                    case 0x0008: return 281;
                    case 0x0009: return 282;
                    case 0x000a: return 283;
                    case 0x000b: return 284;
                    case 0x000c: return 285;
                    case 0x000d: return 286;
                    case 0x000e: return 287;
                    case 0x000f: return 288;
                    case 0x0010: return 289;
                    case 0x0011: return 290;
                }

                SL_0:;

                return 264;

                L18: while (c--) { ch = pop (); shift (ch); }
                goto L17;

            case 8:
                L19: p = fillup ();

                switch (p [0])
                {
                    case '.':    case '0':    case '1':    case '2':    case '3':
                    case '4':    case '5':    case '6':    case '7':    case '8':
                    case '9':    case 'A':    case 'B':    case 'C':    case 'D':
                    case 'E':    case 'F':    case 'H':    case 'Q':    case '_':
                    case 'a':    case 'b':    case 'c':    case 'd':    case 'e':
                    case 'f':    case 'h':    case 'q':
                        c = 1;
                        goto L20;
                }

                if ((c = __number (1)) != 0) return c;

                return 265;

                L20: while (c--) { ch = pop (); shift (ch); }
                goto L19;

            case 9:
                L21: p = fillup ();

                switch (p [0])
                {
                    case 1:    case 2:    case 3:    case 4:    case 5:
                    case 6:    case 7:    case 8:    case 9:    case 11:
                    case 12:    case 13:    case 14:    case 15:    case 16:
                    case 17:    case 18:    case 19:    case 20:    case 21:
                    case 22:    case 23:    case 24:    case 25:    case 26:
                    case 27:    case 28:    case 29:    case 30:    case 31:
                    case ' ':    case '!':    case '"':    case '#':    case '$':
                    case '%':    case '&':    case 39:    case '(':    case ')':
                    case '*':    case '+':    case ',':    case '-':    case '.':
                    case '/':    case '0':    case '1':    case '2':    case '3':
                    case '4':    case '5':    case '6':    case '7':    case '8':
                    case '9':    case ':':    case ';':    case '<':    case '=':
                    case '>':    case '?':    case '@':    case 'A':    case 'B':
                    case 'C':    case 'D':    case 'E':    case 'F':    case 'G':
                    case 'H':    case 'I':    case 'J':    case 'K':    case 'L':
                    case 'M':    case 'N':    case 'O':    case 'P':    case 'Q':
                    case 'R':    case 'S':    case 'T':    case 'U':    case 'V':
                    case 'W':    case 'X':    case 'Y':    case 'Z':    case '[':
                    case 92:    case ']':    case '^':    case '_':    case '`':
                    case 'a':    case 'b':    case 'c':    case 'd':    case 'e':
                    case 'f':    case 'g':    case 'h':    case 'i':    case 'j':
                    case 'k':    case 'l':    case 'm':    case 'n':    case 'o':
                    case 'p':    case 'q':    case 'r':    case 's':    case 't':
                    case 'u':    case 'v':    case 'w':    case 'x':    case 'y':
                    case 'z':    case '{':    case '|':    case '}':    case '~':
                    case 127:    case 128:    case 129:    case 130:    case 131:
                    case 132:    case 133:    case 134:    case 135:    case 136:
                    case 137:    case 138:    case 139:    case 140:    case 141:
                    case 142:    case 143:    case 144:    case 145:    case 146:
                    case 147:    case 148:    case 149:    case 150:    case 151:
                    case 152:    case 153:    case 154:    case 155:    case 156:
                    case 157:    case 158:    case 159:    case 160:    case 161:
                    case 162:    case 163:    case 164:    case 165:    case 166:
                    case 167:    case 168:    case 169:    case 170:    case 171:
                    case 172:    case 173:    case 174:    case 175:    case 176:
                    case 177:    case 178:    case 179:    case 180:    case 181:
                    case 182:    case 183:    case 184:    case 185:    case 186:
                    case 187:    case 188:    case 189:    case 190:    case 191:
                    case 192:    case 193:    case 194:    case 195:    case 196:
                    case 197:    case 198:    case 199:    case 200:    case 201:
                    case 202:    case 203:    case 204:    case 205:    case 206:
                    case 207:    case 208:    case 209:    case 210:    case 211:
                    case 212:    case 213:    case 214:    case 215:    case 216:
                    case 217:    case 218:    case 219:    case 220:    case 221:
                    case 222:    case 223:    case 224:    case 225:    case 226:
                    case 227:    case 228:    case 229:    case 230:    case 231:
                    case 232:    case 233:    case 234:    case 235:    case 236:
                    case 237:    case 238:    case 239:    case 240:    case 241:
                    case 242:    case 243:    case 244:    case 245:    case 246:
                    case 247:    case 248:    case 249:    case 250:    case 251:
                    case 252:    case 253:    case 254:    case 255:
                        c = 1;
                        goto L22;
                }

                return 266;

                L22: while (c--)
                {
                    ch = pop (); shift (ch);
                    if (ch == '\n') __linenum++, __colnum = 1;
                }
                goto L21;

            case 10:
                return 267;
        }

        return c__eof;
    }