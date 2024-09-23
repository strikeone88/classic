/*
    SKEYB.C

    Sensitive Keyword Engine Version 0.01

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <string.h>
    #include <skeyb.h>
    #include <dos.h>

    /* Map of pressed keys. */
    unsigned char keyMap [256];

    /* Previous IRQ1. */
    static void *prev_irq1;

    /* Our keyboard IRQ handler. */
    void interrupt skeyb__handler (void)
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

    /* Starts the engine. */
    void skeyb__start (void)
    {
            memset (keyMap, 0, sizeof (keyMap));

            disable ();

            prev_irq1 = qgetvect (0x09);
            qsetvect (0x09, &skeyb__handler);

            enable ();
    }

    /* Stops the engine. */
    void skeyb__stop (void)
    {
            disable ();
            qsetvect (0x09, prev_irq1);
            enable ();
    }
