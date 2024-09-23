/*
    KS-TGA.C

    Kalissa TGA Writer Version 0.04

    This is the Kalissa Writer, originally it was supposed to generate
    only TGA files, now it allows other formats but since I got used to
    call it TGA-Writer, I got stuck with the name ;)

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <glink.h>

    #include "ks-tga.h"
    #include "xmsh.h"

    /* Special pixel values for the writer. */
    #define W__FLUSH    (0xFF000000UL)
    #define W__RESET    (0xFE000000UL)

    /* Count of surfaces opened by the writer. */
    static unsigned surface_count = 0;

    /* Current output file for the writer. */
    static FILE *c_out;

    /* Given a number, returns a temporal filename. */
    static char *tmpfil (unsigned n)
    {
        static char buf [64];

            sprintf (buf, "$%u.tmp", n);

            return buf;
    }

    /* Write a tryad to the file. */
    static void WriteTryad (FILE *fp, long x)
    {
            putc (x, fp);
            putc (x >> 8, fp);
            putc (x >> 16, fp);
    }

    /* Write a tryad to the surface. */
    static void w__WriteTryad (const surface_t *s, long x)
    {
            if (s->powered_by_xms)
            {
                ((surface_t *)s)->wr.source_offs = (unsigned long)(&x);
                moveEMB_st ((void *)&s->wr);

                ((surface_t *)s)->wr.dest_offs += 4;
            }
            else
                WriteTryad (s->temp, x);
    }

    /* Reads a tryad from the surface. */
    static long ReadTryad (const surface_t *surface)
    {
        long a, b, c, p;

            if (!surface->powered_by_xms)
            {
                a = getc (surface->temp);
                b = getc (surface->temp);
                c = getc (surface->temp);

                return (c << 16UL) | (b << 8L) | a;
            }
            else
            {
                ((surface_t *)surface)->rd.dest_offs = (unsigned long)(&p);
                moveEMB_st ((void *)&surface->rd);

                ((surface_t *)surface)->rd.source_offs += 4;

                return p;
            }
    }

    /* Reads a tryad from the surface (separates RGB). */
    static void w__ReadTryad (const surface_t *s, int *r, int *g, int *b)
    {
        long p;

            if (!s->powered_by_xms)
            {
                *r = (unsigned char)getc (s->temp);
                *g = (unsigned char)getc (s->temp);
                *b = (unsigned char)getc (s->temp);
            }
            else
            {
                ((surface_t *)s)->rd.dest_offs = (unsigned long)(&p);
                moveEMB_st ((void *)&s->rd);

                ((surface_t *)s)->rd.source_offs += 4;

                *r = (unsigned char)(p >> 16);
                *g = (unsigned char)(p >> 8);
                *b = (unsigned char)p;
            }
    }

    /* Seeks to a given position on the temp stream. */
    static void seekto (const surface_t *s, long x)
    {
            if (!s->powered_by_xms)
                fseek (s->temp, x, SEEK_SET);
            else
            {
                ((surface_t *)s)->rd.source_offs = x;
                ((surface_t *)s)->wr.dest_offs = x;
            }
    }

    /* Writes N tryads with value X. */
    static void ClearSurface (surface_t *s, long n, long x)
    {
            seekto (s, 0);

            while (n--) w__WriteTryad (s, x);
    }

    /* Creates a surface for the writer. */
    const surface_t *w__CreateSurface (unsigned w, unsigned h, char *out,
                                       long rgbTryad)
    {
        surface_t *surface = new (surface_t);
        long size = (unsigned long)(w) * h;
        int usexms = 1;
        FILE *fp;

    retry:  if (checkXMS () || !usexms)
            {
                fp = fopen (tmpfil (surface_count), "w+b");
                if (fp == NULL)
                {
                    delete (surface);
                    return NULL;
                }
            }
            else
            {
                usexms = 0;

                surface->handle = allocEMB (size << 3L);
                if (!surface->handle) goto retry;

                surface->powered_by_xms = 1;

                surface->wr.dest_handle = surface->handle;
                surface->wr.source_handle = 0;
                surface->wr.dest_offs = 0;
                surface->wr.length = 4;

                surface->rd.source_handle = surface->handle;
                surface->rd.dest_handle = 0;
                surface->rd.source_offs = 0;
                surface->rd.length = 4;
            }

            surface->surface_id = surface_count++;
            surface->size = size;

            surface->height = h;
            surface->width = w;

            surface->output = out;
            surface->temp = fp;

            ClearSurface (surface, size, rgbTryad);

            return (const surface_t *)surface;
    }

    /* Writes a pixel in TGA format. */
    static void TGA__WritePixel (long pixel)
    {
        static unsigned count, level, i;
        static long buffer [128];
        static long cur_pixel;

            if (pixel == W__RESET)
            {
                cur_pixel = W__RESET;

                count = 0;
                level = 0;

                return;
            }

            if (pixel == W__FLUSH) goto Flush;

            if (cur_pixel == pixel && count < 128)
            {
                count++;
                return;
            }

            if (!count) goto DoneFlushing;

            if (count == 1 && level < 128)
            {
                buffer [level++] = cur_pixel;
            }
            else
            {
         Flush: if (level)
                {
                    putc (level - 1, c_out);
                    i = 0;

                    while (level--) WriteTryad (c_out, buffer [i++]);

                    level = 0;
                }

                if (count)
                {
                    putc ((count - 1) | 0x80U, c_out);
                    WriteTryad (c_out, cur_pixel);
                }
            }

            if (pixel == W__FLUSH) return;

        DoneFlushing:;
            cur_pixel = pixel;
            count = 1;
    }

    /* Writes a pixel in BMP format. */
    static void BMP__WritePixel (long pixel)
    {
            if (pixel == W__RESET || pixel == W__FLUSH) return;

            WriteTryad (c_out, pixel);
    }

    /* Writes a pixel to the TGA file. */
    static void WritePixel (const surface_t *surface, long pixel)
    {
            switch (surface->format)
            {
                case F__TGA:    TGA__WritePixel (pixel);
                                break;

                case F__BMP:    BMP__WritePixel (pixel);
                                break;
            }
    }

    static void putd (unsigned long x, FILE *fp)
    {
            putw (x, fp);
            putw (x >> 16, fp);
    }

    /* Starts the TGA writer. */
    static void StartWriter (const surface_t *surface)
    {
        long w, h;

            h = surface->height;
            w = surface->width;

            WritePixel (surface, W__RESET);

            switch (surface->format)
            {
                case F__TGA:
                    putw (0x00, c_out);
                    putc (0x0A, c_out);

                    putd (0x00, c_out);
                    putc (0x00, c_out);

                    putd (0x00, c_out);

                    putw (w, c_out);
                    putw (h, c_out);

                    putw (0x18, c_out);

                    break;

                case F__BMP:
                    putw ('BM', c_out);

                    putd (0x00, c_out); /* File Length */
                    putd (0x00, c_out);

                    putd (54, c_out);

                    putd (0x28, c_out);

                    putd (w, c_out);
                    putd (h, c_out);

                    putw (0x01, c_out);
                    putw (0x18, c_out);

                    putd (0, c_out);
                    putd (0, c_out);

                    putd (3700, c_out);
                    putd (3700, c_out);

                    putd (0, c_out);
                    putd (0, c_out);

                    break;
            }
    }

    /* Stops the TGA writer. */
    static void StopWriter (const surface_t *surface)
    {
        unsigned long t;

            WritePixel (surface, W__FLUSH);

            if (surface->format == F__BMP)
            {
                t = ftell (c_out);

                fseek (c_out, 2, SEEK_SET);
                putd (t, c_out);
            }
    }

    /* Closes the given surface. */
    void w__CloseSurface (const surface_t *surface)
    {
        long offs, llen;
        int x, y, pad;

            if (surface == NULL) return;

            if (surface->abort) goto Done;

            c_out = fopen (surface->output, "wb");
            if (c_out == NULL) goto Done;

            StartWriter (surface);

            if (surface->powered_by_xms)
            {
                llen = surface->width*4;
                offs = surface->size*4;
            }
            else
            {
                llen = surface->width*3;
                offs = surface->size*3;
            }

            if (surface->format == F__BMP)
                pad = surface->width & 3;
            else
                pad = 0;

            for (y = 0; y < surface->height; y++)
            {
                seekto (surface, offs -= llen);

                for (x = 0; x < surface->width; x++)
                {
                    WritePixel (surface, ReadTryad (surface));
                }

                for (x = 0; x < pad; x++) putc (0, c_out);
            }

            StopWriter (surface);

            fclose (c_out);

      Done: if (!surface->powered_by_xms)
            {
                fclose (surface->temp);
                remove (tmpfil (surface->surface_id));
            }
            else
                freeEMB (surface->handle);

            delete ((void *)(surface));
    }

    /* Returns the offset of a pixel. */
    static long Offset (const surface_t *s, long x, long y)
    {
            if (s->powered_by_xms)
                return ((y * s->width) + x) * 4;
            else
                return ((y * s->width) + x) * 3;
    }

    /* Writes a pixel at (x, y) of the surface. */
    void w__WritePixel (const surface_t *surface, int x, int y, long c)
    {
            if (surface == NULL) return;

            seekto (surface, Offset (surface, x, y));

            w__WriteTryad (surface, c);
    }

    /* Reads a pixel from (x, y). */
    void w__ReadPixel (const surface_t *surface, int x, int y, int *r,
                       int *g, int *b)
    {
            if (surface == NULL) return;

            seekto (surface, Offset (surface, x, y));

            w__ReadTryad (surface, r, g, b);
    }

    /* Writes a horizontal line at (x, y). */
    void w__HorzLine (const surface_t *surface, int x, int y, int w, long c)
    {
            if (surface == NULL) return;

            seekto (surface, Offset (surface, x, y));

            while (w--) w__WriteTryad (surface, c);
    }

    /* Writes a vertical line at (x, y). */
    void w__VertLine (const surface_t *surface, int x, int y, int h, long c)
    {
        long offs, wrap;

            if (surface == NULL) return;

            seekto (surface, offs = Offset (surface, x, y));

            if (surface->powered_by_xms)
                wrap = surface->width * 4;
            else
                wrap = surface->width * 3;

            while (h--)
            {
                w__WriteTryad (surface, c);

                seekto (surface, offs += wrap);
            }
    }
