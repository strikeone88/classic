/*
    KEYB.H

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __KEYB
#define __KEYB

    #ifndef __TYPES
    #include <types.h>
    #endif

    /* Define el maximo numero de teclas que caben en el buffer. */
    #define MAX_TECLAS      128

    /* Los modos en los que se puede encontrar la ISR. */
    enum isr_modes
    {
        READ_FN =   0x01
    };

    /* Los indicadores (flags) del teclado. */
    enum keyb_flags
    {
        /* Bits 0-1 */
        L_SHIFT     = 0x01,     R_SHIFT     = 0x02,     X_SHIFT     = 0x03,

        /* Bits 2-3 */
        L_CTRL      = 0x04,     R_CTRL      = 0x08,     X_CTRL      = 0x0C,

        /* Bits 4-5 */
        L_ALT       = 0x10,     R_ALT       = 0x20,     X_ALT       = 0x30,

        /* Bits 6-8 */
        FLAG_SHIFT  = 6,        FLAG_MASK   = 0x07,
        SCROLL_LOCK = 0x40,     NUM_LOCK    = 0x80,     CAPS_LOCK   = 0x100,

        /* Bits 9-11 */
        LOCK_SL     = 0x200,    LOCK_NL     = 0x400,    LOCK_CL     = 0x800,
    };

    void irq1_handler (void);

    /* Estas son las flags del teclado. */
    extern flag_t kbdf;

    /* El numero de la tecla de funcion F(1..12) */
    extern int Fn;

    /* Retorna el numero de teclas en el buffer del teclado. */
    int kbhit (void);

    /* Retorna un par SCANCODE:ASCII del buffer del teclado, si no hay
       nada, espera a que haya algo. */

    uint getch (void);

#endif
