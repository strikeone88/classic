/*
    GDEV16.C

    Graphics Device Controller for 16-bit Color Modes

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <gdev.h>
    #include <ifm.h>

    /* Real mode buffer. */
    #define RBuf        0x90000
    #define RBufSeg     0x9000
    #define RBufOff     0x0000

    /* Global pack of registers. */
    static REGPACK regs;

    /* Packs a video buffer into a R5G6B5 page. */
    void *packto565 (void *, void *, unsigned);

    void FlipFrameBuffer (void *);

    /* VESA VBE Mode Number. */
    int modeNumber;

    /* Video mode resolution. */
    int PageWeight, BufWeight;
    int ScrWidth;
    int ScrHeight;

    void clearBuf (void *, int);

    void *gdev16 (unsigned Command, ...)
    {
        static void *buf, *page;
        static va_list args;

            va_start (args, Command);

            switch (Command)
            {
                case GStartGfx:
                    switch (va_arg (args, int))
                    {
                        case 640*480:
                            modeNumber = 0x0111;
                            ScrWidth = 640;
                            ScrHeight = 480;
                            break;

                        default:
                            returnv (2);
                    }

                    BufWeight = ScrWidth * ScrHeight * 4 / 8;
                    PageWeight = ScrWidth * ScrHeight * 2 / 8;

                    regs.bx = modeNumber;
                    regs.ax = 0x4F02;
                    v86int (0x10, &regs);
                    returnv (regs.ax != 0x4F);

                case GStopGfx:
                    regs.bx = 0x0003;
                    regs.ax = 0x4F02;
                    v86int (0x10, &regs);
                    returnv (regs.ax != 0x4F);

                case GAllocateBuffer:
                    return dmemset (malloc (ScrWidth * ScrHeight * 4), 0, ScrWidth * ScrHeight * 4);

                case GReleasePage:
                case GReleaseBuffer:
                    buf = va_arg (args, void *);
                    if (buf) free (buf);
                    break;

                case GAllocatePage:
                    return dmemset (malloc (ScrWidth * ScrHeight * 2), 0, ScrWidth * ScrHeight * 2);

                case GScreenWidth:
                    returnv (ScrWidth);

                case GScreenHeight:
                    returnv (ScrHeight);

                case GPackBuffer:
                    return packto565 (va_arg (args, void *), va_arg (args, void *), ScrWidth * ScrHeight);

                case GShowPage:
                    page = va_arg (args, void *);
                    FlipFrameBuffer (page);
                    break;

                case GSyncShowPage:
                    page = va_arg (args, void *);
                    while (inportb (0x3DA) & 8);
                    FlipFrameBuffer (page);
                    break;

                case GClearBuffer:
                    buf = va_arg (args, void *);
                    if (buf) clearBuf (buf, BufWeight);
                    break;

                case GClearPage:
                    page = va_arg (args, void *);
                    if (page) clearBuf (page, PageWeight);
                    break;
            }

            return NULL;
    }

    int main (void)
    {
            registerInterface ("gdev16-0.01", &gdev16);

            setResident ();

            printf ("gdev16-0.01\n");
            return 0;
    }
