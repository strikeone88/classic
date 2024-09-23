
    #include <stdlib.h>
    #include <conio.h>

    /* C-Media Vendor Id */
    #define CMEDIA_VENDOR   0x13F6

    /* Acer Vendor Id */
    #define ACER_VENDOR     0x10B9

    /* Acer Device Id Range */
    #define ACER_DEVICE_L   0x100
    #define ACER_DEVICE_H   0x200

    /* Invalid Value. */
    #define INV_VALUE       (0xFFFFFFFFUL)

    /* Detected IRQ and Base-Port. */
    unsigned IRQ = 0, BASE, Testing;

    /* Scans for a pci sound card that matches the vendor id and device id. */
    static unsigned long scan_card (void)
    {
        unsigned vendor_id, device_id, bus, dev, func;
        unsigned long p;

            p = pci_pack (0, 0, 0);

            for (bus = 0; bus < 256; bus++)
            {
                pci_set_bus (&p, bus);

                for (dev = 0; dev < 32; dev++)
                {
                    pci_set_dev (&p, dev);

                    for (func = 0; func < 8; func++)
                    {
                        pci_set_func (&p, func);
                        pci_set_reg (&p, 0);

                        pci_config_addr (p);
                        vendor_id = pci_getw (0);

                        if (vendor_id == 0 || vendor_id == 0xFFFFU)
                            continue;

                        if (vendor_id != CMEDIA_VENDOR && vendor_id !=
                            ACER_VENDOR) continue;

                        if (vendor_id == ACER_VENDOR)
                        {
                            device_id = pci_getw (2);

                            if (device_id < ACER_DEVICE_L || device_id >
                                ACER_DEVICE_H) continue;
                        }

                        pci_set_reg (&p, 0x0C);
                        pci_config_addr (p);

                        if ((pci_getb (2) & 0x7F) != 0) continue;

                        return p;
                    }
                }
            }

            return INV_VALUE;
    }

    /* Returns the irq-index of the given irq line. */
    static int irq_index (int irq)
    {
            switch (irq)
            {
                case 0x03:  return 0x80;
                case 0x05:  return 0x02;
                case 0x07:  return 0x04;
                case 0x09:  return 0x01;
                case 0x0A:  return 0x08;
                default:    return 0x02;
            }
    }

    void irq3 (void) { printf ("[3]"); IRQ = 0x03; outportb (0x20, 0x20); outportb (0xA0, 0x20); inportb (0x22E); }
    void irq5 (void) { printf ("[5]"); IRQ = 0x05; outportb (0x20, 0x20); outportb (0xA0, 0x20); inportb (0x22E); }
    void irq7 (void) { printf ("[7]"); IRQ = 0x07; outportb (0x20, 0x20); outportb (0xA0, 0x20); inportb (0x22E); }
    void irq9 (void) { printf ("[9]"); IRQ = 0x09; outportb (0x20, 0x20); outportb (0xA0, 0x20); inportb (0x22E); }
    void irqA (void) { printf ("[A]"); IRQ = 0x0A; outportb (0x20, 0x20); outportb (0xA0, 0x20); inportb (0x22E); }
    void irqB (void) { printf ("[B]"); IRQ = 0x0B; outportb (0x20, 0x20); outportb (0xA0, 0x20); inportb (0x22E); }

    void *Prev [6];

    void enableIRQ (int i)
    {
            if (i < 8)
                outportb (0x21, inportb (0x21) & ~(1 << i));
            else
                outportb (0xA1, inportb (0xA1) & ~(1 << (i - 8)));
    }

    void disableIRQ (int i)
    {
            if (i < 8)
                outportb (0x21, inportb (0x21) | (1 << i));
            else
                outportb (0xA1, inportb (0xA1) | (1 << (i - 8)));
    }

    void handler (void);
    void speaker (int);
    void resetdsp (void);
    int readdsp (void);

    void playback8 (unsigned len, unsigned freq, unsigned stereo);

    int main (void)
    {
        unsigned i, j, irq;
        unsigned long p;

            if ((p = scan_card ()) == INV_VALUE)
            {
                printf ("ERROR: C-Media compatible sound card not found.\n");
                getch ();
                return 1;
            }

            pci_set_reg (&p, 0x10);
            pci_config_addr (p);
            BASE = pci_getw (0) & ~3UL;

            printf ("PCI C-Media Compatible Card found at Base Address %04x\n", BASE);

            pci_set_reg (&p, 0x3C);
            pci_config_addr (p);
            irq = pci_getb (0);

            pci_set_reg (&p, 0x04);
            pci_config_addr (p);
            i = pci_getb (0) | 3;
            pci_setb (0, i);
            i = pci_getb (0);

            if ((i & 0x5) != 0x5)
            {
                printf ("ERROR: Device bus master error.\n");
                getch ();
                return 2;
            }

            printf ("I/O Accesses enabled.\n");

            i = irq_index (irq);

            outportb (BASE + 0x04, 0x08);
            outportb (BASE + 0x05, 0x00);

            outportb (BASE + 0x1A, 0x08);
            outportb (BASE + 0x18, 0x03);

            outportb (BASE + 0x16, inportb (BASE + 0x16) | 0x10);

            outportb (BASE + 0x21, 0x05);

            outportb (BASE + 0x27, 0x01);

            outportb (0x224, 0x80);
            outportb (0x225, i);

            outportb (0x224, 0x81);
            outportb (0x225, 0x22);

            printf ("Card Configured.\n");

            resetdsp ();

            i = readdsp ();
            if (i != 0xAA)
            {
                printf ("ERROR: Sound blaster error.\n");
                getch ();
                return 2;
            }

            printf ("Sound Blaster Emulation Detected.\n");

            disable ();

            Prev [0] = getvect (0x20 + 0x03);
            Prev [1] = getvect (0x20 + 0x05);
            Prev [2] = getvect (0x20 + 0x07);
            Prev [3] = getvect (0x20 + 0x09);
            Prev [4] = getvect (0x20 + 0x0A);
            Prev [5] = getvect (0x20 + 0x0B);

            setvect (0x20 + 0x03, &irq3, 0);
            setvect (0x20 + 0x05, &irq5, 0);
            setvect (0x20 + 0x07, &irq7, 0);
            setvect (0x20 + 0x09, &irq9, 0);
            setvect (0x20 + 0x0A, &irqA, 0);
            setvect (0x20 + 0x0B, &irqB, 0);

            i = inportb (0x21);
            j = inportb (0xA1);

            outportb (0x21, 0x00);
            outportb (0xA1, 0x00);

            enable ();

            printf ("Testing IRQ...");

            Testing = 1;
            playback8 (512, 22050, 0);
            Testing = 0;

            delay (5000); //10s

            printf ("Done\n");

            disable ();

            outportb (0x21, i);
            outportb (0xA1, j);

            setvect (0x20 + 0x03, Prev [0], 0);
            setvect (0x20 + 0x05, Prev [1], 0);
            setvect (0x20 + 0x07, Prev [2], 0);
            setvect (0x20 + 0x09, Prev [3], 0);
            setvect (0x20 + 0x0A, Prev [4], 0);
            setvect (0x20 + 0x0B, Prev [5], 0);

            enable ();

            printf ("IRQ Acquired.\n");

            if (IRQ)
                printf ("[IRQ %u]\n", IRQ);
            else
                printf ("IRQ ERROR!!!!!\n", IRQ);

            getch ();

            return 0;
    }

    void deinitAudio (void)
    {
            if (!IRQ) return;

            disable ();

            disableIRQ (IRQ);

            speaker (0);
            resetdsp ();

            setvect (0x20 + IRQ, Prev [0], 0);
            IRQ = 0;

            enable ();
    }

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

    /* This is the interrupt handler given by the user. */
    void (*usrHandler) (unsigned char *);

    int sbuffer, sdone;

    unsigned char *sbuf [2];

    int audioReady (void)
    {
            return IRQ;
    }

    void setHandler (void *p)
    {
            usrHandler = p;
    }

    void handler (void)
    {
            inportb (0x22E);

            outportb (0x20, 0x20);
            outportb (0xA0, 0x20);

            if (usrHandler) usrHandler (sbuf [sbuffer]);
            sbuffer = ++sbuffer & 1;
    }

    void writedsp (int val)
    {
            while (inportb (0x22C) & 0x80);
            outportb (0x22C, val);
    }

    int readdsp (void)
    {
            while (!(inportb (0x22E) & 0x80));
            return inportb (0x22A);
    }

    void speaker (int state)
    {
            writedsp (state ? 0xD1 : 0xD3);
    }

    void resetdsp (void)
    {
            outportb (0x226, 1);
            delay (4); //8ms
            outportb (0x226, 0);
    }

    void setupdma (unsigned long ptr, unsigned len)
    {
            outportb (0x0A, 0x04 + 0x01);
            outportb (0x0C, 0x00);
            outportb (0x0B, 0x58 + 0x01);

            outportb (0x02, ptr);
            outportb (0x02, ptr >> 8);

            outportb (0x03, --len);
            outportb (0x03, len >> 8);

            outportb (0x83, ptr >> 16);

            outportb (0x0A, 0x01);
    }

    int readmixer (unsigned n)
    {
            outportb (0x224, n);
            delay (2); //4ms

            return inportb (0x225);
    }

    void writemixer (unsigned n, unsigned v)
    {
            outportb (0x224, n);
            delay (2); //4ms

            outportb (0x225, v);
            delay (2);
    }

    void playback8 (unsigned len, unsigned freq, unsigned stereo)
    {
        unsigned char *buf = (void *)0x80000;

            if ((!IRQ || !usrHandler) && !Testing) return;

            sbuf [1] = buf + len;
            sbuf [0] = buf;

            if (!Testing)
            {
                usrHandler (sbuf [0]);
                usrHandler (sbuf [1]);
            }

            setupdma (0x80000, len << 1);

            if (stereo)
            {
                writemixer (0x0E, readmixer (0x0E) | 0x22);
                freq <<= 1;
            }

            writedsp (0x40);
            writedsp (256 - (1000000L / freq));

            sbuffer = sdone = 0;

            writedsp (0x48);
            writedsp (--len);
            writedsp (len >> 8);

            writedsp (0x1C);
    }
