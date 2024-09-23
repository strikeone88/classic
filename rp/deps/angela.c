/*
    ANGELA.C

    Angela Encryption Engine Version 0.01

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>

    /* Encryps the given block using Angela. */
    unsigned ax__encrypt (const char *buf, unsigned len, char *dest,
                          char *keyword, unsigned keylen)
    {
        signed short i, j, k, rlen, keyindex;
        unsigned long sbuf, tmp;
        unsigned char sumL, sumH;

            len = (rlen = ((len + 3) & -4)) >> 2;
            keyindex = 0;

            for (i = j = 0; i < keylen; i++) j += 3*keyword [i];

            sumH = (unsigned char)(((unsigned)j) >> 8);
            sumL = (unsigned char)(unsigned)j;

            while (len--)
            {
                sbuf = 0;

                for (i = 0; i < 4; i++)
                {
                    k = (unsigned char)(*buf++);

                    tmp = 0;

                    for (j = 0; j < 8; j++, sbuf >>= 4, k >>= 1)
                        tmp |= (((sbuf & 0x0F) << 1) | (k & 0x01)) << (j << 2);

                    sbuf = tmp;
                }

                for (i = 0; i < 4; i++, sbuf <<= 8)
                {
                    k = (sbuf & 0xFF000000) >> 24;

                    if (keylen)
                    {
                        if (keyindex == keylen) keyindex = 0;

                        if (i & 1)
                        {
                            k = (k ^ sumL) + keyword [keyindex];
                            keyword [keyindex++] += sumH - k;
                        }
                        else
                        {
                            k = (k - sumH) ^ keyword [keyindex];
                            keyword [keyindex++] += sumL + k;
                        }

                        sumH += (k & 0xF0);
                        sumL -= (k & 0x0F);
                    }

                    *dest++ = k;
                }
            }

            return rlen;
    }

    /* Decrypts the given encrypted block. */
    void ax__decrypt (const char *buf, unsigned len, char *dest,
                      char *keyword, unsigned keylen)
    {
        int i, j, k, t, lt, rt, keyindex;
        unsigned char sumH, sumL, u;

            for (i = j = 0; i < keylen; i++) j += 3*keyword [i];

            sumH = (unsigned char)(((unsigned)j) >> 8);
            sumL = (unsigned char)(unsigned)j;

            keyindex = 0;
            len &= ~3;

            for (i = 0; i < len; i += 4, dest += 4)
            {
                for (j = 0; j < 4; j++) dest [j] = 0;

                for (j = t = 0; j < 4; j++, t += 2)
                {
                    lt = rt = (unsigned char)*buf++;

                    if (keylen)
                    {
                        if (keyindex == keylen) keyindex = 0;

                        u = keyword [keyindex];

                        if (j & 1)
                        {
                            keyword [keyindex++] += sumH - lt;
                            lt = (lt - u) ^ sumL;
                        }
                        else
                        {
                            keyword [keyindex++] += sumL + lt;
                            lt = (lt ^ u) + sumH;
                        }

                        sumH += (rt & 0xF0);
                        sumL -= (rt & 0x0F);
                    }

                    rt = lt;

                    for (k = 0; k < 4; k++, lt <<= 1, rt >>= 1)
                    {
                        dest [3 - k] |= (rt & 0x01) << (6 - t);
                        dest [k] |= (lt & 0x80) >> t;
                    }
                }
            }
    }
