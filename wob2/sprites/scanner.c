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

            x = (s__width - bf2__strlen (pause)) >> 1;
            y = (s__height - bf2__height ()) >> 1;

            bf2__printf (x, y, 0xFFFF, pause);

            while (keyMap [M__P]);
            while (!keyMap [M__P]);
            while (keyMap [M__P]);
    }

    static int handler (void)
    {
        static int i, x, y, cCyan, cMagenta, cEnemies;
        static unsigned long vscore;

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
                ((sprite_t *)score)->handler (score, m__resetScore);
                createShip (sWidth / 2, sHeight / 2);

                disablePlaySound ();
                broadcastMessage (t__enemy, m__suicide);
                enablePlaySound ();

                playSound (S__HEAL);
            }

            if (sdelay--) return 0;

            sdelay = ScannerDelay;

            if (spriteCount (t__enemy) < MinEnemies)
            {
                i = (rand () % MaxEnemies) + 1;

                cMagenta = 0;
                cCyan = 0;

                ((sprite_t *)score)->handler (score, m__getScore, &vscore);

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
