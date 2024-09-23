/*
    SCANNER.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    static int sdelay, MinEnemies, MaxEnemies;
    static unsigned long nextRaise;

    static void pauseMode (void)
    {
        static char *pause = "Pause";
        int x, y;

            x = (s__width - (int)bf2 (BStringWidth, Ct, pause)) >> 1;
            y = (s__height - (int)bf2 (BHeight, Ct)) >> 1;

            bf2 (BSetColor, Ct, 255, 255, 255);
            bf2 (BPrintF, Ct, x, y, pause);

            gfx (GShow);

            while (keyMap [M__P]);
            while (!keyMap [M__P]);
            while (keyMap [M__P]);
    }

    static int handler (void)
    {
        static int i, x, y, cCyan, cMagenta, cEnemies;
        static unsigned long vscore = 0;

            if (player)
            {
                if (clock () >= nextRaise)
                {
                    MaxEnemies += RaiseIndex;
                    MinEnemies += RaiseIndex;

                    if (MinEnemies > MaxMinEnemies) MinEnemies = MaxMinEnemies;
                    if (MaxEnemies > MaxMaxEnemies) MaxEnemies = MaxMaxEnemies;

                    nextRaise = clock () + EnemyRaise;
                }
            }

            if (keyMap [M__ESC]) return r__abort;

            if (keyMap [M__P]) post = pauseMode;

            if (keyMap [M__ENTER] && !player)
            {
                MinEnemies = InitMinEnemies;
                MaxEnemies = InitMaxEnemies;

                createShip (sWidth / 2, sHeight / 2);

                broadcastMessage (t__enemy, m__suicide);
            }

            if (sdelay--) return 0;

            sdelay = ScannerDelay;

            if (spriteCount (t__enemy) < MinEnemies)
            {
                i = (rand () % MaxEnemies) + 1;

                cMagenta = 0;
                cCyan = 0;

                if (vscore > 5000)
                    cEnemies = 1;
                else
                    cEnemies = 0;

                while (i)
                {
                    x = rand () & sHeightMask;
                    y = rand () & sWidthMask;

                    switch (rand () & cEnemies)
                    {
                        case 0:     createCyan (x, y);
                                    cCyan++;
                                    i--;
                                    break;

                        case 1:     createMagenta (x, y);
                                    cMagenta++;
                                    i--;
                                    break;
                    }

                    if (cMagenta > maxMagenta)
                    {
                        cMagenta = 0;
                        cEnemies--;
                    }
                }
            }

            return 0;
    }

    void createScanner (void)
    {
        static int already = 0;
        sprite_t *s;

            if (already) return;

            s = new (sprite_t);
            spriteHandler (s, handler);
            addSprite (s, t__scanner);

            MinEnemies = InitMinEnemies;
            MaxEnemies = InitMaxEnemies;

            sdelay = ScannerDelay;

            nextRaise = clock () + EnemyRaise;
    }
