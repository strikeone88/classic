/*
    SKEYB.C

    Sensitive Keyword Engine Version 0.01

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <skeyb.h>

    /* Map of pressed keys. */
    unsigned char keyMap [256];

    /* Keyboard handler. */
    void handler (void)
    {
        int i;

            if (!(inportb (0x64) & 1)) goto Done;

            i = inportb (0x60);

            if (i & 0x80)
                keyMap [i & 0x7F] = 0;
            else
                keyMap [i & 0x7F] = 1;

      Done: outportb (0x20, 0x20);
    }

    void *skeyb (unsigned Command, ...)
    {
        static void *prev = NULL;
        static va_list args;

            va_start (args, Command);

            switch (Command)
            {
                case KStart:
                    if (prev) returnv (1);

                    dmemset (keyMap, 0, sizeof (keyMap));

                    prev = getvect (0x21);
                    setvect (0x21, &handler, 0);

                    printf ("SKeyb Started!\n");

                    break;

                case KStop:
                    if (!prev) returnv (1);

                    setvect (0x21, prev, 0);
                    prev = NULL;

                    break;

                case KGetKeyMap:
                    return keyMap;
            }

            return NULL;
    }

    int main (void)
    {
            registerInterface ("skeyb-0.01", &skeyb);
            setResident ();

            printf ("skeyb-0.01\n");
            return 0;
    }
