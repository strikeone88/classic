/*
    ANGELA.H

    Angela Encryption Engine Version 0.01

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __ANGELA_H
#define __ANGELA_H

    /* Encryps the given block using Angela. */
    unsigned ax__encrypt (const char *buf, unsigned len, char *dest,
                          char *keyword, unsigned keylen);

    /* Decrypts the given encrypted block. */
    void ax__decrypt (const char *buf, unsigned len, char *dest,
                      char *keyword, unsigned keylen);

#endif
