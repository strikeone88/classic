/*
    EXP.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         rad, max, inc;
        int         x, y;
    }
    explosion_t;

    static int handler (explosion_t *p, int m)
    {
            return 0;
    }

    void createExplostion (int x, int y, int rad, int maxrad, int inc)
    {
        explosion_t *s;

            s = new (explosion_t);

            spriteHandler (s, handler);
            addSprite (s, t__back);
    }
