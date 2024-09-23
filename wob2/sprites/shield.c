/*
    SHIELD.C

    Copyright (C) 2008 Novak Laboratories
    Written By J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         i, f, c, delay;
        void        *obj;
    }
    shield_t;

    /* Shield animation. */
    static anim_t *shield;

    /* Sprite limits and half-resolutions. */
    static int hw, hh, hyl, hxl, lxl, lyl;

    /* Shield half resolutions. */
    int shield__hw, shield__hh;

    static int handler (shield_t *p, int m)
    {
        int x, y;

            if (m == m__restartShield)
                return p->c = 0;

            x = sendMessage (p->obj, m__getX);
            y = sendMessage (p->obj, m__getY);

            drawFrame (x, y, shield, p->f + p->c);

            if (!p->delay--)
            {
                p->delay = ShieldDelay;
                p->c++;

                if (p->c == ShieldFadeC)
                {
                    sendMessage1 (p->obj, m__shieldOff, p->i);

                    removeSprite (p);
                    delete (p);
                }
            }

            return 0;
    }

    void createShield (int t, void *obj)
    {
        static int initialized = 0;
        shield_t *s;

            if (!useHQ)
            {
                shield__hw = 16;
                shield__hh = 16;

                return;
            }

            if (!initialized)
            {
                shield = nla__load ("data/shield.nla");
                if (!shield) animDie ("shield");

                shield__hw = hw = shield->xres >> 1;
                shield__hh = hh = shield->yres >> 1;

                hyl = sHeight - hh;
                hxl = sWidth - hw;

                lxl = hw;
                lyl = hh;

                initialized = 1;
            }

            if (t == -1) return;

            t = (t & 255) >> ShieldInc;

            if (!sendMessage1 (obj, m__acceptShield, t)) return;

            s = new (shield_t);

            s->f = (s->i = t) * ShieldFadeC;
            s->delay = ShieldDelay;
            s->c = 0;

            s->obj = obj;

            spriteHandler (s, handler);
            addSprite (s, t__back);
    }
