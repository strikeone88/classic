/*
    GAME.H

    Game Header File

    Simply includes all the things that are needed to build the game,
    things such as graphics, keyboard handling and lists.

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __GAME_H
#define __GAME_H

    /* Virtual screen resolution. */
    #define sHeightMask     (sHeight - 1)
    #define sWidthMask      (sWidth - 1)
    #define sWidth          1024
    #define sHeight         1024

    /* Active screen area. */
    #define aY2             (s__height - 1)
    #define aX2             (s__width - 1)
    #define aHeight         (aY2 - aY1 + 1)
    #define aWidth          (aX2 - aX1 + 1)
    #define aY1             30
    #define aX1             0

    /* Some constants. */
    #define MsX1            (sWidth - aWidth)
    #define MsY1            (sHeight - aHeight)

    /* Color of the status bars. */
    #define BarColor        packrgb (0, 16, 32)

    /* SShot */
    #define ShotSpeed       16
    #define ShotDist        32
    #define ShotTDlt1       convdeg (10)
    #define ShotTDlt2       convdeg (20)
    #define ShotTDelay      8

    /* Ship */
    #define AlignSpeed      16
    #define ShipSpeed       10
    #define ShieldEnergy    16
    #define LookAheadX      256
    #define LookAheadY      256
    #define PlayerShips     3
    #define PlayerBombs     5
    #define ShieldBarWL2    5
    #define ShieldBarW      (1 << ShieldBarWL2)
    #define ShieldBarH      6
    #define ShieldBarC1     packrgb (0, 200, 0)
    #define ShieldBarB1     packrgb (0, 128, 0)
    #define ShieldBarC0     packrgb (0, 70, 0)
    #define ShieldBarB0     packrgb (0, 52, 0)

    /* Boom */
    #define BoomDec         16
    #define BoomLen         6
    #define BoomStep        4
    #define BoomRad         31
    #define BoomDist        400
    #define BoomSpeed       4 /* DON'T CHANGE !!!! */

    /* HBoom */
    #define HBoomStepL2     3
    #define HBoomStep       (1 << HBoomStepL2)
    #define HBoomFadeCL2    2
    #define HBoomFadeC      (1 << HBoomFadeCL2)

    #define HBoomHDelay     4

    #define HBoomCyan       0
    #define HBoomMagenta    1
    #define HBoomYellow     2
    #define HBoomOrange     3
    #define HBoomWhite      4
    #define HBoomBlue       5
    #define HBoomGreen      6

    /* Cyan */
    #define CyanShield      32
    #define CyanUpdate      8
    #define CyanSpeed       4
    #define CyanPrice       25
    #define CyanR           1

    /* Magenta */
    #define MagentaShield   32
    #define MagentaUpdate   0
    #define MagentaSpeed    7
    #define MagentaFDelay   0
    #define MagentaPrice    50
    #define MagentaR        16

    /* MMagenta */
    #define MMagentaUpdate  1
    #define MMagentaSpeed   1
    #define MMagentaFDelay  4
    #define MMagentaPrice   10
    #define MMagentaBR      16
    #define MMagentaTI      8
    #define MMagentaR       2

    /* Scanner */
    #define ScannerDelay    4//avril
    #define InitMinEnemies  16//32
    #define InitMaxEnemies  32//48
    #define EnemyRaise      (3*60*CLOCKS_PER_SEC)
    #define RaiseIndex      16

    #define maxMagenta      8
    #define maxCyan         64

    /* Text */
    #define TextDecF        32
    #define TextDecS        8
    #define TextDecSS       4

    /* SQuake */
    #define SQuakeDelay     16
    #define SQuakeDec       1

    /* Shield */
    #define ShieldInc       5 /* Log2 */
    #define ShieldFadeC     3
    #define ShieldDelay     1

    /* General */
    #define ScrollLengthX   8
    #define ScrollLengthY   8
    #define UNUSED          31

    #define MaxShieldL2     10
    #define MaxShield       (1 << MaxShieldL2)

    #define MaxBombs        10
    #define MaxShips        5
    #define MaxMult         50
    #define MaxWeapons      4

    #define MaxMinEnemies   128
    #define MaxMaxEnemies   200

    #define WeaponRaise     100
    #define MultRaise       150
    #define ShieldRaise     250
    #define BombRaise       350
    #define ShipRaise       450

    #define ShieldIndex     128

    #include <stdarg.h>
    #include <stdlib.h>
    #include <time.h>
    #include <dos.h>

    #ifndef __GFX_H
    #include <gfx.h>
    #endif

    #ifndef __GLINK_H
    #include <glink.h>
    #endif

    #ifndef __SKEYB_H
    #include <skeyb.h>
    #endif

    #ifndef __TRIG_H
    #include <trig.h>
    #endif

    #ifndef __NLA_H
    #include <nla.h>
    #endif

    #ifndef __SH_H
    #include <sh.h>
    #endif

    #ifndef __BF2_H
    #include <bf2.h>
    #endif

    #ifndef __AUDIO_H
    #include <audio.h>
    #endif

    #ifndef __NE_H
    #include <ne.h>
    #endif

    #ifndef __XMSH_H
    #include <xmsh.h>
    #endif

    /* Extra messages. */
    enum xmessages
    {
        m__getX = 0x10, m__getY, m__addScore, m__getMultiplier, m__resetCounters,
        m__addKill, m__suicide, m__getScore, m__resetScore, m__getShips,
        m__addShip, m__getW, m__getH, m__getWeapon, m__setWeapon,
        m__getWeapons, m__setWeapons, m__getBombs, m__addBomb, m__hit,
        m__shieldOff, m__acceptShield, m__restartShield, m__getShieldEnergy,
        m__addShieldEnergy, m__getShieldState, m__activeShield, m__damageLevel
    };

    /* Extra return codes. */
    enum xretcodes
    {
        r__abort = 0x10, r__noplayer
    };

    /* Sprites types. */
    enum stypes
    {
        t__ship = 1, t__sshot, t__enemy, t__scanner, t__score, t__squake
    };

    /* Easier way to identify each loaded sound. */
    enum loadedSounds
    {
        S__EXP, S__SHOT, S__HIT, S__HEAL
    };

    /* Draws a line from (x1, y1) to (x2, y2) with color c. */
    void line (int x1, int y1, int x2, int y2, int c, int sX1, int sY1);

    /* Draws a blended line from (x1, y1) to (x2, y2) with color c. */
    void lineBlend (int x1, int y1, int x2, int y2, int c, int c2, int sX1, int sY1);

    /* Draws a frame in the virtual screen (works as nla__drawframec). */
    void drawFrame (int x, int y, anim_t *q, unsigned frame);

    /* Same as above but you supply the sX1 and sY1. */
    void drawFrameWsx (int x, int y, anim_t *q, unsigned frame, int sX1, int sY1);

    /* Forces visibility of an area with center (x, y) and length p. */
    void forceVisible (int x, int y, int p, int q, int loop);

    /* Plots a pixel. */
    void qputpixel (int x, int y, int c, int sX1, int sY1);

    /* Prints a string at (x, y) with color c. */
    void bprintf (int x, int y, char *s, int c, int sX1, int sY1);

    /* Plays a sound given a symbolic index. */
    void playSound (unsigned n);

    /* Fills a rectangle with color c. */
    void fillrect (int x, int y, int w, int h, int c);

    /* Draws a rectangle (outline). */
    void drawrect (int x, int y, int w, int h, int c);

    /* Indicates we're using HQ graphics. */
    extern int useHQ;

    /* Indicates HBoom is enabled. */
    extern int useHBoom;

    /* Current virtual screen coords (and their original dups). */
    extern int sX1, sY1, dsX1, dsY1;

    /* Virtual screen end point. */
    extern int sX2, sY2;

    /* Global text buffers. */
    extern char g__tempBuf [512], gbuf [512];

    /* Global sprites. */
    extern void *player, *score;

    /* Shield half resolution. */
    extern int shield__hw, shield__hh;

#endif
