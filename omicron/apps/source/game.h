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

    /* Maximum FPS limit. */
    #define MaxFPS          75

    /* Screen resolution. */
    #define s__width        640
    #define s__height       480

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
    #define aY1             0
    #define aX1             0

    /* Some constants. */
    #define MsX1            (sWidth - aWidth)
    #define MsY1            (sHeight - aHeight)

    /* SShot */
    #define ShotSpeed       16
    #define ShotDist        32
    #define ShotTDlt1       convdeg (5)
    #define ShotTDlt2       convdeg (10)
    #define ShotTDelay      24

    /* Ship */
    #define AlignSpeed      16
    #define ShipSpeed1      8
    #define ShipSpeed2      10
    #define ShieldEnergy    128
    #define LookAheadX      256
    #define LookAheadY      256
    #define PlayerShips     3
    #define ShieldBarC1     200, 0, 0
    #define ShieldBarB1     128, 0, 0
    #define ShieldBarC0     70, 0, 0
    #define ShieldBarB0     52, 0, 0
    #define WeaponBarC1     0, 200, 0
    #define WeaponBarB1     0, 128, 0
    #define WeaponBarC0     0, 70, 0
    #define WeaponBarB0     0, 52, 0

    #define MaxWeaponE      8190
    #define MaxCont         10
    #define MinWeaponE      ContEnergy
    #define ContEnergy      (MaxWeaponE / MaxCont)

    #define MaxGen          10
    #define GenValue        32

    #define BarW            176
    #define BarH            10

    #define ShipM           15

    /* Boom */
    #define BoomDec         16
    #define BoomDecF        32
    #define BoomLen         6//6
    #define BoomStep        4
    #define BoomRad         15 //31
    #define BoomDist        100 //300
    #define BoomSpeed       4

    /* HBoom */
    #define HBoomStepL2     3
    #define HBoomStep       (1 << HBoomStepL2)
    #define HBoomFadeCL2    2
    #define HBoomFadeC      (1 << HBoomFadeCL2)

    #define HBoomHDelay     32

    #define HBoomCyan       0
    #define HBoomMagenta    1
    #define HBoomYellow     2
    #define HBoomOrange     3
    #define HBoomWhite      4
    #define HBoomBlue       5
    #define HBoomGreen      6

    /* Cyan */
    #define CyanShield      128
    #define CyanUpdate      16
    #define CyanSpeed       2
    #define CyanPrice       1000//5,50
    #define CyanR           1

    /* Magenta */
    #define MagentaShield   128
    #define MagentaUpdate   0
    #define MagentaSpeed    3
    #define MagentaFDelay   0
    #define MagentaPrice    500//8,100
    #define MagentaR        16

    /* MMagenta */
    #define MMagentaUpdate  1
    #define MMagentaSpeed   1
    #define MMagentaFDelay  4
    #define MMagentaPrice   100//2,20
    #define MMagentaBR      16
    #define MMagentaTI      8
    #define MMagentaR       2

    /* Scanner */
    #define ScannerDelay    64
    #define InitMinEnemies  16
    #define InitMaxEnemies  32
    #define EnemyRaise      (1*60*CLOCKS_PER_SEC)
    #define RaiseIndex      16

    #define maxMagenta      64
    #define maxCyan         64

    /* Text */
    #define TextDecF        24
    #define TextDecS        16
    #define TextDecSS       8
    #define TextDecSSS      2

    /* SQuake */
    #define SQuakeDelay     4

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
    #define MaxShips        10
    #define MaxMult         50
    #define MaxWeapons      4

    #define MaxMinEnemies   24
    #define MaxMaxEnemies   64

    #define ShieldRaise     50
    #define WeaponRaise     100
    #define MultRaise       150
    #define BombRaise       350
    #define ShipRaise       450

    #define ShieldIndex     128

    #include <stdarg.h>
    #include <stdlib.h>
    #include <time.h>

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

    /* Extra messages. */
    enum xmessages
    {
        m__getX = 0x10, m__getY, m__addScore, m__getMultiplier, m__resetCounters,
        m__addKill, m__suicide, m__getScore, m__resetScore, m__getShips,
        m__addShip, m__getW, m__getH, m__getWeapon, m__setWeapon,
        m__getWeapons, m__setWeapons, m__getBombs, m__addBomb, m__hit,
        m__shieldOff, m__acceptShield, m__restartShield, m__getShieldEnergy,
        m__addShieldEnergy, m__getShieldState, m__activeShield, m__damageLevel,
        m__getK
    };

    /* Extra return codes. */
    enum xretcodes
    {
        r__abort = 0x10, r__noplayer
    };

    /* Sprites types. */
    enum stypes
    {
        t__ship = 1, t__sshot, t__enemy, t__scanner, t__score, t__squake,
        t__item, t__dollar
    };

    /* Draws a line from (x1, y1) to (x2, y2) with color c. */
    void line (int x1, int y1, int x2, int y2, int c, int sX1, int sY1);

    /* Draws a blended line from (x1, y1) to (x2, y2) with color c. */
    void lineBlend (int x1, int y1, int x2, int y2, int r1, int g1, int b1,
                    int r2, int g2, int b2, int sX1, int sY1);

    /* Draws a frame in the virtual screen (works as nla__drawframec). */
    void drawFrame (int x, int y, anim_t *q, unsigned frame);

    /* Draws a frame statically. */
    void drawFrameS (int x, int y, anim_t *q, unsigned frame);

    /* Draws a frame in the virtual screen (works as nla__drawframe). */
    void drawFrameB (int x, int y, anim_t *q, unsigned char *buf);

    /* Same as above but you supply the sX1 and sY1. */
    void drawFrameWsx (int x, int y, anim_t *q, unsigned frame, int sX1, int sY1);

    /* Forces visibility of an area with center (x, y) and length p. */
    void forceVisible (int x, int y, int p, int q, int loop);

    /* Plots a pixel. */
    void qputpixel (int x, int y, int c, int sX1, int sY1);

    /* Prints a string at (x, y) with color c. */
    void bprintf (int x, int y, char *s, int r, int g, int b, int sX1, int sY1);

    /* Prints a string at (x, y) with color c. */
    void bprintfs (int x, int y, char *s, int r, int g, int b, int sX1, int sY1);

    /* Fills a rectangle with color c. */
    void fillrect (int x, int y, int w, int h, int r, int g, int b);

    /* Draws a rectangle (outline). */
    void drawrect (int x, int y, int w, int h, int r, int g, int b);

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

    /* KeyMap. */
    extern char *keyMap;

    extern ServiceInterface gfx, gdev, nla, bf2, skeyb;
    extern void *Ct;

    /* Shows a message and terminates execution. */
    void animDie (char *s);

/*같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같*/

    void createScanner (void);
    void createStar (int x, int y, int c);
    void createScore (void);
    void createText (int x, int y, char *st, int r, int g, int b, int dec, int sts);
    void createShip (int x, int y);
    void createHBoom (int x, int y, int c, int delay, int r, int g, int b, int Dec);
    void createShield (int t, void *obj);
    void createSpaceQuake (int scale);
    void createCyan (int x, int y);
    void createMagenta (int x, int y);
    void createMMagenta (int x, int y);
    void createTrail (int x, int y, int frame, anim_t *anim, int i, int s);
    void createItem (int x, int y, int m, int c, int flags, int code, anim_t *anim, int k);

    int getWeaponCost (int sl);

    void createExplosion (int x, int y, int rad, int maxrad, int inc);

/*같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같*/

    extern anim_t *dollar, *bomb, *econt, *gen, *fire;

    #define RADIAL      0x8000
    #define RANDOM      0x4000

    extern int gfx__fps;

/*같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같*/

    int initAudio (void);
    void deinitAudio (void);

    extern int sbuffer, sdone;

    void writedsp (int val);
    int readdsp (void);

    void speaker (int state);
    void resetdsp (void);

    int readmixer (unsigned n);
    void writemixer (unsigned n, unsigned v);

    void playback8 (unsigned len, unsigned freq, unsigned stereo);

    void setHandler (void *);

    int audioReady ();

    void start_w2mixer (void);

/*같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같*/

    enum Sounds
    {
        S__EXP, S__SHOT, S__LATCH, S__ALARM,
        S__SAY_SHIP, S__SAY_BOMB, S__SAY_FIRE
    };

    void playSound (unsigned s, unsigned leftvol, unsigned rightvol,
                    unsigned delay);

#endif
