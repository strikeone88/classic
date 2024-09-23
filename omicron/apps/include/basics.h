/*
    BASICS.H
*/

#ifndef __BASICS_H
#define __BASICS_H

    #define setvect(x,y,z) (void)KernelInterface (F__SETVECT, (int)(x), (void *)(y), (int)(z))
    #define getvect(x) (void *)KernelInterface (F__GETVECT, (int)(x))

    #define gtime() (time_t)KernelInterface (F__GTIME)
    #define clock() (time_t)KernelInterface (F__CLOCK)
    #define packtime(x) (time_t *)KernelInterface (F__PACKTIME, (tm *)(x))
    #define unpacktime(x,y) (tm *)KernelInterface (F__UNPACKTIME, (time_t *)(x), (tm *)(y))

    #define v86int(x,y) (void)KernelInterface (F__V86INT, (int)(x), (REGPACK *)(y))

    #define CLOCKS_PER_SEC 500
    #define NULL ((void *)(0))

    /* Registry pack for V86 interrupt call. */
    typedef struct
    {
        unsigned    ax, bx, cx, dx;
        unsigned    bp, si, di, ds, es;
    }
    REGPACK;

    /* Class Code 0003 */
    enum CC_0003
    {
        F__OUTPORTB = 0x00030000,   /* (int, int) : void */
        F__OUTPORTW,                /* (int, int) : void */
        F__OUTPORTD,                /* (int, int) : void */
        F__INPORTB,                 /* (int) : int */
        F__INPORTW,                 /* (int) : int */
        F__INPORTD,                 /* (int) : int */
        F__SETVECT,                 /* (int, void *) : void */
        F__GETVECT,                 /* (int) : void * */
        F__GTIME,                   /* (void) : time_t * */
        F__PACKTIME,                /* (tm *) : time_t * */
        F__UNPACKTIME,              /* (time_t, tm *) : tm * */
        F__V86INT,                  /* (int, REGPACK *) : void */
        F__CLOCK,                   /* (void) : time_t */
    };

    typedef unsigned int size_t;
    typedef unsigned int uint;
    typedef unsigned char uchar;
    typedef unsigned long time_t;
    typedef unsigned long ulong;
    typedef void *va_list;

    typedef void *(*ServiceInterface) (int Code, ...);

    extern ServiceInterface KernelInterface;

    /* Packs the given data on one double word. */
    unsigned long pci_pack (unsigned bus, unsigned dev, unsigned func);

    /* Changes the register number (8-bits) of a packet. */
    void pci_set_reg (unsigned long *p, unsigned reg);

    /* Changes the bus number (8-bits) of a packet. */
    void pci_set_bus (unsigned long *p, unsigned bus);

    /* Changes the device number (5-bits) of a packet. */
    void pci_set_dev (unsigned long *p, unsigned dev);

    /* Changes the function number (3-bits) of a packet. */
    void pci_set_func (unsigned long *p, unsigned func);

    /* Sends the packet to the PCI config address port. */
    void pci_config_addr (unsigned long p);

    /* Reads one byte from the PCI config data port. */
    unsigned char pci_getb (unsigned offs);

    /* Reads a word from the PCI config data port. */
    unsigned pci_getw (unsigned offs);

    /* Reads the double word from the PCI config data port. */
    unsigned long pci_getd (void);

    /* Writes a byte to the PCI config data port. */
    void pci_setb (unsigned offs, unsigned val);

    /* Writes a word to the PCI config data port. */
    void pci_setw (unsigned offs, unsigned val);

    /* Writes the given double word to the PCI config data port. */
    void pci_setd (unsigned long);

    void disable (void);
    void enable (void);

    void delay (int);

    void outportb (unsigned, unsigned);
    void outportw (unsigned, unsigned);
    void outportd (unsigned, unsigned);

    unsigned inportb (unsigned);
    unsigned inportw (unsigned);
    unsigned inportd (unsigned);

    unsigned long distance (unsigned long dx, unsigned long dy);

#endif
