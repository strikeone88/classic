/*
    DEV.H

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __DEV
#define __DEV

    #ifndef __FILE
    #include <file.h>
    #endif

    #ifndef __TYPES
    #include <types.h>
    #endif

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

    /* Clona un descriptor. */
    struct FILE *clonar_descriptor (struct FILE *fp);

    /* Retorna un puntero a la VDIS si se da el nombre del dispositivo. */
    dev_s *get_dev (char *);

    /* Registra un dispositivo virtual. */
    int register_dev (dev_s *);

    /* Remueve un dispositivo virtual. */
    int unregister_dev (dev_s *);

    /* Abre un archivo de dispositivo, ver OPEN_MODES para mayor
       informacion de lo que va en la flag_t. */

    struct FILE *devopen (char *, flag_t);

    /* Lee "n" (size_t) bytes del archivo en el buffer dado, retorna
       el numero de bytes leidos o un numero negativo si hubo error. */

    size_t fread (struct FILE *, size_t, void *);

    /* Escribe "n" (size_t) bytes del buffer en el archivo y retorna
       el numero de bytes escritor o numero negativo si hubo error. */

    size_t fwrite (struct FILE *, size_t n, void *);

    /* Mueve el cursor del archivo a la posicion dada basandose en
       el modo de movimiento (int), ver SEEK_MODES para mayor info. */

    int fseek (struct FILE *, disp_t, int);

    /* Cierra un archivo de dispositivo y retorna codigo de error o
       cero si todo salio bien. */

    int fclose (struct FILE *);

    /* Escribe un byte al disp. */
    int fputc (struct FILE *, int);

    /* Lee un byte del disp.*/
    int fgetc (struct FILE *);

    /* Escribe dos bytes al disp. */
    int fgetw (struct FILE *);

    /* Lee una dword del archivo. */
    unsigned long fgetd (struct FILE *fp);

    /* Escribe una cadena al disp. */
    int fputs (struct FILE *, const char *);

#endif
