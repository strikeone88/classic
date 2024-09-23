/*
    SH.C

    Simple Sprite Handler Version 0.01

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    /* List of active sprites and background sprites. */
    static list_t sprites, bsprites;

    /* Current calculated FPS. */
    int gfx__fps;

    /* Sprite being checked in the collision cicle. */
    void *cSprite;

    /* Last sprite added to the sprites list. */
    void *lSprite;

    /* Special POST function that will be called ONCE after a frame. */
    void (*post) (void);

    /* Adds a sprite to the list of sprites. */
    void addSprite (void *p, int type)
    {
            if ((TYPE(p) = type) == t__back)
                list__add (&bsprites, p);
            else
                list__add (&sprites, p);

            SIGN(p) = s__sign;

            lSprite = p;
    }

    /* Removes a sprite from the list of sprites. */
    void removeSprite (void *p)
    {
            if (TYPE(p) == t__back)
                list__rem (&bsprites, p);
            else
                list__rem (&sprites, p);

            SIGN(p) = 0;
    }

    /* Returns true if the squares collide. */
    int detectCollision (int Rx, int Ry, int Rw, int Rh,
                         int Sx, int Sy, int Sw, int Sh)
    {
        int dstx, dsty, dx, dy, px, py, qx, qy;

            px = Rx + (Rw >> 1);
            py = Ry + (Rh >> 1);

            qx = Sx + (Sw >> 1);
            qy = Sy + (Sh >> 1);

            dx = (Rw + Sw) >> 1;
            dy = (Rh + Sh) >> 1;

            dstx = abs (px - qx);
            dsty = abs (py - qy);

            return (dstx < dx) && (dsty < dy);
    }

    /* Returns the first sprite of the given type that collides. */
    void *spriteCollision (int type, void *q, int x, int y, int w, int h)
    {
        sprite_t *p;

            cSprite = q;

            for (p = TOP(&sprites); p; p = NEXT(p))
            {
                if (p->type != type) continue;

                if (p->handler (p, m__collides, x, y, w, h))
                    return p;
            }

            return NULL;
    }

    /* Returns the count of sprites of the given types. */
    unsigned spriteCount (int type)
    {
        unsigned count = 0;
        sprite_t *p;

            for (p = TOP(&sprites); p; p = NEXT(p))
                if (p->type == type) count++;

            return count;
    }

    /* Sends a message to all sprites of the same type. */
    void broadcastMessage (int type, int m)
    {
        sprite_t *p, *pn;

            for (p = TOP(&sprites); p; p = pn)
            {
                pn = NEXT(p);
                if (p->type != type) continue;

                p->handler (p, m);
            }
    }

    /* Starts the handling cicle. */
    int startCicle (void)
    {
        clock_t temp, frames;
        sprite_t *p, *np;
        int i, page;

            gfx__fps = frames = page = 0;

            temp = clock ();

            vpage (page++);

            while (COUNT(&sprites))
            {
                apage (page);
                clearpage (page, 0);

                for (p = TOP(&bsprites); p; p = np)
                {
                    np = NEXT(p);

                    i = p->handler (p, m__update);
                    if (i != r__continue) return i;
                }

                for (p = TOP(&sprites); p; p = np)
                {
                    np = NEXT(p);

                    i = p->handler (p, m__update);
                    if (i == r__restart)
                    {
                        if (np && SIGN(np) != s__sign)
                            break;
                        else
                            continue;
                    }

                    if (i != r__continue) return i;
                }

                frames++;

                if ((clock () - temp) > 3)
                {
                    temp = clock () - temp;
                    gfx__fps = temp ? frames*(long)CLOCKS_PER_SEC / temp : 0;

                    temp = clock ();
                    frames = 0;
                }

                vpage (page++);

                page &= 1;

                if (post) post ();
                post = NULL;
            }

            return 0;
    }
