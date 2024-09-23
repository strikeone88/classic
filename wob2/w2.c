/*
    W2.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <fontdata.h>
    #include <stdarg.h>
    #include <alloc.h>
    #include <game.h>

    /* Global sprites. */
    void *player, *score;

    /* Global text buffers. */
    char g__tempBuf [512], gbuf [512];

    /* Current virtual screen coords (and their original dups). */
    int sX1, sY1, dsX1, dsY1;

    /* Virtual screen end point. */
    int sX2, sY2;

    /* Indicates we're using HQ graphics. */
    int useHQ;

    /* Indicates HBoom is enabled (requires HQ). */
    int useHBoom;

    /* Indices of loaded sounds. */
    static unsigned Sound [MAX_SOUNDS];

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

            stopGfx ();
            skeyb__stop ();

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
    void fillrect (int x, int y, int w, int h, int c)
    {
        int x2, y2;

            x2 = (x -= sX1 - aX1) + w - 1;
            y2 = (y -= sY1 - aY1) + h - 1;

            if (x < aX1) x = aX1;
            if (y < aY1) y = aY1;

            if (y2 >= aY2) y2 = aY2;
            if (x2 >= aX2) x2 = aX2;

            if (x2 <= x || y2 <= y) return;

            for (h = y; h <= y2; h++)
            for (w = x; w <= x2; w++)
                putpixel (w, h, c);
    }

    /* Draws a rectangle (outline). */
    void drawrect (int x, int y, int w, int h, int c)
    {
        int x2, y2;

            x2 = (x -= sX1 - aX1) + w - 1;
            y2 = (y -= sY1 - aY1) + h - 1;

            if (x < aX1) x = aX1;
            if (y < aY1) y = aY1;

            if (y2 >= aY2) y2 = aY2;
            if (x2 >= aX2) x2 = aX2;

            if (x2 <= x || y2 <= y) return;

            for (w = x; w <= x2; w++)
                putpixel (w, y, c), putpixel (w, y2, c);

            for (h = y; h <= y2; h++)
                putpixel (x, h, c), putpixel (x2, h, c);
    }

    /* Draws a frame in the virtual screen (works as nla__drawframe). */
    void drawFrame (int x, int y, anim_t *q, unsigned frame)
    {
            nla__drawframe (x - sX1 + aX1, y - sY1 + aY1, q, frame, aX1, aY1, aX2, aY2);
    }

    /* Same as above but you supply the sX1 and sY1. */
    void drawFrameWsx (int x, int y, anim_t *q, unsigned frame, int sX1, int sY1)
    {
            nla__drawframe (x - sX1 + aX1, y - sY1 + aY1, q, frame, aX1, aY1, aX2, aY2);
    }

    /* Prints a string at (x, y) with color c. */
    void bprintf (int x, int y, char *s, int c, int sX1, int sY1)
    {
            x -= sX1 - aX1;
            y -= sY1 - aY1;

            if (NotVisible (x, y)) return;

            bf2__print (x, y, c, s, aX1, aY1, aX2, aY2);
    }

    /* Plots a pixel. */
    void qputpixel (int x, int y, int c, int sX1, int sY1)
    {
            x -= sX1 - aX1;
            y -= sY1 - aY1;

            if (NotVisible (x, y)) return;

            putpixel (x, y, c);
    }

    /* Draws a line from (x1, y1) to (x2, y2) with color c. */
    void line (int x1, int y1, int x2, int y2, int c, int sX1, int sY1)
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
                putpixel (x >> 7L, y >> 7L, c);

                x += ix;
                y += iy;
            }
    }

    /* Draws a blended line from (x1, y1) to (x2, y2) with color c. */
    void lineBlend (int x1, int y1, int x2, int y2, int c, int c2, int sX1, int sY1)
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

                putpixelBlend (x1, y1, c2);

                if (x1 > aX1) putpixelBlend (x1 - 1, y1, c);
                if (x1 < aX2) putpixelBlend (x1 + 1, y1, c);
                if (y1 > aY1) putpixelBlend (x1, y1 - 1, c);
                if (y1 < aY2) putpixelBlend (x1, y1 + 1, c);

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

    /* Plays a sound given a symbolic index. */
    void playSound (unsigned n)
    {
            PlaySound (Sound [n]);
    }

    void postFunc (void)
    {
        char *msg = "Press [ENTER] to Start";
        int x, y;

            x = (s__width - bf2__strlen (msg)) >> 1;
            y = (s__height - bf2__height ()) >> 1;

            bf2__printf (x, y, 0xFFFF, msg);

            while (!keyMap [M__ENTER]);
            while (keyMap [M__ENTER]);

            bf2__printf (x, y, 0, msg);
    }

/*****************************************************************************
*****************************************************************************/

    void main (int argc, char *argv [])
    {
        int i;

            useHBoom = 0;
            useHQ = 0;

            for (i = 1; i < argc; i++)
            {
                if (!stricmp (argv [i], "hq"))
                    useHQ = 1;

                if (!stricmp (argv [i], "bq"))
                    useHQ = useHBoom = 1;
            }

            skeyb__start ();

            sX1 = dsX1 = 0;
            sY1 = dsY1 = 0;

            sY2 = aHeight - 1;
            sX2 = aWidth - 1;

            if (bf2__set (fontdata, sizeof (fontdata))) goto Done;

            if (startGfx ()) goto Done;

            StartAudio (0x240, 5, 1);

            Sound [S__EXP] = LoadSound ("data/snd/exp.pcm");
            Sound [S__SHOT] = LoadSound ("data/snd/shot.pcm");
            Sound [S__HIT] = LoadSound ("data/snd/hit.pcm");
            Sound [S__HEAL] = LoadSound ("data/snd/heal.pcm");

            createShip (-1);
            createShot (-1);
            createBoom (-1);
            createCyan (-1);
            createMMagenta (-1);
            createMagenta (-1);
            createStar (-1);
            createHBoom (-1);
            createShield (-1);

//            fillrect (0, 0, s__width - 1, s__height - 1, BarColor);
//            fillrect (aX1, aY1, aWidth, aHeight, 0);

            createScanner ();
            createScore ();

            for (i = 0; i < 64; i++)
                createStar (rand () % aWidth, rand () % aHeight, 0);

            for (i = 0; i < 16; i++)
                createStar (rand () % ((MsX1 >> 3) + aWidth), rand () % ((MsY1 >> 3) + aHeight), 1);

            for (i = 0; i < 64; i++)
                createStar (rand () % ((MsX1 >> 2) + aWidth), rand () % ((MsY1 >> 2) + aHeight), 2);

            createShip (s__width / 2, s__height / 2);

            post = postFunc;
            startCicle ();

      Done: stopGfx ();
            skeyb__stop ();
            StopAudio ();

            printf ("stats: %lu bytes (%lu kb) left.\n",
                coreleft (), coreleft () >> 10);
    }
