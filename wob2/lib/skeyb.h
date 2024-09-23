/*
    SKEYB.H

    Sensitive Keyword Engine Version 0.01

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __SKEYB_H
#define __SKEYB_H

    /* Changes interrupt vector very quicky. */
    #define qsetvect(n,v)   (*((void *(*)[256])NULL)) [(n)] = v
    #define qgetvect(n)     (*((void *(*)[256])NULL)) [(n)]

    /* List of single make codes (w/o E0 or E1) for each key. */
    #define M__BACKSPACE    0x0E
    #define M__ENTER        0x1C
    #define M__ESC          0x01
    #define M__ALT          0x38
    #define M__CTRL         0x1D
    #define M__LSHIFT       0x2A
    #define M__RSHIFT       0x36
    #define M__SPACE        0x39
    #define M__TAB          0x0F
    #define M__MINUS        0x0C
    #define M__EQUAL        0x0D
    #define M__LBRACKET     0x1A
    #define M__RBRACKET     0x1B
    #define M__SEMICOLON    0x27
    #define M__QUOTE        0x28
    #define M__IQUOTE       0x29
    #define M__BSLASH       0x2B
    #define M__COMMA        0x33
    #define M__POINT        0x34
    #define M__SLASH        0x35
    #define M__UP           0x48
    #define M__LEFT         0x4B
    #define M__DOWN         0x50
    #define M__RIGHT        0x4D

    /* Single make codes for grouped keys. */
    enum groupedKeys
    {
        M__Q = 0x10, M__W, M__E, M__R, M__T, M__Y, M__U, M__I, M__O, M__P,
        M__A = 0x1E, M__S, M__D, M__F, M__G, M__H, M__J, M__K, M__L,
        M__Z = 0x2C, M__X, M__C, M__V, M__B, M__N, M__M,
        M__1 = 0x02, M__2, M__3, M__4, M__5, M__6, M__7, M__8, M__9, M__0
    };

    /* Starts the engine. */
    void skeyb__start (void);

    /* Stops the engine. */
    void skeyb__stop (void);

    /* Map of pressed keys. */
    extern unsigned char keyMap [256];

#endif
