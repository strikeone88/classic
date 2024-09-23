/*
    TIMER.H

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __TIMER
#define __TIMER

    /* La frecuencia del timer, init.c se encarga de configurarlo. */
    #define TIMER_FREQ      500UL

    /* Convierte de ms a ticks. */
    uint ms2ticks (uint);

    /* En handler de la irq0 (timer). */
    void irq0_handler (void);

    /* El contador de ticks. */
    extern unsigned long tick_counter;

#endif
