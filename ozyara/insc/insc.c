/*
    INSC.C

    Ozyara's Instruction Set Compiler Version 0.01

    Copyright (C) 2007-2014 RedStar Technologies
    Written by J. Palencia (ciachn@gmail.com)
*/

    #include <gstring.h>
    #include <stdarg.h>
    #include <stdio.h>

    #include <aurora.h>
    #include <pcc.h>

    /* Instruction information structure. */
    typedef struct /* direct cast: linkable_t, strn_t */
    {
        linkable_t  link;

        char        *name;
        unsigned    long offset;
    }
    ins_t;

    /* Abstract Operand */
    enum AbstractOperand
    {
        __rel8=1, __rel16, __rel32, __r8, __r16, __r32, __sreg,
        __mmxreg, __xmmreg, __imm8, __imm16, __imm32, __rm8,
        __rm16, __rm32, __rm64, __rm128, __mem, __mem8, __mem16,
        __mem32, __mem48, __mem64, __mem128, __i16moffs, __i32moffs,
        __moffs8, __moffs16, __moffs32, __cr, __tr, __dr, __r32m16,
        __r128m32, __r128m64, __far32, __far48, __far64, __ptr32, __ptr48,
        __ptr64, __eptr32, __eptr48, __fpreg, __mem80, __rm80
    };

    /* Opcode Specifier */
    enum OpcodeSpecifier
    {
        __S0, __S1, __S2, __S3, __S4, __S5, __S6, __S7, __Sr, __Sd, __ib,
        __iw, __id, __Prb, __Prw, __Prd, __db, __dw, __dd, __cb, __cw, __cd,
        __rm, __pm, __xm, __pd, __pf, __pq, __idw, __idd
    };

    /* Count of errors encountered. */
    static unsigned errc;

    /* Name of the file being translated. */
    static char *fname;

    /* Output file and its bottom offset. */
    static FILE *fp; static unsigned long bottom;

    /* List of instructions. */
    static list_t *inslist;

    /* Count of instruction forms. */
    static unsigned long forms = 0;

    /* Function to write 'formal' messages to the screen. */
    void pmsg (char *fmts, ...)
    {
        static int flg = 0;
        va_list p;
        char *s;

            if (fmts == NULL)
            {
                flg = 0;
                return;
            }

            va_start (p, fmts);

            if (flg) printf ("\n"); else flg = 1;

            switch (*fmts++)
            {
                case '*':   s = " (error)";
                            errc++;
                            break;

                case '#':   s = " (warning)";
                            break;

                default:    s = "";
                            fmts--;
            }

            if (fname != NULL)
                printf ("%s %u, %u%s: ", fname, linenum, colnum, s);
            else
                printf ("%s%s: ", "insc", s);

            vprintf (fmts, p);
    }

    static void putW (unsigned n, FILE *fp)
    {
            putc (n & 0xFFU, fp);
            putc (n >> 8, fp);
    }

    static void putD (unsigned long n, FILE *fp)
    {
            putW (n & 0xFFFFU, fp);
            putW (n >> 16, fp);
    }

    /* Entry point. */
    int main (int argc, char *argv [])
    {
        FILE *lfp; ins_t *ins;
        unsigned long offs;
        int i, j, k;

            if (argc < 2) return 1;

            printf ("RedStar Ozyara INSC Version 0.01 Copyright (C) 2008-2014 ciachn@gmail.com\n");

            fname = NULL;

            lfp = fopen (argv [1], "rt");
            if (lfp == NULL)
            {
                pmsg ("unable to open input file '%s'", argv [1]);
                return 2;
            }

            fp = fopen ("ozyara.dat", "w+b");
            if (!fp)
            {
                pmsg ("unable to open output file '%s'", "ozyara.dat");
                return 2;
            }

/*
    DWORD   Signature 'ISET'
    WORD    Version (0x0001)
    DWORD   Strings List Offset
*/

            putW ('IS', fp);
            putW ('ET', fp);
            putW (0x0001, fp);
            putD (0, fp);

            inslist = new (list_t);

            setinputstream (lfp);

            fname = argv [1];
            __strcmp = stricmp;
            errc = 0;

            i = parseinput ();

            fname = NULL;

            fclose (lfp);

            if (i && !errc)
            {
                offs = ftell (fp);

                k = 0;

                while ((ins = list__extract (inslist)) != NULL)
                {
                    i = strlen (ins->name);
                    putc (i, fp);

                    for (j = 0; j < i; j++) putc (ins->name [j], fp);

                    putD (ins->offset, fp);

                    delete (ins->name);
                    delete (ins);

                    k++;
                }

                delete (ins);

                putc (0, fp);

                pmsg ("%u instructions compiled, %lu forms (%lu bytes)", k, forms, ftell (fp));

                fseek (fp, 6, SEEK_SET);
                putD (offs, fp);

                fclose (fp);
            }
            else
            {
                fclose (fp);
                remove ("ozyara.dat");
            }

            pmsg ("finished.\n\n");

            return 0;
    }

    /* Use option -p-e (from Piper) to enable catcher calls. */
    void __catcher (unsigned r, unsigned e, unsigned s)
    {
            symbolstr [symbollen] = '\0';

            pmsg ("*rule %u, elem %u, state %u symbol=`%s'",
                r, e, s, symbolstr);
    }

    /* Allocates a block on the file and returns the offset. [1] */
    unsigned long d__alloc (unsigned long size)
    {
            return (bottom += size) - size;
    }

    /* Reads N bytes from the given offset into buf. [1] */
    void d__read (void *buf, unsigned n, unsigned long offset)
    {
            fseek (fp, offset, SEEK_SET);
            fread (buf, n, 1, fp);
    }

    /* Writes N bytes from buf to the offset. [1] */
    void d__write (void *buf, unsigned n, unsigned long offset)
    {
            fseek (fp, offset, SEEK_SET);
            fwrite (buf, n, 1, fp);
    }

/****************************************************************************
****************************************************************************/

    #define MAX_FORMS           255
    #define MAX_SPECIFIERS      32
    #define MAX_OPERANDS        32

    #define ABSTRACT_OPERANDM   0x01
    #define ABSTRACT_OPERAND    0x02
    #define EXPLICIT_OPERAND    0x03
    #define CONSTANT            0x04
    #define OPCODE_SPECIFIER    0x05
    #define OPCODE_LENGTH       0x06

    static unsigned char specdata [MAX_FORMS][MAX_SPECIFIERS][2];
    static unsigned char oprdata [MAX_FORMS][MAX_OPERANDS][2];

    static unsigned speccount [MAX_FORMS];
    static unsigned oprcount [MAX_FORMS];

    static unsigned formcount;

    static list_t names;

    static int isREG (int n)
    {
            switch (n)
            {
                case __r8:
                case __r16:
                case __r32:
                case __sreg:
                case __mmxreg:
                case __xmmreg:
                case __cr:
                case __tr:
                case __dr:
                case __fpreg:
                    return 1;
            }

            return 0;
    };

    void operand (int form, unsigned n, char *argv [], unsigned m [])
    {
            if ((form == 1) && !m [0]) return;

            form += ABSTRACT_OPERANDM;

            if (form == ABSTRACT_OPERANDM && !isREG (m [0]))
            {
                pmsg ("*memory modifier allowed only with registers");
                return;
            }

            oprdata [formcount][oprcount [formcount]][0] = form;

            if (form == CONSTANT)
                oprdata [formcount][oprcount [formcount]++][1] = atoi (argv [0]);
            else
                oprdata [formcount][oprcount [formcount]++][1] = m [0];
    }

    void explicit_operand (int form, unsigned n, char *argv [], unsigned m [])
    {
            p__pushback ("", ((form & 15) << 4) | (m [0] & 15));
    }

    void inc_formcount (void)
    {
            oprcount [++formcount] = 0;
            speccount [formcount] = 0;
            forms++;
    }

    void instruction_name (int form, unsigned n, char *argv [], unsigned m [])
    {
            list__add (&names, new_strn (dups (strupr (argv [0]))));

            oprcount [formcount = 0] = 0;
            speccount [0] = 0;
    }

    void instruction_definition (void)
    {
        ins_t *ins; strn_t *s;
        unsigned len;
        int i, n, m;

            while ((s = list__extract (&names)) != NULL)
            {
                if (gsearch (inslist, s->str))
                {
                    pmsg ("*instruction '%s' already defined", s->str);
                    return;
                }

                ins = new (ins_t);

                ins->name = s->str;
                ins->offset = ftell (fp);

                list__add (inslist, ins);

                delete (s);
            }

            len = 0;

            for (n = 0; n < formcount; n++)
                len += (oprcount [n] + speccount [n] + 2) << 1;

            putW (len + 1, fp);
            putc (formcount, fp);

            for (n = 0; n < formcount; n++)
            {
                putc (oprcount [n], fp);
                putc (speccount [n] + 1, fp);

                for (i = 0; i < oprcount [n]; i++)
                {
                    putc (oprdata [n][i][0], fp);
                    putc (oprdata [n][i][1], fp);
                }

                for (i = m = 0; i < speccount [n]; i++)
                {
                    if (specdata [n][i][0] == CONSTANT) m++;
                }

                putc (OPCODE_LENGTH, fp);
                putc (m, fp);

                for (i = 0; i < speccount [n]; i++)
                {
                    putc (specdata [n][i][0], fp);
                    putc (specdata [n][i][1], fp);
                }
            }

/*
Instruction:
    WORD    Block Length
    BYTE    Form Count

Form:
    BYTE    Operand Count
    BYTE    Specifier Count

Operand / Specifier:
    BYTE    Type
    BYTE    Value
*/
    }

    static unsigned val (char *s)
    {
        unsigned i, p = 0;

            while ((i = *s++) != '\0')
            {
                if ('0' <= i && i <= '9')
                    i -= '0';
                else
                    i = (i & 0xDF) - 'A' + 0x0A;

                p = (p << 4) + i;
            }

            return p;
    }

    void opcode_specifier (int form, unsigned n, char *argv [], unsigned m [])
    {
            form = form ? CONSTANT : OPCODE_SPECIFIER;

            specdata [formcount][speccount [formcount]][0] = form;

            if (form == CONSTANT)
                specdata [formcount][speccount [formcount]++][1] = val (argv [0] + 2);
            else
                specdata [formcount][speccount [formcount]++][1] = m [0];

            if (form == OPCODE_SPECIFIER && (m [0] == __rm || m [0] == __pm || m [0] == __xm))
            {
                for (n = speccount [formcount] - 1; n > 0; n--)
                {
                    specdata [formcount][n][0] = specdata [formcount][n-1][0];
                    specdata [formcount][n][1] = specdata [formcount][n-1][1];
                }

                specdata [formcount][0][0] = OPCODE_SPECIFIER;
                specdata [formcount][0][1] = m [0];
            }
    }
