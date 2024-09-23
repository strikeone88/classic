/*
    TRAIL.C

    Copyright (C) 2008 Novak Laboratories
    Written By J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         x, y, i, t;
        unsigned    count;

        unsigned    char *buf;
        anim_t      *anim;
    }
    trail_t;

    static void decrease (unsigned char *p, int count, int factor)
    {
        int i;

            while (count--)
            {
                i = *p - factor; *p++ = max (i, 0);
                i = *p - factor; *p++ = max (i, 0);
                i = *p - factor; *p++ = max (i, 0);
                p++;
            }
    }

    static int handler (trail_t *p)
    {
            drawFrameB (p->x, p->y, p->anim, p->buf);

            decrease (p->buf, p->count, p->i);
            p->t += p->i;

            if (p->t > 250)
            {
                removeSprite (p);

                delete (p->buf);
                delete (p);
            }

            return 0;
    }

    void createTrail (int x, int y, int frame, anim_t *anim, int i, int p)
    {
        trail_t *s;
        int count;

            s = new (trail_t);

            count = (s->count = anim->xres * anim->yres) * 4;
            dmemcpy (s->buf = malloc (count), anim->frames [frame], count);

            decrease (s->buf, s->count, p);

            s->x = x;
            s->y = y;
            s->i = i;
            s->t = p;

            s->anim = anim;

            spriteHandler (s, handler);
            addSprite (s, t__back);
    }
