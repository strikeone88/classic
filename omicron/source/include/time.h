/*
    TIME.H

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __TIME
#define __TIME

    #ifndef __TYPES
    #include <types.h>
    #endif

    /* La estructura de tiempo, separado en partes. */
    typedef struct tm
    {
        uint    year, month, day;
        uint    hour, min, sec;
    }
    tm;

    /* Inicializa el RTC en modo binario, reloj de 24 horas. */
    void initRTC (void);

    /* Empaqueta la estructura tm a un time_t. */
    time_t packTime (tm *);

    /* Desempaqueta un time_t en una estructura tm. */
    tm *unpackTime (time_t, tm *);

    /* Retorna el total de segundos desde 00:00:00 GMT 1st Jan 1988. */
    time_t gtime (void);

#endif
