/*
    SQUAKE.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    static int Scale = 0, nextCicle, t;
    static int psX1, psY1;

    static int handler (void)
    {
        int k;

            if (t > 0)
            {
                k = rsint (Scale, t);
                if (!k)
                {
                    Scale = 0;

                    sX1 = dsX1;
                    sY1 = dsY1;

                    t = 0;

                    return 0;
                }

                sX1 = dsX1 + (rand () % k) - (rand () % k);
                sY1 = dsY1 + (rand () % k) - (rand () % k);

                if (!nextCicle--)
                {
                    nextCicle = SQuakeDelay;
                    t -= 1;
                }
            }

            return 0;
    }

    void createSpaceQuake (int scale)
    {
        static int already;
        sprite_t *s;

            Scale = (Scale >> 2) + scale * 3;

            psX1 = dsX1;
            psY1 = dsY1;

            nextCicle = SQuakeDelay;
            t = _90deg;

            if (already) return;

            s = new (sprite_t);
            spriteHandler (s, handler);
            addSprite (s, t__squake);
    }
