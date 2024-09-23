/*
    SHIP.C

    Copyright (C) 2008 Novak Laboratories
    Written By J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>
    #include <math.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         ok, x, y, t;

        int         nextshot, delay;
        int         shield;

        int         ships, bombs;
        int         weapons, weapon;

        int         altDown, spaceDown, eDown;

        void        *shieldP [256 >> ShieldInc];
        int         shieldEnabled, shieldEnergy;
    }
    ship_t;

    static anim_t *ship = NULL, *sship = NULL;

    static int lxl, lyl, hxl, hyl, hw, hh;

    static void updateAngle4 (int *t, int I, int II, int III, int IV)
    {
            switch (*t >> 6)
            {
                case 0:
                    (*t) += I;
                    break;

                case 1:
                    (*t) += II;
                    break;

                case 2:
                    (*t) += III;
                    break;

                case 3:
                    (*t) += IV;
                    break;
            }

            (*t) = ((*t) + 256) & 255;
    }

    static void updateAngle_2a (int *t, int I, int II)
    {
            if (_135deg <= *t && *t <= _315deg)
                (*t) += II;
            else
                (*t) += I;

            (*t) = ((*t) + 256) & 255;
    }

    static void updateAngle_2b (int *t, int I, int II)
    {
            if (_45deg <= *t && *t <= _225deg)
                (*t) += II;
            else
                (*t) += I;

            (*t) = ((*t) + 256) & 255;
    }

    static int handler (ship_t *p, int m)
    {
        static int x, y, w, h, t1, t2;
        static unsigned long vscore;
        static va_list args;

            va_start (args, m);

            switch (m)
            {
                case m__activeShield:
                    return p->shieldEnabled && p->shieldEnergy;

                case m__getShieldState:
                    return p->shieldEnabled;

                case m__addShieldEnergy:
                    p->shieldEnergy += va_arg (args, unsigned);

                    if (p->shieldEnergy > MaxShield) p->shieldEnergy = MaxShield;
                    break;

                case m__getShieldEnergy:
                    return p->shieldEnergy;

                case m__shieldOff:
                    p->shieldP [va_arg (args, unsigned)] = NULL;
                    break;

                case m__acceptShield:
                    if (!p->shieldP [x = va_arg (args, unsigned)])
                        return 1;

                    sendMessage (p->shieldP [x], m__restartShield);
                    break;

                case m__getBombs:
                    return p->bombs;

                case m__addBomb:
                    p->bombs++;
                    break;

                case m__getWeapon:
                    return p->weapon;

                case m__setWeapon:
                    p->weapon = va_arg (args, unsigned) % p->weapons;
                    p->delay = getWeaponDelay (p->weapon);
                    break;

                case m__getWeapons:
                    return p->weapons;

                case m__setWeapons:
                    p->weapons = va_arg (args, unsigned);
                    break;

                case m__getShips:
                    return p->ships;

                case m__addShip:
                    p->ships++;
                    break;

                case m__destroy:
                    if (p->shield || (p->shieldEnergy && p->shieldEnabled))
                        break;

                    if (p->ships)
                    {
                        if (useHQ && useHBoom)
                            createHBoom (p->x, p->y, HBoomWhite, HBoomHDelay, 255, 255, 255);
                        else
                            createBoom (p->x, p->y, 255, 255, 255);

                        ((sprite_t *)score)->handler (score, m__resetCounters);

                        p->shield = ShieldEnergy;
                        p->ships--;

                        if (!p->ships)
                        {
                            createText (x, y, "!!!WARNING LAST SHIP!!!", 255, 0, 0, TextDecS);
                        }
                        else
                        {
                            if (p->ships == 1)
                            {
                                createText (x, y, "WARNING: One Ship Left", 255, 128, 0, TextDecS);
                            }
                            else
                            {
                                sprintf (gbuf, "Warning: %u ships left", p->ships);
                                createText (x, y, gbuf, 255, 255, 0, TextDecS);
                            }
                        }

                        break;
                    }

                    createText (x, y, "!! You're Dead !!", 255, 128, 0, TextDecSS);

                    if (useHQ && useHBoom)
                        createHBoom (p->x, p->y, HBoomWhite, HBoomHDelay, 255, 255, 255);
                    else
                        createBoom (p->x, p->y, 255, 255, 255);

                    ((sprite_t *)score)->handler (score, m__resetCounters);

                    removeSprite (p);
                    delete (p);

                    player = NULL;
                    break;

                case m__collides:
                    if (!p->ok) break;

                    if (p->shieldEnergy && p->shieldEnabled)
                    {
                        x = va_arg (args, int);
                        y = va_arg (args, int);
                        w = va_arg (args, int);
                        h = va_arg (args, int);

                        if (detectCollision (p->x - shield__hw, p->y - shield__hh,
                                shield__hw << 1, shield__hh << 1, x, y, w, h))
                        {
                            t1 = p->y - sendMessage (cSprite, m__getY);
                            t2 = sendMessage (cSprite, m__getX) - p->x;

                            if (t2)
                            {
                                w = (int)(128 * atan ((float)t1 / (float)t2) / M_PI);
                                if (t2 < 0) w += _180deg;
                            }
                            else
                                w = t1 < 0 ? _270deg : _90deg;

                            p->shieldEnergy -= sendMessage (cSprite, m__damageLevel);
                            if (p->shieldEnergy < 1)
                            {
                                p->shieldEnergy = 0;
                                createText (p->x, p->y, "Shield Depleted", 255, 0, 0, TextDecS);
                            }

                            createShield (w & 255, player);
                            return 1;
                        }
                    }
                    else
                    {
                        x = va_arg (args, int);
                        y = va_arg (args, int);
                        w = va_arg (args, int);
                        h = va_arg (args, int);

                        return detectCollision (p->x - hw, p->y - hh,
                                ship->xres, ship->yres, x, y, w, h);
                    }

                    break;

                case m__getX:
                    return p->x;

                case m__getY:
                    return p->y;

                case m__update:
                    goto Default;
            }

            return 0;

   Default: x = p->x;
            y = p->y;

            if (p->shield) p->shield--;

            if (keyMap [M__ALT] && !p->altDown) p->altDown = 1;

            if (!keyMap [M__ALT] && p->altDown)
            {
                p->weapon = (p->weapon + 1) % p->weapons;
                p->altDown = 0;

                p->delay = getWeaponDelay (p->weapon);
            }

            if (x < lxl) x = lxl;
            if (y < lyl) y = lyl;
            if (x > hxl) x = hxl;
            if (y > hyl) y = hyl;

            forceVisible (x, y, LookAheadX, LookAheadY, 0);

            drawFrame (p->x = x, p->y = y, p->shield ? sship : ship, p->t >> 4);

            t1 = x + rcost (hh+ShieldBarH, _90deg);
            t2 = y - rsint (hh+ShieldBarH, _90deg);

            w = (p->shieldEnergy << ShieldBarWL2) >> MaxShieldL2;

            fillrect (t1 - (ShieldBarW >> 1), t2 - (ShieldBarH >> 1), w,
                     ShieldBarH, p->shieldEnabled ? ShieldBarC1 : ShieldBarC0);

            drawrect (t1 - (ShieldBarW >> 1), t2 - (ShieldBarH >> 1), ShieldBarW,
                    ShieldBarH, p->shieldEnabled ? ShieldBarB1 : ShieldBarB0);

            p->ok = 1;

            if (!keyMap [M__E]) p->eDown = 0;

            if (keyMap [M__E] && !p->eDown)
            {
                p->shieldEnabled = (p->shieldEnabled + 1) & 1;
                p->eDown = 1;
            }

            if (!keyMap [M__SPACE]) p->spaceDown = 0;

            if (keyMap [M__SPACE] && !p->spaceDown)
            {
                if (p->bombs)
                {
                    disablePlaySound ();
                    broadcastMessage (t__enemy, m__suicide);
                    enablePlaySound ();

                    playSound (S__EXP);
                    createSpaceQuake (8);

                    p->bombs--;
                }

                p->spaceDown = 1;

                return r__restart;
            }

            ((sprite_t *)score)->handler (score, m__getScore, &vscore);

            t1 = t2 = -1;

            switch (keyMap [M__W]*8 | keyMap [M__A]*4 |
                    keyMap [M__S]*2 | keyMap [M__D])
            {
                case 0x01: /* Right */
                    if (p->nextshot--) break;
                    createShot (x, y, t1 = _0deg, p->weapon);
                    p->nextshot = p->delay;
                    break;

                case 0x02: /* Down */
                    if (p->nextshot--) break;
                    createShot (x, y, t1 = _270deg, p->weapon);
                    p->nextshot = p->delay;
                    break;

                case 0x03: /* Down Right */
                    if (p->nextshot--) break;
                    createShot (x, y, t1 = _315deg, p->weapon);
                    p->nextshot = p->delay;
                    break;

                case 0x04: /* Left */
                    if (p->nextshot--) break;
                    createShot (x, y, t1 = _180deg, p->weapon);
                    p->nextshot = p->delay;
                    break;

                case 0x05: /* Left Right */
                    if (p->nextshot--) break;
                    createShot (x, y, t1 = _180deg, p->weapon);
                    createShot (x, y, t2 = _0deg, p->weapon);
                    p->nextshot = p->delay;
                    break;

                case 0x06: /* Down Left */
                    if (p->nextshot--) break;
                    createShot (x, y, t1 = _225deg, p->weapon);
                    p->nextshot = p->delay;
                    break;

                case 0x08: /* Up */
                    if (p->nextshot--) break;
                    createShot (x, y, t1 = _90deg, p->weapon);
                    p->nextshot = p->delay;
                    break;

                case 0x09: /* Up Right */
                    if (p->nextshot--) break;
                    createShot (x, y, t1 = _45deg, p->weapon);
                    p->nextshot = p->delay;
                    break;

                case 0x0A: /* Up Down */
                    if (p->nextshot--) break;
                    createShot (x, y, t1 = _270deg, p->weapon);
                    createShot (x, y, t2 = _90deg, p->weapon);
                    p->nextshot = p->delay;
                    break;

                case 0x0C: /* Up Left */
                    if (p->nextshot--) break;
                    createShot (x, y, t1 = _135deg, p->weapon);
                    p->nextshot = p->delay;
                    break;
            }

            switch (keyMap [M__UP]*8 | keyMap [M__LEFT]*4 |
                    keyMap [M__DOWN]*2 | keyMap [M__RIGHT])
            {
                case 0x01: /* Right */
                    x += rcost (ShipSpeed, _360deg);
                    y -= rsint (ShipSpeed, _360deg);

                    if (p->t != _360deg) updateAngle4 (&p->t, -AlignSpeed, -AlignSpeed, AlignSpeed, AlignSpeed);
                    break;

                case 0x02: /* Down */
                    x += rcost (ShipSpeed, _270deg);
                    y -= rsint (ShipSpeed, _270deg);

                    if (p->t != _270deg) updateAngle4 (&p->t, -AlignSpeed, AlignSpeed, AlignSpeed, -AlignSpeed);
                    break;

                case 0x03: /* Down-Right */
                    x += rcost (ShipSpeed, _315deg);
                    y -= rsint (ShipSpeed, _315deg);

                    if (p->t != _315deg)
                        updateAngle_2a (&p->t, -AlignSpeed, AlignSpeed);
                    break;

                case 0x04: /* Left */
                    x += rcost (ShipSpeed, _180deg);
                    y -= rsint (ShipSpeed, _180deg);

                    if (p->t != _180deg) updateAngle4 (&p->t, AlignSpeed, AlignSpeed, -AlignSpeed, -AlignSpeed);
                    break;

                case 0x06: /* Down Left */
                    x += rcost (ShipSpeed, _225deg);
                    y -= rsint (ShipSpeed, _225deg);

                    if (p->t != _225deg)
                        updateAngle_2b (&p->t, -AlignSpeed, AlignSpeed);
                    break;

                case 0x08: /* Up */
                    x += rcost (ShipSpeed, _90deg);
                    y -= rsint (ShipSpeed, _90deg);

                    if (p->t != _90deg) updateAngle4 (&p->t, AlignSpeed, -AlignSpeed, -AlignSpeed, AlignSpeed);
                    break;

                case 0x09: /* Up Right */
                    x += rcost (ShipSpeed, _45deg);
                    y -= rsint (ShipSpeed, _45deg);

                    if (p->t != _45deg)
                        updateAngle_2b (&p->t, AlignSpeed, -AlignSpeed);
                    break;

                case 0x0C: /* Up Left */
                    x += rcost (ShipSpeed, _135deg);
                    y -= rsint (ShipSpeed, _135deg);

                    if (p->t != _135deg)
                        updateAngle_2a (&p->t, AlignSpeed, -AlignSpeed);
                    break;
            }

            p->x = x;
            p->y = y;

            return 0;
    }

    void createShip (int x, int y)
    {
        ship_t *s;

            if (!ship)
            {
                if (useHQ)
                {
                    ship = nla__load ("data/ship.nla");
                    if (!ship) animDie ("ship");

                    sship = nla__load ("data/sship.nla");
                    if (!sship) animDie ("sship");
                }
                else
                {
                    ship = nla__load ("data/shiplq.nla");
                    if (!ship) animDie ("shiplq");

                    sship = nla__load ("data/sshiplq.nla");
                    if (!sship) animDie ("sshiplq");
                }

                hw = ship->xres >> 1;
                hh = ship->yres >> 1;

                hyl = sHeight - hh;
                hxl = sWidth - hw;

                lxl = hw;
                lyl = hh;
            }

            if (x == -1) return;

            s = new (ship_t);

            s->x = x;
            s->y = y;

            s->nextshot = s->delay = getWeaponDelay (0);
            s->t = _90deg;

            s->weapons = 1;

            s->shield = ShieldEnergy;
            s->ships = PlayerShips;
            s->bombs = PlayerBombs;

            spriteHandler (s, handler);
            addSprite (s, t__ship);

            player = s;
    }
