/*
    AUDIO.C

    Audio Play Back Engine Version 0.01

    NOTE: All the loaded sounds MUST be 8-bit 22050 Hz RAW PCMs.

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdio.h>
    #include <game.h>

    /* Length of the playing buffer. */
    #define BufLen      512

    /* Loaded sound data. */
    static struct
    {
        unsigned    long length;
        unsigned    xmsHandle;
    }
    soundData [MAX_SOUNDS];

    /* Active sound info. */
    static struct
    {
        unsigned    char active;

        unsigned    long length;
        unsigned    long offset;

        unsigned    xmsHandle;
    }
    soundInfo [MAX_MIX];

    /* Indicates if the audio interface is ready to be used. */
    static int audioReady;

    /* Count of loaded sounds and active sounds. */
    static unsigned lCount, aCount;

    /* Playing buffer. */
    static unsigned char buffer [2][BufLen];

    /* Indicates if the PlaySound function should not try to add more snds. */
    static unsigned noMoreSounds;

    /* Audio playback handler. */
    static void handler (void)
    {
        static unsigned char temp [BufLen];
        static unsigned i, j, length;
        static moveEMBst_t P;
        static char *buff;
        int a, b, c;

            memset (buff = buffer [ne__buffer], 0, BufLen);

            P.dest_offs = (unsigned long)&temp;
            P.dest_handle = length = 0;

            for (i = 0; i < aCount; i++)
            {
                if (!soundInfo [i].active) continue;

                P.source_handle = soundInfo [i].xmsHandle;
                P.source_offs = soundInfo [i].offset;

                length = soundInfo [i].length - soundInfo [i].offset;
                if (length > BufLen) length = BufLen;

                if ((soundInfo [i].offset += length) >= soundInfo [i].length)
                    soundInfo [i].active = 0;

                P.length = length;

                moveEMB_st (&P);

                for (j = 0; j < length; j++)
                {
/*                    a = temp [j] - 128;
                    b = buff [j];

                    if (a < 0 && b < 0)
                    {
                        c = a < b ? a : b;
                    }
                    else
                    {
                        if (a >= 0 && b >= 0)
                        {
                            c = a > b ? a : b;
                        }
                        else
                        {
                            c = (a + b) >> 1;
                        }
                    }

                    buff [j] = c;
*/
                    buff [j] += temp [j] - 128;
                }
            }

            for (j = 0; j < BufLen; j++) buff [j] += 128;
    }

    /* Starts the audio engine returns 1 if no SB and 2 if no XMS. */
    int StartAudio (int base, int irq, int dma)
    {
        int i;

            audioReady = 0;

            if (checkXMS ()) return 2;

            if (!ne__start (base, irq, dma, handler)) return 1;

            ne__playback8 (buffer, BufLen, 22050, 0);

            lCount = aCount = noMoreSounds = 0;

            audioReady = 1;

            return 0;
    }

    /* Stops the audio engine and releases XMS blocks. */
    void StopAudio (void)
    {
        int i;

            if (!audioReady) return;

            ne__stop ();

            for (i = 0; i < lCount; i++)
                freeEMB (soundData [i].xmsHandle);
    }

    /* Loads a sound file and returns an index. */
    unsigned LoadSound (char *s)
    {
        static char buf [4096];
        static moveEMBst_t P;
        FILE *fp;

            if (!audioReady) return 0;

            if (lCount >= MAX_SOUNDS) return 0;

            if ((fp = fopen (s, "rb")) == NULL) return 0;

            fseek (fp, 0, SEEK_END);
            soundData [lCount].length = (ftell (fp) & 3L) & -4L;

            soundData [lCount].xmsHandle = allocEMB (soundData [lCount].length);
            if (!soundData [lCount].xmsHandle)
            {
                fclose (fp);
                return 0;
            }

            P.source_handle = 0;
            P.source_offs = (unsigned long)&buf;

            P.dest_handle = soundData [lCount].xmsHandle;
            P.dest_offs = 0;

            rewind (fp);

            memset (buf, 0, sizeof (buf));

            while ((P.length = fread (buf, 1, sizeof (buf), fp)) != 0)
            {
                moveEMB_st (&P);
                P.dest_offs += P.length;

                memset (buf, 0, sizeof (buf));
            }

            fclose (fp);

            return ++lCount;
    }

    /* Disables the PlaySound function. */
    void disablePlaySound (void)
    {
            noMoreSounds = 1;
    }

    /* Enables the PlaySound function. */
    void enablePlaySound (void)
    {
            noMoreSounds = 0;
    }

    /* Starts playing the given sound. */
    void PlaySound (unsigned i)
    {
        int n;

            if (!audioReady || i < 1 || i > lCount || noMoreSounds) return;

            i--;

            for (n = 0; n < aCount; n++)
                if (!soundInfo [n].active) break;

            if (n == aCount) if (aCount == MAX_MIX) return;

            soundInfo [n].xmsHandle = soundData [i].xmsHandle;
            soundInfo [n].length = soundData [i].length;
            soundInfo [n].offset = 0;
            soundInfo [n].active = 1;

            if (n == aCount) aCount++;
    }
