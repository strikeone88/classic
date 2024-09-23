/*
    BF2.C

    RedStar Binary Font 2.0 Engine Version 0.03

    Copyright (C) 2007-2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <stdarg.h>
    #include <bf2.h>

    /* BF2 Context Structure. */
    typedef struct
    {
        /* BF2 header information. */
        int startCh, endCh, charHeight;

        /* Offsets table. */
        unsigned char *OffsTable [256];

        /* Length of an space in pixels. */
        int spaceLen;

        /* Current text color. */
        int r, g, b;

        /* Graphics interface. */
        ServiceInterface gfx;

        /* Text view port. */
        int hasViewPort, x1, y1, x2, y2;
    }
    Context;

    /* This is a global general purpose temporal buffer. */
    unsigned char g__tempBuf [1024];

    void *bf2 (unsigned Command, ...)
    {
        static int w, u, v, a, b;
        static va_list args;
        static Context *Ct;

        unsigned char *p, *q;
        int i, j, k;

            va_start (args, Command);

            switch (Command)
            {
                case BCreateContext:
                    return calloc (sizeof (Context));

                case BDestroyContext:
                    free (va_arg (args, void *));
                    break;

                case BSelectGfx:
                    Ct = va_arg (args, Context *);
                    if (!Ct) returnv (1);

                    Ct->gfx = va_arg (args, ServiceInterface);
                    break;

                case BSetFont:
                    Ct = va_arg (args, Context *);
                    if (!Ct) returnv (1);

                    p = q = va_arg (args, unsigned char *);

                    if (*((unsigned long *)p) != 0x21324642UL)
                        returnv (1);

                    Ct->charHeight = *((unsigned char *)(p + 0x06));
                    Ct->startCh = *((unsigned char *)(p + 0x04));
                    Ct->endCh = *((unsigned char *)(p + 0x05));

                    j = Ct->endCh - Ct->startCh + 1;
                    i = va_arg (args, unsigned);

                    p += i - 2*j;

                    for (i = 0; i < j; i++)
                        Ct->OffsTable [i] = q + *((unsigned short *)p)++;

                    Ct->spaceLen = 5;
                    break;

                case BSetColor:
                    Ct = va_arg (args, Context *);
                    if (!Ct) returnv (1);

                    Ct->r = va_arg (args, int);
                    Ct->g = va_arg (args, int);
                    Ct->b = va_arg (args, int);
                    break;

                case BGetColor:
                    Ct = va_arg (args, Context *);
                    if (!Ct) returnv (1);

                    *va_arg (args, int *) = Ct->r;
                    *va_arg (args, int *) = Ct->g;
                    *va_arg (args, int *) = Ct->b;
                    break;

                case BEnableViewPort:
                    Ct = va_arg (args, Context *);
                    if (!Ct) returnv (1);

                    Ct->x1 = va_arg (args, int);
                    Ct->y1 = va_arg (args, int);
                    Ct->x2 = va_arg (args, int);
                    Ct->y2 = va_arg (args, int);

                    Ct->hasViewPort = 1;

                    break;

                case BDisableViewPort:
                    Ct = va_arg (args, Context *);
                    if (!Ct) returnv (1);

                    Ct->hasViewPort = 0;
                    break;

                case BSetSpaceLen:
                    Ct = va_arg (args, Context *);
                    if (!Ct) returnv (1);

                    Ct->spaceLen = va_arg (args, int);
                    break;

                case BGetSpaceLen:
                    Ct = va_arg (args, Context *);
                    if (!Ct) returnv (1);

                    returnv (Ct->spaceLen);

                case BWidth:
                    Ct = va_arg (args, Context *);
                    if (!Ct) break;

                    k = (va_arg (args, int) + 256) & 255;

                    if (k < Ct->startCh || k > Ct->endCh)
                        returnv (Ct->spaceLen);

                    returnv (*Ct->OffsTable [k - Ct->startCh]);

                case BHeight:
                    Ct = va_arg (args, Context *);
                    if (!Ct) break;

                    returnv (Ct->charHeight);

                case BStringWidth:
                    Ct = va_arg (args, Context *);
                    if (!Ct) break;

                    p = va_arg (args, unsigned char *);
                    if (!p) break;

                    j = 0;

                    while ((i = (unsigned char)*p++) != '\0')
                    {
                        if (i < Ct->startCh || i > Ct->endCh)
                            j += Ct->spaceLen;
                        else
                            j += *Ct->OffsTable [i - Ct->startCh];
                    }

                    returnv (j);

                case BPutC:
                    Ct = va_arg (args, Context *);
                    if (!Ct) break;

                    u = va_arg (args, int);
                    v = va_arg (args, int);

                    k = (va_arg (args, int) + 256) & 255;

                    if (k < Ct->startCh || k > Ct->endCh)
                        returnv (Ct->spaceLen);

                    p = Ct->OffsTable [k - Ct->startCh];
                    w = *p++;

                    k = *((unsigned short *)p)++;

                    if (Ct->hasViewPort)
                    {
                        while (k--)
                        {
                            i = *p++;
                            j = *p++;

                            a = u + i;
                            b = v + j;

                            if (Ct->x1 <= a && a <= Ct->x2 &&
                                Ct->y1 <= b && b <= Ct->y2)
                            {
                                Ct->gfx (GPutPixel, a, b, Ct->r, Ct->g, Ct->b);
                            }
                        }
                    }
                    else
                    {
                        while (k--)
                        {
                            i = *p++;
                            j = *p++;

                            Ct->gfx (GPutPixel, u + i, v + j, Ct->r, Ct->g, Ct->b);
                        }
                    }

                    returnv (w);

                case BPrintF:
                    Ct = va_arg (args, Context *);
                    if (!Ct) returnv (1);

                    i = va_arg (args, int);
                    j = va_arg (args, int);

                    p = va_arg (args, unsigned char *);
                    if (!p) returnv (1);

                    vsprintf ((char *)g__tempBuf, (const char *)p, args);

                    k = strlen ((char *)(p = g__tempBuf));

                    while (k--)
                        i += (int)bf2 (BPutC, Ct, i, j, *p++);

                    break;
            }

            return NULL;
    }

    int main (void)
    {
            registerInterface ("bf2-0.03", &bf2);
            setResident ();

            printf ("bf2-0.03\n");
            return 0;
    }
