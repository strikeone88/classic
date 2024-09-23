/*
    W2MIXER.C

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <conio.h>
    #include <game.h>

    #include "sounds/exp.h"
    #include "sounds/shot.h"
    #include "sounds/latch.h"
    #include "sounds/alarm.h"

    /* Length of the playback buffer. */
    #define bufLen      512

    /* Maximum count of sounds to mix. */
    #define maxSounds   32

    /* Sounds being mixed. */
    unsigned char *SoundData [maxSounds];
    unsigned SoundPos [maxSounds];
    unsigned SoundLen [maxSounds];
    unsigned char SoundChannel [maxSounds];
    unsigned SoundVol [maxSounds][2];
    unsigned char SoundSt [maxSounds];

    /* Loaded sounds. */
    unsigned char *LSoundData [] =
    {
        &expD, &shotD, &latchD, &alarmD
    };

    unsigned LSoundLen [] =
    {
        sizeof (expD), sizeof (shotD), sizeof (latchD),
        sizeof (alarmD)
    };

    unsigned mix (unsigned char *a, unsigned char *b, unsigned bytes,
              int leftvol, int rightvol, int stereo)
    {
        int sa, sb, sc, sd;
        unsigned samples;

            if (!stereo) bytes >>= 1;

            samples = bytes;

            while (samples--)
            {
                sb = (unsigned char)(((char)*b * leftvol) >> 8) - 128;
                sa = (unsigned char)(((char)*a * leftvol) >> 8) - 128;

                sc = sa + sb;
                sc = max (sc, -128);
                sc = min (sc, 127);

                sd = max (sa, sb);
                sc = min (sc, sd);

                *a++ = sc + 128;
                if (stereo) b++;

                sb = (unsigned char)(((char)*b * rightvol) >> 8) - 128;
                sa = (unsigned char)(((char)*a * rightvol) >> 8) - 128;

                sc = sa + sb;
                sc = max (sc, -128);
                sc = min (sc, 127);

                sd = max (sa, sb);
                sc = min (sc, sd);

                *a++ = sc + 128;
                b++;
            }

            return bytes;
    }

    void w2mixer (unsigned char *buffer)
    {
        unsigned i, j;

            dmemset (buffer, 0x80808080, bufLen);

            for (i = 0; i < maxSounds; i++)
            {
                if (!SoundData [i]) continue;

                j = SoundLen [i] - SoundPos [i];
                if (j > bufLen) j = bufLen;

                j = mix (buffer, SoundData [i] + SoundPos [i], j,
                    SoundVol [i][0], SoundVol [i][1],
                    SoundSt [i]);

                if (j)
                {
                    if ((SoundPos [i] += j) >= SoundLen [i])
                        SoundData [i] = NULL;
                }
                else
                    SoundData [i] = NULL;
            }
    }

    void start_w2mixer (void)
    {
        int i;

            for (i = 0; i < maxSounds; i++)
                SoundData [i] = NULL;

            setHandler (&w2mixer);
            playback8 (bufLen, 22050, 1);
    }

    void playSound (unsigned s, unsigned leftvol, unsigned rightvol,
                    int stereo)
    {
        int i;

            if (rightvol > 256) rightvol = 256;
            if (leftvol > 256) leftvol = 256;

            for (i = 0; i < maxSounds; i++)
            {
                if (!SoundData [i])
                {
                    SoundSt [i] = stereo;

                    SoundVol [i][1] = rightvol;
                    SoundVol [i][0] = leftvol;

                    SoundLen [i] = LSoundLen [s];
                    SoundPos [i] = 0;
                    SoundData [i] = LSoundData [s];

                    break;
                }
            }
    }
