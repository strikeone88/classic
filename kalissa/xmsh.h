/*
    XMSH.H

    XMS Helper Version 0.1 -- Interface

    Copyright (C) 2007 RedStar Technologies
    Written by J. Palencia (zipox@ureach.com)
*/

#ifndef __XMSH_H
#define __XMSH_H

    /* Structure used to move data between memory blocks. */
    typedef struct
    {
        unsigned long   length;

        unsigned        source_handle;
        unsigned long   source_offs;

        unsigned        dest_handle;
        unsigned long   dest_offs;
    }
    moveEMBst_t;

    /* Checks if there is a valid XMS driver, returns zero if success,
       one if no driver is installed and two if driver has a version
       below 2.00. */

    int checkXMS (void);

    /* Allocates "n" bytes of extended memory and returns handle,
       if zero, function was not able to allocate memory block. */

    unsigned allocEMB (unsigned long);

    /* Releases the given extended memory block. */
    void freeEMB (unsigned);

    /* Moves data between the memory blocks. */
    int moveEMB (unsigned src, unsigned long src_offs, unsigned dst,
                 unsigned long dst_offs, unsigned long count);

    /* Moves data between the memory blocks (given a move structure). */
    int moveEMB_st (moveEMBst_t *);

#endif
