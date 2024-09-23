/*
    W2.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <conio.h>
    #include <game.h>

    #include "fontdata.h"

    ServiceInterface gfx, gdev, nla, bf2, skeyb;
    void *Ct;

    /* Global sprites. */
    void *player;

    /* Global text buffers. */
    char g__tempBuf [512], gbuf [512];

    /* Current virtual screen coords (and their original dups). */
    int sX1, sY1, dsX1, dsY1;

    /* Virtual screen end point. */
    int sX2, sY2;

    /* KeyMap. */
    char *keyMap;

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

            gfx (GSetDevice, NULL);
            gdev (GStopGfx);

            skeyb (KStop);

            va_start (p, fmts);

            if (flg) printf ("\n"); else flg = 1;

            switch (*fmts++)
            {
                case '*':   s = " (error)";
                            break;

                case '#':   s = " (warning)";
                            break;

                default:    s = "";
                            fmts--;
            }

            printf ("w2%s: ", s);

            vprintf (fmts, p);
    }

    /* Shows a message and terminates execution. */
    void animDie (char *s)
    {
            pmsg ("*can't load animation for '%s'", s);
            exit (3);
    }

    /* Returns true if the coordinate it not visible. */
    static int NotVisible (int x, int y)
    {
            return x < aX1 || y < aY1 || x > aX2 || y > aY2;
    }

    /* Fills a rectangle with color c. */
    void fillrect (int x, int y, int w, int h, int r, int g, int b)
    {
        int x2, y2;

            x2 = x + w - 1;
            y2 = y + h - 1;

            for (h = y; h <= y2; h++)
            for (w = x; w <= x2; w++)
                gfx (GPutPixel, w, h, r, g, b);
    }

    /* Draws a rectangle (outline). */
    void drawrect (int x, int y, int w, int h, int r, int g, int b)
    {
        int x2, y2;

            x2 = x + w - 1;
            y2 = y + h - 1;

            for (w = x; w <= x2; w++)
                gfx (GPutPixel, w, y, r, g, b), gfx (GPutPixel, w, y2, r, g, b);

            for (h = y; h <= y2; h++)
                gfx (GPutPixel, x, h, r, g, b), gfx (GPutPixel, x2, h, r, g, b);
    }

    /* Draws a frame in the virtual screen (works as nla__drawframe). */
    void drawFrame (int x, int y, anim_t *q, unsigned frame)
    {
            nla (NDrawFrame, q, frame, x - sX1 + aX1, y - sY1 + aY1);
    }

    /* Draws a frame statically. */
    void drawFrameS (int x, int y, anim_t *q, unsigned frame)
    {
            nla (NDrawFrame, q, frame, x + (q->xres >> 1), y + (q->yres >> 1));
    }

    /* Draws a frame in the virtual screen (works as nla__drawframe). */
    void drawFrameB (int x, int y, anim_t *q, unsigned char *buf)
    {
            nla (NDrawBuf, q, buf, x - sX1 + aX1, y - sY1 + aY1);
    }

    /* Same as above but you supply the sX1 and sY1. */
    void drawFrameWsx (int x, int y, anim_t *q, unsigned frame, int sX1, int sY1)
    {
            nla (NDrawFrame, q, frame, x - sX1 + aX1, y - sY1 + aY1);
    }

    /* Prints a string at (x, y) with color c. */
    void bprintf (int x, int y, char *s, int r, int g, int b, int sX1, int sY1)
    {
            x -= sX1 - aX1;
            y -= sY1 - aY1;

            if (NotVisible (x, y)) return;

            bf2 (BSetColor, Ct, r, g, b);
            bf2 (BPrintF, Ct, x, y, s);
    }

    /* Prints a string at (x, y) with color c. */
    void bprintfs (int x, int y, char *s, int r, int g, int b, int sX1, int sY1)
    {
            if (NotVisible (x, y)) return;

            bf2 (BSetColor, Ct, r, g, b);
            bf2 (BPrintF, Ct, x, y, s);
    }

    /* Plots a pixel. */
    void qputpixel (int x, int y, int c, int sX1, int sY1)
    {
            x -= sX1 - aX1;
            y -= sY1 - aY1;

            if (NotVisible (x, y)) return;

            gfx (GPutPixelP, x, y, &c);
    }

    /* Draws a blended line from (x1, y1) to (x2, y2) with color c. */
    void lineBlend (int x1, int y1, int x2, int y2, int r1, int g1, int b1,
                    int r2, int g2, int b2, int sX1, int sY1)
    {
        static long x, y, ix, iy, dx, dy;
        static int adx, ady, adz;

            x1 -= sX1 - aX1;
            x2 -= sX1 - aX1;
            y1 -= sY1 - aY1;
            y2 -= sY1 - aY1;

            if (NotVisible (x1, y1) || NotVisible (x2, y2)) return;

            dx = x2 - x1;
            dy = y2 - y1;

            x = x1;
            y = y1;

            adx = abs (dx);
            ady = abs (dy);

            dx <<= 7L;
            dy <<= 7L;

            x <<= 7L;
            y <<= 7L;

            adz = adx > ady ? adx : ady;
            if (!adz) return;

            ix = dx / adz;
            iy = dy / adz;

            while (adz--)
            {
                x1 = x >> 7L;
                y1 = y >> 7L;

                gfx (GAddPixel, x1, y1, r2, g2, b2);

                if (x1 > aX1) gfx (GAddPixel, x1 - 1, y1, r1, g1, b1);
                if (x1 < aX2) gfx (GAddPixel, x1 + 1, y1, r1, g1, b1);
                if (y1 > aY1) gfx (GAddPixel, x1, y1 - 1, r1, g1, b1);
                if (y1 < aY2) gfx (GAddPixel, x1, y1 + 1, r1, g1, b1);

                x += ix;
                y += iy;
            }
    }

    /* Forces visibility of an area with center (x, y) and length p. */
    void forceVisible (int x, int y, int p, int q, int loop)
    {
        int odsX1 = dsX1, odsY1 = dsY1;

            if (loop)
            {
                while (x - p < dsX1)
                {
                    dsX1 -= ScrollLengthX;
                    sX2 -= ScrollLengthX;
                }

                while (y - q < dsY1)
                {
                    dsY1 -= ScrollLengthY;
                    sY2 -= ScrollLengthY;
                }

                while (x + p > sX2)
                {
                    dsX1 += ScrollLengthX;
                    sX2 += ScrollLengthX;
                }

                while (y + q > sY2)
                {
                    dsY1 += ScrollLengthY;
                    sY2 += ScrollLengthY;
                }

                while (dsX1 < 0)
                {
                    p = -dsX1;
                    dsX1 += p;
                    sX2 += p;
                }

                while (dsY1 < 0)
                {
                    p = -dsY1;
                    dsY1 += p;
                    sY2 += p;
                }

                while (sX2 >= sWidth)
                {
                    p = sX2 - sWidth + 1;
                    dsX1 -= p;
                    sX2 -= p;
                }

                while (sY2 >= sHeight)
                {
                    p = sY2 - sHeight + 1;
                    dsY1 -= p;
                    sY2 -= p;
                }

            }
            else
            {
                if (x - p < dsX1)
                {
                    dsX1 -= ScrollLengthX;
                    sX2 -= ScrollLengthX;
                }

                if (y - q < dsY1)
                {
                    dsY1 -= ScrollLengthY;
                    sY2 -= ScrollLengthY;
                }

                if (x + p > sX2)
                {
                    dsX1 += ScrollLengthX;
                    sX2 += ScrollLengthX;
                }

                if (y + q > sY2)
                {
                    dsY1 += ScrollLengthY;
                    sY2 += ScrollLengthY;
                }

                if (dsX1 < 0)
                {
                    p = -dsX1;
                    dsX1 += p;
                    sX2 += p;
                }

                if (dsY1 < 0)
                {
                    p = -dsY1;
                    dsY1 += p;
                    sY2 += p;
                }

                if (sX2 >= sWidth)
                {
                    p = sX2 - sWidth + 1;
                    dsX1 -= p;
                    sX2 -= p;
                }

                if (sY2 >= sHeight)
                {
                    p = sY2 - sHeight + 1;
                    dsY1 -= p;
                    sY2 -= p;
                }
            }

            if (odsX1 != dsX1) sX1 = dsX1;
            if (odsY1 != dsY1) sY1 = dsY1;
    }

    void postFunc (void)
    {
        char *msg = "Press [ENTER] to Start (%u) %u";
        int x, y;

            x = (s__width - (int)bf2 (BStringWidth, Ct, msg)) >> 1;
            y = (s__height - (int)bf2 (BHeight, Ct)) >> 1;

                bf2 (BPrintF, Ct, x, y, msg);
                gfx (GShow);

            while (!keyMap [M__ENTER])
            {
            }

            while (keyMap [M__ENTER])
            {
            }
    }

/*****************************************************************************
*****************************************************************************/

    int main (void)
    {
        int i;

            gdev = getServiceInterface ("gdev16-0.01");
            if (!gdev) goto Done0;

            gfx = getServiceInterface ("gfx-0.01");
            if (!gfx) goto Done0;

            nla = getServiceInterface ("nla-0.01");
            if (!nla) goto Done0;

            bf2 = getServiceInterface ("bf2-0.03");
            if (!bf2) goto Done0;

            skeyb = getServiceInterface ("skeyb-0.01");
            if (!skeyb) goto Done0;

            Ct = bf2 (BCreateContext);
            bf2 (BSelectGfx, Ct, gfx);

            if (bf2 (BSetFont, Ct, fontdata, sizeof (fontdata))) goto Done0;

            skeyb (KStart);

            if (gdev (GStartGfx, s__width*s__height)) goto Done0;

            gfx (GSetDevice, gdev);

            if (nla (NSetGfx, gfx)) goto Done1;

            nla (NSetViewPort, aX1, aY1, aX2, aY2);
            bf2 (BEnableViewPort, Ct, 0, 0, aX2, aY2);

            sX1 = dsX1 = 0;
            sY1 = dsY1 = 0;

            sY2 = aHeight - 1;
            sX2 = aWidth - 1;

            keyMap = skeyb (KGetKeyMap);

            createScanner ();

            for (i = 0; i < 64; i++)
                createStar (rand () % aWidth, rand () % aHeight, 0);

            for (i = 0; i < 16; i++)
                createStar (rand () % ((MsX1 >> 3) + aWidth), rand () % ((MsY1 >> 3) + aHeight), 1);

            for (i = 0; i < 64; i++)
                createStar (rand () % ((MsX1 >> 2) + aWidth), rand () % ((MsY1 >> 2) + aHeight), 2);

            createShip (s__width / 2, s__height / 2);

            post = postFunc;
            startCicle ();

     Done2: skeyb (KStop);

     Done1: gfx (GSetDevice, NULL);
            gdev (GStopGfx);

     Done0: 
            return 0;
    }
