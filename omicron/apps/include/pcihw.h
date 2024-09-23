/*
    PCIHW.H

    PCI Hardware Interface Helper (Header File)

    Copyright (C) 2006 RedStar Technologies
    Written by J. Palencia (zipox@ureach.com)
*/

#ifndef __PCIHW
#define __PCIHW

    #ifdef __cplusplus
    extern "C"
    {
    #endif

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

    #ifdef __cplusplus
    }
    #endif

#endif
