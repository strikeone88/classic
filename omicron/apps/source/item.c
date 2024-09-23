/*
    ITEM.C

    Copyright (C) 2008 Novak Laboratories
    Written By J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         x, y, k, px, py, f, t, m, c, flags;
        anim_t      *anim;
    }
    item_t;

    #define Magic  ((rand () & p->m) - (rand () & p->m))

    static int handler (item_t *p, int m, ...)
    {
        int x, y, w, h;
        va_list args;

            va_start (args, m);

            if (m == m__collides)
            {
                x = va_arg (args, int);
                y = va_arg (args, int);
                w = va_arg (args, int);
                h = va_arg (args, int);

                return detectCollision (p->px - (p->anim->xres >> 1),
                                        p->py - (p->anim->yres >> 1),
                                        p->anim->xres,
                                        p->anim->yres,
                                        x, y, w, h);
            }

            if (m == m__getK) return p->k;

            if (m == m__destroy)
            {
                removeSprite (p);
                delete (p);

                return 0;
            }

            if (p->c && clock () >= p->c)
            {
                if (p->f == p->anim->fcount) p->f = 0;

                createTrail (p->x, p->y, p->f, p->anim, 24, 0);

                removeSprite (p);
                delete (p);

                return 0;
            }

            if (p->f == p->anim->fcount)
            {
                if (!p->c)
                {
                    removeSprite (p);
                    delete (p);

                    return 0;
                }

                p->f = 0;
            }

            if (p->flags & RANDOM)
            {
                x = p->px; y = p->py;
                drawFrame (p->px = p->x + Magic, p->py = p->y + Magic, p->anim, p->f++);
            }
            else
            {
                if (p->flags & RADIAL)
                {
                    x = p->x + rcost (p->m, p->t);
                    y = p->y - rsint (p->m, p->t);

                    drawFrame (p->px = x, p->py = y, p->anim, p->f++);
                    p->t++;
                }
                else
                {
                    x = p->px;
                    y = p->py;

                    drawFrame (p->px = p->x, p->py = p->y, p->anim, p->f++);
                }
            }

            return 0;
    }

    void createItem (int x, int y, int m, int c, int flags, int code, anim_t *anim, int k)
    {
        item_t *s;

            s = new (item_t);

            if (c) s->c = clock () + c;
            s->flags = flags;

            s->x = x;
            s->y = y;
            s->m = m;
            s->k = k;

            s->anim = anim;

            spriteHandler (s, handler);
            addSprite (s, code);
    }
