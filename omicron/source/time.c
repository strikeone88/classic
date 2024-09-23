/*
    TIME.C

    Proveedor y Manipulador del Tiempo

    Este modulo es el encargado de controlar el reloj del sistema, si
    se necesita la hora, fecha o lo que sea, se le debe preguntar a
    este. Se utiliza el Real Time Clock (puertos 0x70 y 0x71).

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <portio.h>
    #include <time.h>

    /* La tabla de dias que han pasado entre meses. */
    static uint dTbl [] =
    {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };

    /* Retorna el total de segundos que han pasado entre dos fechas,
       repara para a\xA5os bisiestos, pero no para centurias. */

    static time_t segundosPasados (int y1, int m1, int d1, int y2, int m2, int d2)
    {
        time_t d, ydelta = y2 - y1;

            if (!(y1 & 3) && m1 > 1) d1++;
            if (!(y2 & 3) && m2 > 1) d2++;

            d = dTbl [m2] - dTbl [m1] + d2 - d1 +
                (ydelta >> 2) + (ydelta + 1 & 3 ? 0 : 1) +
                (y2 - y1) * 365UL;

            return d * (24UL * 3600UL);
    }

    /* Convierte de BCD a binario. */
    uint getBin (uint x)
    {
            return (x >> 4) * 10 + (x & 0x0F);
    }

    /* Empaqueta la estructura tm a un time_t. */
    time_t packTime (tm *t)
    {
            return segundosPasados (1988, 0, 1, t->year, t->month, t->day) +
                   t->hour * 3600UL + t->min * 60UL + t->sec;
    }

    /* Desempaqueta un time_t en una estructura tm. */
    tm *unpackTime (time_t x, tm *t)
    {
        int i; uint y;

            if (t == NULL) return NULL;

            t->sec = x % 60;
            x /= 60;

            t->min = x % 60;
            x /= 60;

            t->hour = x % 24;
            x /= 24;

            y = t->year = x / 365;
            x -= y * 365;

            for (i = 0; i < 12; i++)
                if (x >= dTbl [i] && x <= dTbl [i + 1]) break;

            t->month = i;
            t->day = 1 + x - dTbl [i] - (y >> 2) - (y + 1 & 3 ? 0 : 1) -
                    ((y + 2004) & 3 ? 0 : 1);

            t->year += 1988;

            return t;
    }

    /* Lee un byte de un registro del RTC. */
    static uint readRTC (int r)
    {
            outportb (0x70, 0x0A);
            while (inportb (0x71) & 0x80);

            outportb (0x70, r);
            return inportb (0x71);
    }

    /* Retorna el total de segundos desde 00:00:00 GMT 1st Jan 1988. */
    time_t gtime (void)
    {
        time_t a, b;
        tm t;

     Retry: t.hour = getBin (readRTC (0x04)) % 24;
            t.min = getBin (readRTC (0x02)) % 60;
            t.sec = getBin (readRTC (0x00)) % 60;
            t.year = getBin (readRTC (0x09)) + getBin (readRTC (0x32)) * 100;
            t.month = getBin (readRTC (0x08)) % 13 - 1;
            t.day = getBin (readRTC (0x07)) % 32;
            a = packTime (&t);

            t.hour = getBin (readRTC (0x04)) % 24;
            t.min = getBin (readRTC (0x02)) % 60;
            t.sec = getBin (readRTC (0x00)) % 60;
            t.year = getBin (readRTC (0x09)) + getBin (readRTC (0x32)) * 100;
            t.month = getBin (readRTC (0x08)) % 13 - 1;
            t.day = getBin (readRTC (0x07)) % 32;
            b = packTime (&t);

            if (a != b) goto Retry;

            return a;
    }
