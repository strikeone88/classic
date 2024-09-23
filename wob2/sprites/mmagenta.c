/*
    MMAGENTA.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         ok, x, y, bx, by, f, p, t, r;

        int         m, b, s, c;

        int         frameDelay;
    }
    mmagenta_t;

    static anim_t *mmagenta;

    static int lxl, lyl, hxl, hyl, hw, hh;

    static int handler (mmagenta_t *p, int m, ...)
    {
        int x, y, x1, y1, w, h, b, i, t;
        static unsigned long r;
        static va_list args;
        static void *q;

            va_start (args, m);

            switch (m)
            {
                case m__damageLevel:
                    return 1;

                case m__hit:
                    if (!p->r--) return 1; else return 0;

                case m__suicide:
                case m__destroy:
           Destroy: if (useHQ && useHBoom)
                        createHBoom (p->x, p->y, HBoomBlue, HBoomHDelay, UNUSED, UNUSED, 255);
                    else
                        createBoom (p->x, p->y, UNUSED, UNUSED, 255);

                    if (score && m != m__suicide)
                    {
                        ((sprite_t *)score)->handler (score, m__addKill);

                        ((sprite_t *)score)->handler (score, m__getMultiplier, &r);
                        ((sprite_t *)score)->handler (score, m__addScore, r *= MMagentaPrice);

                        sprintf (g__tempBuf, "%lu", r);
                        createText (p->x, p->y, g__tempBuf, 255, 255, 255, TextDecF);
                    }

                    removeSprite (p);
                    delete (p);

                    break;

                case m__collides:
                    if (!p->ok) break;

                    x = va_arg (args, int);
                    y = va_arg (args, int);
                    x1 = va_arg (args, int);
                    y1 = va_arg (args, int);

                    return detectCollision (p->x - hw, p->y - hh,
                            mmagenta->xres, mmagenta->yres, x, y, x1, y1);

                case m__update:
                    goto Default;
            }

            return 0;

   Default: x = p->x;
            y = p->y;

            if (x < lxl) x = lxl;
            if (y < lyl) y = lyl;
            if (x > hxl) x = hxl;
            if (y > hyl) y = hyl;

            x1 = x - hw;
            y1 = y - hh;
            w = mmagenta->xres;
            h = mmagenta->yres;

            if ((q = spriteCollision (t__ship, p, x1, y1, w, h)) != NULL)
            {
                sendMessage (q, m__destroy);
                if (sendMessage (q, m__activeShield)) goto Destroy;
            }

            drawFrame (p->x = x, p->y = y, mmagenta, p->f);
            p->ok = 1;

            if (!p->frameDelay--)
            {
                p->frameDelay = MMagentaFDelay;

                p->f++;
                if (p->f >= mmagenta->fcount) p->f = 0;
            }

            p->x = p->bx + rcost (MMagentaBR, p->t);
            p->y = p->by - rsint (MMagentaBR, p->t);

            p->t = (p->t + MMagentaTI) & 255;

            if (!player) return 0;

            x1 = sendMessage (player, m__getX);
            y1 = sendMessage (player, m__getY);

            i = abs (x1 - x);
            m = abs (y1 - y);

            if (!i && !m) return 0;

            if (!p->p--)
            {
                if ((p->c = m > i) != 0)
                {
                    p->m = m = (x1 - x) / (y1 - y);
                    p->b = b = x - m*y;

                    if ((p->s = y1 < y) != 0)
                        p->bx = m * (p->by -= MMagentaSpeed) + b;
                    else
                        p->bx = m * (p->by += MMagentaSpeed) + b;
                }
                else
                {
                    p->m = m = (y1 - y) / (x1 - x);
                    p->b = b = y - m*x;

                    if ((p->s = x1 < x) != 0)
                        p->by = m * (p->bx -= MMagentaSpeed) + b;
                    else
                        p->by = m * (p->bx += MMagentaSpeed) + b;
                }

                p->p = MMagentaUpdate;
            }
            else
            {
                if (p->c)
                {
                    if (p->s)
                        p->bx = p->m * (p->by -= MMagentaSpeed) + p->b;
                    else
                        p->bx = p->m * (p->by += MMagentaSpeed) + p->b;
                }
                else
                {
                    if (p->s)
                        p->by = p->m * (p->bx -= MMagentaSpeed) + p->b;
                    else
                        p->by = p->m * (p->bx += MMagentaSpeed) + p->b;
                }
            }

            return 0;
    }

    void createMMagenta (int x, int y)
    {
        static int initialized = 0;
        mmagenta_t *s;

            if (!initialized)
            {
                if (useHQ)
                {
                    mmagenta = nla__load ("data/mmag.nla");
                    if (!mmagenta) animDie ("mmag");
                }
                else
                {
                    mmagenta = nla__load ("data/mmaglq.nla");
                    if (!mmagenta) animDie ("mmaglq");
                }

                hw = mmagenta->xres >> 1;
                hh = mmagenta->yres >> 1;

                hyl = sHeight - hh;
                hxl = sWidth - hw;

                lxl = hw;
                lyl = hh;

                initialized = 1;
            }

            if (x == -1) return;

            s = new (mmagenta_t);

            s->x = s->bx = x;
            s->y = s->by = y;

            s->frameDelay = MagentaFDelay;
            s->r = MMagentaR;

            spriteHandler (s, handler);
            addSprite (s, t__enemy);
    }
