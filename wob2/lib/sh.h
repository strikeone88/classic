/*
    SH.H

    Simple Sprite Handler Version 0.01

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __SH_H
#define __SH_H

    #ifndef __GLINK_H
    #include <glink.h>
    #endif

    /* Changes the sprite handler of a sprite. */
    #define spriteHandler(s,h) ((sprite_t *)(s))->handler = (void *)&h

    /* Sends a message to an sprite. */
    #define sendMessage(s,m) ((sprite_t *)(s))->handler (s, m)

    /* Same as above but also sends one argument. */
    #define sendMessage1(s,m,a) ((sprite_t *)(s))->handler (s, m, a)

    /* Accesses the "type" and "sign" fields of a sprite. */
    #define TYPE(x) ((sprite_t *)(x))->type
    #define SIGN(x) ((sprite_t *)(x))->sign

    /* Background sprite type. */
    #define t__back     0

    /* Sprite signature. */
    #define s__sign     0x149A

    /* List of messages. */
    enum messages
    {
        m__update, m__destroy, m__collides
    };

    /* Return codes. */
    enum retcodes
    {
        r__continue, r__restart
    };

    /* Sprite structure. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;

        int         type;
        int         sign;

        int         (*handler) (void *, int, ...);
    }
    sprite_t;

    /* Adds a sprite to the list of sprites. */
    void addSprite (void *p, int type);

    /* Removes a sprite from the list of sprites. */
    void removeSprite (void *p);

    /* Starts the handling cicle. */
    int startCicle (void);

    /* Returns true if the squares collide. */
    int detectCollision (int Rx, int Ry, int Rw, int Rh,
                         int Sx, int Sy, int Sw, int Sh);

    /* Returns the first sprite of the given type that collides. */
    void *spriteCollision (int type, void *q, int x, int y, int w, int h);

    /* Returns the count of sprites of the given types. */
    unsigned spriteCount (int type);

    /* Sends a message to all sprites of the same type. */
    void broadcastMessage (int type, int m);

    /* Sprite being checked in the collision cicle. */
    extern void *cSprite;

    /* Current calculated FPS. */
    extern int gfx__fps;

    /* Last sprite added to the sprites list. */
    extern void *lSprite;

    /* Special POST function that will be called ONCE after a frame. */
    extern void (*post) (void);

#endif
