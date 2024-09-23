/*
    SCORE.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <alloc.h>
    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        unsigned    long kills, multiplier, prevmult;
        unsigned    long score;

        unsigned    kMult, kShield, kBomb, kShip, kWeapon;
    }
    score_t;

    static int tx, ty, sdelay;

    static int handler (score_t *p, int m, ...)
    {
        static int x, y, b;
        static va_list args;

            va_start (args, m);

            switch (m)
            {
                case m__update:
                    if (player)
                    {
                        x = sendMessage (player, m__getShips);
                        b = sendMessage (player, m__getBombs);
                    }
                    else
                        x = b = 0;

                    bf2__printf (tx, ty, 0xFFFF, "Score: %lu   Kills: %lu  Ships: %u  Bombs: %u   [%u FPS, %lu kb]", p->score, p->kills, x, b, gfx__fps, coreleft () >> 10);

                    break;

                case m__addScore:
                    p->score += va_arg (args, unsigned long);
                    break;

                case m__getScore:
                    *va_arg (args, unsigned long *) = p->score;
                    break;

                case m__getMultiplier:
                    *va_arg (args, unsigned long *) = p->multiplier;
                    break;

                case m__resetCounters:
                    p->kMult = p->kShield = p->kBomb = p->kShip = p->kWeapon = 0;
                    p->multiplier = 1;
                    break;

                case m__resetScore:
                    p->score = 0;
                    break;

                case m__addKill:
                    p->kills++;

                    if (++p->kMult >= MultRaise && player)
                    {
                        p->multiplier += 2;
                        if (p->multiplier > MaxMult) p->multiplier = MaxMult;

                        if (p->prevmult == p->multiplier) break;

                        p->prevmult = p->multiplier;

                        p->kMult = 0;

                        x = sendMessage (player, m__getX);
                        y = sendMessage (player, m__getY);

                        sprintf (g__tempBuf, "%lux Multiplier", p->multiplier);
                        createText (x, y, g__tempBuf, 255, 255, 0, TextDecSS);
                    }

                    if (++p->kShield >= ShieldRaise && player)
                    {
                        sendMessage1 (player, m__addShieldEnergy, ShieldIndex);
                        p->kShield = 0;
                    }

                    if (++p->kBomb >= BombRaise && player)
                    {
                        p->kBomb = 0;

                        if (sendMessage (player, m__getBombs) < MaxBombs)
                        {
                            ((sprite_t *)player)->handler (player, m__addBomb);

                            x = sendMessage (player, m__getX);
                            y = sendMessage (player, m__getY);

                            createText (x, y, "Gained Another Bomb!", 255, 128, 0, TextDecS);
                        }
                    }

                    if (++p->kShip >= ShipRaise && player)
                    {
                        p->kShip = 0;

                        if (sendMessage (player, m__getShips) < MaxShips)
                        {
                            ((sprite_t *)player)->handler (player, m__addShip);

                            x = sendMessage (player, m__getX);
                            y = sendMessage (player, m__getY);

                            createText (x, y, "Gained Another Ship!", 255, 128, 0, TextDecS);
                        }
                    }

                    if (++p->kWeapon >= WeaponRaise && player)
                    {
                        p->kWeapon = 0;

                        if ((x = sendMessage (player, m__getWeapons)) < MaxWeapons)
                        {
                            sendMessage1 (player, m__setWeapons, x + 1);
                            sendMessage1 (player, m__setWeapon, x);
                        }
                    }

                    break;
            }

            return 0;
    }

    void createScore (void)
    {
        static int already = 0;
        score_t *s;

            if (already) return;

            s = new (score_t);
            spriteHandler (s, handler);
            addSprite (s, t__score);

            s->multiplier = 1;
            s->prevmult = 1;

            score = s;

            ty = (aY1 - bf2__height ()) >> 1;
            tx = 5;
    }
