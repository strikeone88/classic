/*
    MAIN.C

    Inicializador del Kernel y Modulo Principal

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <vfprintf.h>
    #include <process.h>
    #include <time.h>
    #include <dev.h>

    /* Inicializa el manejador de memoria. */
    void *start_mm (size_t krnl);

    /* El punto de entrada del kernel. */
    int main (size_t totalMem, ulong kernelStart, ulong kernelLen)
    {
        ulong bottom, p, q, r, t, lt;
        FILE *STREAM;
        tm time;
        int i;

            /* Inicializa el sistema de gestion de memoria. */
            start_mm (totalMem);

            /* Esta funcion hace de todo :) */
            if (iniciar_sistema () < 0) return 0;

            printf ("\nOmicron-32 Version 1.00 Ä Nucleus N8Y3EC0\n");

            /* Crear un archivo virtual utilizando CON. */
            STREAM = devopen ("CON", O_READ);
            STREAM->mem_size = 0x1000000; //16M
            STREAM->flags |= O_RIN;

            bottom = kernelStart + kernelLen;

            /* Correr los procesos integrados. */
            while (1)
            {
                STREAM->mem_ptr = (void *)bottom;
                STREAM->pos = 0;

                printf ("fspawnv (%08lx): ", bottom);

                if ((i = fspawnv (STREAM, P_WAIT, NULL, NULL, &p)) != 0)
                {
                    printf ("fspawnv error %i\n", i);
                    break;
                }

                bottom += p;
            }

            printf ("\nOmicron: F1: Restart, F2: Halt, F3: Memory Node Dump\n\n");

            totalMem >>= 10;
            i = lt = 0;

            while (1)
            {
                while (!kbhit ())
                {
                    if ((t = gtime ()) == lt) continue;

                    p = q = r = kcoreleft () >> 10;
                    i = 0;

                    q = 100 * (q & 1023) >> 10;
                    p >>= 10;

                    unpackTime (lt = t, &time);

                    printf ("\r%02u:%02u:%02u : [ %lu.%02lu mb free, %lu mb total (%u%% used, %u.%04u%% fragmented)]",
                        time.hour, time.min, time.sec, p, q,
                        totalMem >> 10, ((totalMem - r) * 100 / totalMem),
                        (kdcoreleft () >> 10) * 100 / r,
                        1000 * ((kdcoreleft () >> 10) * 100 % r) / r,
                        );
                }

                i = getch ();

                if (i == 0x3B00) return 0;
                if (i == 0x3C00) break;

                if (i == 0x3D00)
                {
                    printf ("\n");
                    dump_mem_nodes (kprocess->stdout, ALL_PROCESSES);
                    printf ("\n");
                }
            }

            printf ("\n\nOmicron: System Halted");
            return 1;
    }
