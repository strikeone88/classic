/*
    FILE.H

    Descriptor de Archivo

    Este archivito de encabezado contiene el formato de un descriptor de
    archivo el cual es utilizado practicamente para TODO!!.

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __FILE
#define __FILE

    #ifndef __TYPES
    #include <types.h>
    #endif

    #ifndef __DEV
    #include <dev.h>
    #endif

    /* Bits de comportamiendo para FILE.FLAGS, tambien conocido
       como MODO DE ABRIR EL ARCHIVO (OPEN MODE). */

    enum BEHAVIOR_BITS
    {
        O_READ  =   0x001,  /* Bits 0-1 */
        O_WRITE =   0x002,
        O_RW    =   0x003,

        O_TEXT  =   0x008,  /* Bit 3 */

        O_APPEND =  0x010,  /* Bits 4-5 */
        O_TRUNC =   0x020,

        O_RIN   =   0x080,  /* Bits 7-8 */
        O_ROUT  =   0x100
    };

    /* Descriptor de Medio */
    enum MEDIA_TYPES
    {
        REMOVABLE_MEDIA,    /* Medio removible: floppies, CDs, etc. */
        FIXED_MEDIA,        /* Medio fijo: disco duro, ?, etc. */
        RAMDISK_MEDIA       /* Medio RAMDISK: Obvio!! */
    };

    /* La estructura de un descriptor de archivo. */
    typedef struct FILE
    {
        uint            process_id; /* Proceso que creo el descriptor. */
        int             error_c;    /* Codigo de error (si hubo). */
        ulong           flags;      /* Flags de comportamiento. */
        struct dev_s    *dev;       /* Puntero a dispositivo controlador. */
        ulong           pos;        /* Cursor del archivo. */
        ulong           size;       /* Total de bytes en el archivo. */
        void            *mem_ptr;   /* Puntero a bloque de memoria. */
        uint            mem_size;   /* Total de bytes el el bloque. */
        uint            sel_dev;    /* Dispositivo FISICO seleccionado. */
        uchar           dev_type;   /* Tipo de dispositivo. */
        uchar           media_type; /* Descriptor de medio. */
        ulong           available;  /* Libre para ser utilizado. */
        struct FILE     *siguiente; /* Siguiente descriptor. */
    }
    FILE;

#endif
