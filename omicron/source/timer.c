/*
    TIMER.C

    Driver del Timer

    Este modulo contiene la rutina de servicio de interrupcion de la
    IRQ 0, que es el reloj del tiempo del dia. Ademas el timer es el
    que se encargara de hacer multitasking uniforme usando la lista
    de procesos, al timer tambien le llamo el "Cazador de Zombies".

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <process.h>
    #include <portio.h>
    #include <timer.h>

    /* El contador de ticks. */
    unsigned long tick_counter = 0;

    /* Convierte el valor dado en milisegundos a ticks. Hay que mencionar
       que un TICK es la unidad que denota un ciclo del timer programable
       es decir, el 8253/8254. La definicion TIMER_FREQ contiene el numero
       de TICKS que el timer hace en un segundo. */

    uint ms2ticks (uint x)
    {
            return (TIMER_FREQ * (ulong)x) / 1000UL;
    }

    /* Esta es la ISR (Interrupt Service Routine) de la IRQ 0. */
    void irq0_handler (void)
    {
        process_s *nprocess = process, *q, *p;
        int focus, multitasking = !lock;

            /* Incrementar contador de ticks. */
            tick_counter++;

            /* Si estamos en un area no-critica, entonces esto quiere
               decir que podemos cambiar de tareas (multitasking). */

            if (multitasking)
            {
                /* Si no hay proceso en espera, todo sigue normal. */
                if (w_process == NULL)
                {
                    /* Veamos si el proceso actual ya se comio su
                       racion de tiempo, o si se cambio su estado
                       de ejecucion. */

            Normal: if (++nprocess->ticks < nprocess->max_ticks &&
                        nprocess->xst == X_NORMAL)
                    {
                        /* Si todavia no lo ha hecho y sigue en ejecucion
                           normal, no vamos a cambiar de proceso. */

                        multitasking = 0;
                    }
                    else
                    {
                        /* Si ya se le acabo el tiempo o se cambio su xst
                           vamos a resetear su contador, a guardar el estado
                           y a cambiar de proceso (mas adelante). */

                        nprocess->ticks = 0;

                        /* Guardar el estado del proceso. */
                        save_state (&nprocess->state);
                    }

                    focus = 0;
                }
                else
                {
                    /* Si llegamos a este punto entonces hay un proceso
                       en espera, cuando eso pasa se cambia de proceso
                       inmediatamente (el cambio se hace mas adelante). */

                    if (p_process != nprocess) goto Normal;

                    /* Guardar el estado del proceso actual. */
                    save_state (&nprocess->state);

                    focus = 1;
                }
            }

            /* Enviar EOI */
            outportb (0x20, 0x20);

            /* Ahora vamos a cambiar de proceso siempre y cuando
               podamos hacer tal cosa (multitasking = 1). */

            if (multitasking)
            {
                /* Hay que guardar el puntero al proceso actual ya que
                   vamos a cambiarlo. */

                p = nprocess;

                /* Si no hay proceso en espera, simplemente hay que intentar
                   cambiar de proceso (hacer taskswitch). */

                if (!focus)
                {
                    /* Cambiar el proceso actual por el siguiente. */
                    nprocess = p->siguiente;
                }
                else
                {
                    /* Si hay alguien esperando entonces vamos a cambiar de
                       proceso, el destino sera el proceso en espera. */
    
                    nprocess = w_process;
                }

                /* Buscar otro proceso si el proceso destino no se
                   esta ejecutando en modo normal. */

                while (nprocess->xst != X_NORMAL && nprocess->xst != X_WAITRUN)
                {
                    q = nprocess->siguiente;

                    /* Si es un zombie, hay que liquidarlo de una vez por
                       todas, o convertirlo en residente, si es un zombie
                       que quiere ser proceso residente. */

                    if (nprocess->xst == X_ZOMBIE || nprocess->xst == X_RZOMBIE)
                    {
                        if (nprocess->xst == X_ZOMBIE)
                            nprocess->xst = X_KILLED;
                        else
                            nprocess->xst = X_RESIDENT;

                        unlink_process (nprocess);
                    }

                    /* Continuar con el siguiente. */
                    nprocess = q;
                }

                /* Se va a cambiar el proceso principal solo si habia
                   proceso en espera o si el proceso actual es ZOMBIE. */

                if (focus || p->xst == X_ZOMBIE || p->xst == X_RZOMBIE)
                {
                    /* Poner al proceso nuevo como primario. */
                    focus_process (nprocess);

                    w_process = NULL;
                }

                /* Esta funcion cambiara de proceso, siempre y cuando
                   la fuente y destino sean diferentes. */

                if (nprocess != p)
                {
                    /* Poner el estado de ejecucion en NORMAL, solo si
                       no estaba asi antes. */

                    if (nprocess->xst != X_NORMAL) nprocess->xst = X_NORMAL;

                    process = nprocess;

                    /* Cambiar de proceso. */
                    switch_task (&nprocess->state, &p->state);
                }
            }
    }
