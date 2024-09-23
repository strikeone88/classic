/*
    GFX.C

    Generic Graphics Engine Version 0.01

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <gfx.h>

    unsigned LineLen, Width, Height;
    unsigned char *Buffer, *Page;

    ServiceInterface gdev = NULL;

    void addBufferF (void *, void *, int, int, int, int);
    void addBuffer (void *, void *, int, int, int);

    void *gfx (unsigned Command, ...)
    {
        static int x, y, r, g, b, w, h;
        static unsigned char *p, *q;
        static int *rp, *gp, *bp;
        static va_list args;

            va_start (args, Command);

            switch (Command)
            {
                case GSetDevice:
                    if (gdev)
                    {
                        gdev (GReleaseBuffer, Buffer);
                        gdev (GReleasePage, Page);
                    }

                    gdev = va_arg (args, ServiceInterface);
                    if (!gdev) returnv (1);

                    Buffer = gdev (GAllocateBuffer);
                    Page = gdev (GAllocatePage);

                    Height = (unsigned)gdev (GScreenHeight);
                    Width = (unsigned)gdev (GScreenWidth);

                    LineLen = Width * 4;

                    break;

                case GGetDevice:
                    return gdev;

                case GPutPixel:
                    x = va_arg (args, int);
                    y = va_arg (args, int);

                    r = va_arg (args, int);
                    g = va_arg (args, int);
                    b = va_arg (args, int);

                    p = Buffer + y*LineLen + (x << 2);

                    *p++ = r, *p++ = g, *p++ = b;
                    break;

                case GPutPixelP:
                    x = va_arg (args, int);
                    y = va_arg (args, int);
                    q = va_arg (args, unsigned char *);

                    p = Buffer + y*LineLen + (x << 2);
                    *(unsigned *)p = *(unsigned *)q;
                    break;

                case GGetPixel:
                    x = va_arg (args, int);
                    y = va_arg (args, int);
                    rp = va_arg (args, int *);
                    gp = va_arg (args, int *);
                    bp = va_arg (args, int *);

                    p = Buffer + y*LineLen + (x << 2);

                    *rp = *p++, *gp = *p++, *bp = *p++;
                    break;

                case GGetPixelP:
                    x = va_arg (args, int);
                    y = va_arg (args, int);
                    p = va_arg (args, unsigned char *);

                    q = Buffer + y*LineLen + (x << 2);
                    *(unsigned *)p = *(unsigned *)q;
                    break;

                case GAddPixel:
                    x = va_arg (args, int);
                    y = va_arg (args, int);
                    r = va_arg (args, int) & 0xFF;
                    g = va_arg (args, int) & 0xFF;
                    b = va_arg (args, int) & 0xFF;

                    p = q = Buffer + y*LineLen + (x << 2);

                    r += *p++, g += *p++, b += *p++;

                    *q++ = min (r, 255);
                    *q++ = min (g, 255);
                    *q++ = min (b, 255);
                    break;

                case GAddPixelP:
                    x = va_arg (args, int);
                    y = va_arg (args, int);
                    q = va_arg (args, unsigned char *);

                    p = Buffer + y*LineLen + (x << 2);

                    r = *p + (*q++ & 0xFF); *p++ = min (r, 255);
                    r = *p + (*q++ & 0xFF); *p++ = min (r, 255);
                    r = *p + (*q++ & 0xFF); *p++ = min (r, 255);
                    break;

                case GSubPixelP:
                    x = va_arg (args, int);
                    y = va_arg (args, int);
                    q = va_arg (args, unsigned char *);

                    p = Buffer + y*LineLen + (x << 2);

                    r = *p - *q++; *p++ = max (r, 0);
                    r = *p - *q++; *p++ = max (r, 0);
                    r = *p - *q++; *p++ = max (r, 0);
                    break;

                case GGetBuffer:
                    return Buffer;

                case GGetPage:
                    return Page;

                case GShow:
                    gdev (GPackBuffer, Buffer, Page);
                    gdev (GShowPage, Page);
                    break;

                case GSyncShow:
                    gdev (GPackBuffer, Buffer, Page);
                    gdev (GSyncShowPage, Page);
                    break;

                case GClearScreen:
                    gdev (GClearPage, Page);
                    gdev (GSyncShowPage, Page);
                    break;

                case GClear:
                    gdev (GClearBuffer, Buffer);
                    break;

                case GAddBuffer:
                    x = va_arg (args, int);
                    y = va_arg (args, int);

                    w = va_arg (args, int);
                    h = va_arg (args, int);

                    p = Buffer + y*LineLen + (x << 2);
                    q = va_arg (args, unsigned char *);

                    addBuffer (p, q, w, h, (Width - w) << 2);
                    break;

                case GPutBuffer:
                    x = va_arg (args, int);
                    y = va_arg (args, int);

                    w = va_arg (args, int);
                    h = va_arg (args, int);

                    p = Buffer + y*LineLen + (x << 2);
                    q = va_arg (args, unsigned char *);

                    r = (Width - w) << 2;
                    w <<= 2;

                    while (h--)
                    {
                        dmemcpy (p, q, w);
                        p += r;
                    }

                    break;

                case GAddBufferF:
                    x = va_arg (args, int);
                    y = va_arg (args, int);

                    w = va_arg (args, int);
                    h = va_arg (args, int);

                    p = Buffer + y*LineLen + (x << 2);
                    q = va_arg (args, unsigned char *);

                    addBufferF (p, q, w, h, (Width - w) << 2, va_arg (args, int));
                    break;

            }

            return NULL;
    }

    int main (void)
    {
            registerInterface ("gfx-0.01", &gfx);

            setResident ();

            printf ("gfx-0.01\n");
            return 0;
    }
