/*
    TR.C

    Trillian Virtual Address Space Engine Version 0.01

    Copyright (C) 2007 RedStar Technologies
    Written by J. Palencia (zipox@ureach.com)
*/

    #include <alloc.h>
    #include <glink.h>
    #include <stdio.h>
    #include <tr.h>

    /* The 'offset' type, just a 32-bit unsigned integer. */
    #define offset_t    unsigned long

    /* I call a "slice" a portion of RAM (when using VM). */
    typedef struct
    {
        linkable_t  link;

        offset_t    offset;
        unsigned    len;
        char        invm;

        void        *data;
    }
    slice_t;

    /* Prints a message on the screen. */
    void pmsg (const char *s, ...);

    /* The storage file for the data. */
    static FILE *file = NULL;

    /* Offset to the bottom and dead-list. */
    static offset_t bottom, deadlst;

    /* Maximum memory for trillian. */
    static offset_t TR_MAX_MEM;

    /* Indicates whether to destroy the temporal file or not. */
    int tr__remove;

    /* The name of file used for storage. */
    static const char *fname;

    /* The read function used by this module (MUST BE HOLLOW SLICE). */
    static int tr__read (slice_t *slice)
    {
            if (slice->offset >= TR_MAX_MEM) return 0;

            fseek (file, slice->offset, SEEK_SET);

            if (fread (&slice->len, 2, 1, file) != 1)
                return 0;

            slice->data = alloc (slice->len + 4);

            *((void **)slice->data) = slice;

            if (fread ((char *)slice->data + 4, slice->len, 1, file) != 1)
            {
                delete (slice->data);
                return 0;
            }
            else
                return 1;
    }

    /* Write function used by this module (MUST BE FULL SLICE). */
    static int tr__write (slice_t *slice)
    {
            if (slice->offset >= TR_MAX_MEM) return 0;

            fseek (file, slice->offset, SEEK_SET);

            if (fwrite (&slice->len, 2, 1, file) != 1)
                return 0;

            if (fwrite ((char *)slice->data + 4, slice->len, 1, file) != 1)
                return 0;
            else
                return 1;
    }

    /* Writes the next-dead field in the entry. */
    static int tr__wrdead (offset_t p, offset_t x)
    {
            if (p >= TR_MAX_MEM) return 0;

            fseek (file, p + 2, SEEK_SET);

            if (fwrite (&x, 4, 1, file) != 1)
                return 0;

            return 1;
    }

    /* Reads the next-dead field and the length of the entry. */
    static int tr__rddead (offset_t p, offset_t *x, unsigned *c)
    {
            if (p >= TR_MAX_MEM) return 0;

            fseek (file, p, SEEK_SET);

            if (fread (c, 2, 1, file) != 1)
                return 0;

            if (fread (x, 4, 1, file) != 1)
                return 0;

            return 1;
    }

    /* Reserves a block on virtual memory. */
    static int reserve (unsigned len, offset_t *offs)
    {
        offset_t prev, cur, next;
        unsigned l;

            if (len < 4) len = 4;

            cur = deadlst;
            prev = 0;

            while (cur)
            {
                if (!tr__rddead (cur, &next, &l)) break;

                if (l >= len)
                {
                    if (prev)
                    {
                        if (!tr__wrdead (prev, next)) break;
                    }
                    else
                        deadlst = next;

                    *offs = cur;

                    return 1;
                }

                prev = cur;
                cur = next;
            }

            if (bottom + (len += 2) >= TR_MAX_MEM) return 0;

            *offs = bottom;
            bottom += len;

            return 1;
    }

    /* Starts Trillian, returns zero if no error occurred (max is in mb). */
    int tr__start (const char *_fname, unsigned max)
    {
            if (file != NULL) return 0;

            file = fopen (fname = _fname, "w+b");
            if (file == NULL) return 2;

            if (max > 256) max = 256;
            if (max < 1) max = 1;

            TR_MAX_MEM = max;
            TR_MAX_MEM <<= 20;

            tr__remove = 1;

            deadlst = 0;
            bottom = 16;

            return 0;
    }

    /* Stops Trillian. */
    void tr__stop (void)
    {
            if (file == NULL) return;

            fclose (file);
            file = NULL;

            if (tr__remove) remove (fname);
    }

    /* Saves the context of the current space.  */
    void tr__save (spacectx_t *p)
    {
            if (p == NULL) return;

            p->TR_MAX_MEM = TR_MAX_MEM;
            p->tr__remove = tr__remove;
            p->deadlst = deadlst;
            p->bottom = bottom;
            p->fname = fname;
            p->file = file;
    }

    /* Restore the context of the current space. */
    void tr__restore (spacectx_t *p)
    {
            if (p == NULL) return;

            TR_MAX_MEM = p->TR_MAX_MEM;
            tr__remove = p->tr__remove;
            deadlst = p->deadlst;
            bottom = p->bottom;
            fname = p->fname;
            file = p->file;
    }

    /* Allocates a new block and opens it automatically. */
    void *tr__alloc (unsigned len)
    {
        slice_t *slice;

            if (file == NULL) return NULL;

            slice = new (slice_t);
            slice->len = len;

            slice->data = alloc (len + 4);

            if (coreleft () < 96*1024UL)
            {
                slice->invm = 1;

                if (!reserve (len, &slice->offset))
                {
                    delete (slice->data);
                    delete (slice);
                    return NULL;
                }
            }
            else
                slice->invm = 0;

            *((void **)slice->data) = slice;

            return (char *)slice->data + 4;
    }

    /* Closes a slice (converts to block). */
    void tr__close (void **p, int flush)
    {
        slice_t *slice;

            if (file == NULL || p == NULL || *p == NULL) return;

            slice = *(void **)((char *)*p - 4);

            if (!slice->invm) return;

            if (flush)
            {
                if (!tr__write (slice))
                {
                    pmsg ("*unable to write back slice to %08lX.",
                            slice->offset);

                    exit (9);
                }
            }

            *p = (void *)(slice->offset | 0xA0000000);

            delete (slice->data);
            delete (slice);
    }

    /* Opens a block to be used as a slice. */
    void tr__open (void **p)
    {
        slice_t *slice;
        offset_t q;

            if (file == NULL || p == NULL || *p == NULL) return;

            q = (offset_t)*p;
            if ((q & 0xF0000000) != 0xA0000000) return;

            slice = new (slice_t);
            slice->offset = q & 0x0FFFFFFF;
            slice->invm = 1;

            if (!tr__read (slice))
            {
                pmsg ("*unable to read block from %08lX.", slice->offset);
                exit (9);
            }

            *p = (void *)((char *)slice->data + 4);
    }

    /* Releases a slice. */
    void tr__free (void *p)
    {
        slice_t *slice;

            if (file == NULL || p == NULL) return;

            slice = *(void **)((char *)p - 4);

            if (!slice->invm)
            {
                delete (slice->data);
                delete (slice);

                return;
            }

            *(offset_t *)p = deadlst;

            tr__close (&p, 1);

            deadlst = (offset_t)p;
    }

    /* Releases a block. */
    void tr__freeb (void *p)
    {
        slice_t *slice;

            if (file == NULL || p == NULL) return;

            if (((offset_t)p & 0xF0000000) != 0xA0000000)
            {
                slice = *(void **)((char *)p - 4);

                delete (slice->data);
                delete (slice);
                return;
            }

            if (!tr__wrdead ((offset_t)p, deadlst))
            {
                pmsg ("*unable to write dead link to %08lX.", p);
                exit (9);
            }

            deadlst = (offset_t)p;
    }

    /* Returns the memory left in the virtual space. */
    unsigned long tr__coreleft (void)
    {
            return TR_MAX_MEM - bottom;
    }
