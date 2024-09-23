/*
    INIT.C

    Inicializador del Sistema

    La funcion llamada "iniciar_sistema" DEBE ser llamada antes que todo,
    esto hara que todas las cosas necesarias sean iniciadas para su uso
    futuro y no se presenten errores raros.

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <portio.h>
    #include <timer.h>
    #include <keyb.h>

    #include <vfprintf.h>
    #include <process.h>
    #include <kiface.h>
    #include <time.h>
    #include <dev.h>
    #include <mm.h>

    /* Escribe un mensaje en STDERR y elimina el proceso actual. */
    void die (process_s *p, char *s)
    {
           eprintf ("\nProceso %02X (%s) eliminado por %s", p->pid,
                    p->id, s);

           if (kill_process (p) < 0)
           eprintf ("\nALERTA: No se pudo eliminar %s!!!", p->id);
    }

    /* ERROR: Division Entre Cero. */
    void int0_handler (void)
    {
        int i;

            i = send_sig (process, SIGFPE);
            if (i < 0) die (process, "SIGFPE");
    }

    /* ERROR: Desbordamiendo. */
    void int4_handler (void)
    {
        int i;

            i = send_sig (process, SIGOFL);
            if (i < 0) die (process, "SIGOFL");
    }

    /* ERROR: Violacion de Limite de Segmento. */
    void int5_handler (void)
    {
        int i;

            i = send_sig (process, SIGSEGV);
            if (i < 0) die (process, "SIGSEGV");
    }

    /* ERROR: Instruccion Ilegal Ejecutada. */
    void int6_handler (void)
    {
        int i;

            i = send_sig (process, SIGILL);
            if (i < 0) die (process, "SIGILL");
    }

    /* ERROR: Coprocessor Error. */
    void int7_handler (void)
    {
            sCR0 (gCR0 () & ~8);
    }

    /* Dispositivos STDOUT, STDIN y STDERR del kernel. */
    FILE *kstd_out, *kstd_in, *kstd_err;

    /* Inicializa el sistema, si hubo algun error retornara un valor
       negativo, o cero si todo se hizo bien. */

    int iniciar_sistema (void)
    {
        uint i;

            /* Inicializa la consola y el control de procesos. */
            if (instalar_CON () < 0 || iniciar_proceso_k () < 0)
                return -1;

            /* Abre el dispositivo CON en modo lectura y escritura. */
            kstd_in = kstd_out = devopen ("CON", O_RW | O_TEXT);
            if (kstd_in == NULL) return -1;

            /* Inicialmente NO hay dispositivo para errores. */
            kstd_err = NULL;

            /* Iniciar la salida/entrada estandar. */
            kprocess->stdout = kprocess->stdin = kstd_in;

            /* Dirige los vectores de las IRQ utilizadas por el kernel
               a las ISR del kernel. */

            setvect (0x00, (void *)&int0_handler, 0x00000000);
            setvect (0x04, (void *)&int4_handler, 0x00000000);
            setvect (0x05, (void *)&int5_handler, 0x00000000);
            setvect (0x06, (void *)&int6_handler, 0x00000000);
            setvect (0x07, (void *)&int7_handler, 0x00000000);

            setvect (0x20, (void *)&irq0_handler, 0x00000000);
            setvect (0x21, (void *)&irq1_handler, 0x00000000);

            Kernel_Interface (F__REGIFACE, "KERNEL", &Kernel_Interface);

            /* Ahora vamos a reprogramar la velocidad del timer a
               la frecuencia deseada, que es TIMER_FREQ. */

            /* Deshabilitar Interrupciones */
            disable ();

            /* Configurar RTC a modo binario y reloj de 24 horas. */
            outportb (0x70, 0x0B);
            outportb (0x71, 0x02);

            /* Calcular Divisor de Frecuencia (1.19318 MHz / Freq). */
            i = 1193180UL / TIMER_FREQ;

            /* Enviar la palabra de control a el controlador 8253 (PIT). */
            outportb (0x43, 0x36);

            /* Enviar el divisor de la frecuencia. */
            outportb (0x40, i & 0xFF);
            outportb (0x40, i >> 8);

            /* Habilitar solamente las IRQ que deseamos. */
            outportb (0x21, 0xFC);
            outportb (0xA1, 0xFF);

            /* Habilitar las interrupciones. */
            enable ();

            return 0;
    }
