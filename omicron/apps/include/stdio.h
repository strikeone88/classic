/*
    STDIO.H
*/

#ifndef __STDIO_H
#define __STDIO_H

    #ifndef __BASICS_H
    #include <basics.h>
    #endif

    /* Clona un descriptor. */
    #define clone_desc(x) (FILE *)KernelInterface (F__CLONEDESC, (FILE *)(x))

    /* Retorna un puntero a la VDIS si se da el nombre del dispositivo. */
    #define get_dev(x) (dev_s *)KernelInterface (F__GETDEV, (char *)(x))

    /* Registra un dispositivo virtual. */
    #define register_dev(x) (int)KernelInterface (F__REGISTERDEV, (dev_s *)(x))

    /* Remueve un dispositivo virtual. */
    #define unregister_dev(x) (int)KernelInterface (F__UNREGISTERDEV, (dev_s *)(x))

    /* Abre un archivo de dispositivo, ver OPEN_MODES para mayor info. */
    #define devopen(x,y) (FILE *)KernelInterface (F__DEVOPEN, (char *)(x), (int)(y))

    /* Lee "n" (size_t) bytes del archivo en el buffer dado, retorna
       el numero de bytes leidos o un numero negativo si hubo error. */

    #define fread(x,y,z) (int)KernelInterface (F__FREAD, (FILE *)(x), (int)(y), (void *)(z))

    /* Escribe "n" (size_t) bytes del buffer en el archivo y retorna
       el numero de bytes escritor o numero negativo si hubo error. */

    #define fwrite(x,y,z) (int)KernelInterface (F__WRITE, (FILE *)(x), (int)(y), (void *)(z))

    /* Mueve el cursor del archivo a la posicion dada basandose en
       el modo de movimiento (int), ver SEEK_MODES para mayor info. */

    #define fseek(x,y,z) (int)KernelInterface (F__FSEEK, (FILE *)(x), (int)(y), (int)(z))

    /* Cierra un archivo de dispositivo y retorna codigo de error o
       cero si todo salio bien. */

    #define fclose(x) (int)KernelInterface (F__FCLOSE, (FILE *)(x))

    /* Escribe un byte al disp. */
    #define fputc(x,y) (int)KernelInterface (F__PUTC, (FILE *)(x), (int)(y))

    /* Lee un byte del disp. */
    #define fgetc(x) (int)KernelInterface (F__FGETC, (FILE *)(x))

    /* Escribe dos bytes al disp. */
    #define fgetw(x) (int)KernelInterface (F__FGETW, (FILE *)(x))

    /* Lee una dword del archivo. */
    #define fgetd(x) (int)KernelInterface (F__FGETD, (FILE *)(x))

    /* Escribe una cadena al disp. */
    #define fputs(x,y) (int)KernelInterface (F__PUTS, (FILE *)(x), (char *)(y))

    /* vfprintf: puntero a argumentos, archivo, cadena con formato. */
    #define vfprintf(x,y,z) (int)KernelInterface (F__VFPRINTF, (FILE *)(x), (char *)(y), (void *)(z))

    /* vprintf: puntero a argumentos, cadena con formato a STDOUT. */
    #define vprintf(x,y) (int)KernelInterface (F__VPRINTF, (char *)(x), (void *)(y))

    /* fprintf: archivo, cadena con formato, argumentos variables. */
    int fprintf (struct FILE *, const char *, ...);

    /* printf: cadena con formato, argumentos variables a STDOUT. */
    int printf (const char *, ...);

    /* Los modos de movimiento para ejecutar fseek en un archivo. */
    enum SEEK_MODES
    {
        SEEK_SET = 0,   /* Movimiento Absoluto. */

        SEEK_CUR,       /* Movimiento relativo desde la posicion
                           actual. */

        SEEK_END        /* Movimiento relativo desde el final del
                           archivo. */
    };

    /* Los tipos de dispositivos. */
    enum dev_types
    {
        CHRDEV,     /* Dispositivos de caracteres: pantalla, teclado,
                       impresora, etc. */

        BLKDEV,     /* Dispositivos de bloques: Floppies, discos duros,
                       CDs, etc. */
    };

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


    /* La estructura de interfaz con el dispositivo virtual (VDIS). */
    typedef struct dev_s
    {
        uchar           dev_type;
        char            reserved;

        char            *nombre;
        struct dev_s    *siguiente;

        int     (*start)        (void);
        int     (*stop)         (void);

        int     (*open)         (struct FILE *);
        int     (*configure)    (struct FILE *, ...);
        int     (*close)        (struct FILE *);

        size_t  (*read)         (struct FILE *, size_t, void *);
        size_t  (*write)        (struct FILE *, size_t, void *);

        int     (*seek)         (struct FILE *, disp_t, int);
    }
    dev_s;

    /* Class Code 0001 */
    enum CC_0001
    {
        F__CLONEDESC = 0x00010000,  /* (FILE *) : FILE * */
        F__GET_DEV,                 /* (char *) : dev_s * */
        F__REGISTER_DEV,            /* (dev_s *) : int */
        F__UNREGISTER_DEV,          /* (dev_s *) : int */
        F__DEVOPEN,                 /* (char *, int flags) : FILE * */
        F__FREAD,                   /* (FILE *, int, void *) : int */
        F__FWRITE,                  /* (FILE *, int, void *) : int */
        F__FSEEK,                   /* (FILE *, int, int) : int */
        F__FCLOSE,                  /* (FILE *) : int */
        F__FPUTC,                   /* (FILE *, int) : int */
        F__FGETC,                   /* (FILE *) : int */
        F__FGETW,                   /* (FILE *) : int */
        F__FGETD,                   /* (FILE *) : int */
        F__FPUTS,                   /* (FILE *) : int */
        F__VPRINTF,                 /* (char *, void *) : int */
        F__VFPRINTF,                /* (FILE *, char *, void *) : int */
    };

    /* sprintf: buffer, cadena con formato. */
    void sprintf (char *buf, const char *fmt, ...);

    /* vsprintf: puntero a parameters, buffer, cadena con formato. */
    void vsprintf (char *buf, const char *fmt, void *args);

#endif
