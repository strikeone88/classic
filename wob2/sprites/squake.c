/*
    SQUAKE.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    static int Scale, Mod, nextCicle;
    static int psX1, psY1;

    static int handler (void)
    {
            if (Scale > 0)
            {
                sX1 = dsX1 + (rand () % Mod) - (rand () % Mod);
                sY1 = dsY1 + (rand () % Mod) - (rand () % Mod);

                if (!nextCicle--)
                {
                    nextCicle = SQuakeDelay;
                    Scale -= SQuakeDec;

                    Mod = Scale << 2;
                }
            }

            return 0;
    }

    void createSpaceQuake (int scale)
    {
        static int already;
        sprite_t *s;

            Mod = (Scale = scale) << 4;

            psX1 = dsX1;
            psY1 = dsY1;

            nextCicle = 0;

            if (already) return;

            s = new (sprite_t);
            spriteHandler (s, handler);
            addSprite (s, t__squake);
    }
