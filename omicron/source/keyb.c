/*
    KEYB.C

    Driver de Teclado Estandar

    Este driver funciona para TODOS los teclados que hay (al menos hasta
    donde yo se) si se quieren agregar nuevos codigos de rastreo se puede
    hacer facilmente.

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <process.h>
    #include <portio.h>
    #include <keyb.h>

    /* Estas son las flags del teclado. */
    static flag_t kbdf = 0;

    /* El numero de la tecla de funcion F(1..12) */
    static int Fn = 0;

    /* Este es el buffer de teclado (una cola). */
    static ushort kbd_buf [MAX_TECLAS], kbd_tail = 0;

    /* A hora se presenta un bloque de arreglos, cada arreglo contiene N
       caracteres, cada caracter representa el ScanCode de una tecla. */

    /* Caracteres tipo Alpha (las letras de la A a la Z). */
    static char alpha_s [] =
    {
        0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25,
        0x26, 0x32, 0x31, 0x18, 0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F,
        0x11, 0x2D, 0x15, 0x2C
    };

    /* Todos estos scan codes obtendran un ASCII de cero. */
    static char zero_s [] =
    {
        0x53, 0x50, 0x4F, 0x47, 0x52, 0x4B, 0x51, 0x49, 0x4D, 0x48
    };

    /* Esta se utiliza cuando se presiona shift mas un ch extendido. */
    static char shift_zero_s [] =
    {
        0x2E, 0x32, 0x31, 0x37, 0x30, 0x34, 0x33, 0x39, 0x36, 0x38
    };

    /* Aqui van los scan codes que no tienen un patron ASCII, a cada
       indice le corresponde un elemento de misc_a, que es el ASCII. */

    static char misc_s [] =
    {
        0x0E, 0x1C, 0x01, 0x4C, 0x37, 0x4A, 0x4E, 0x35, 0x39, 0x0F, 0x0C,
        0x0D, 0x1A, 0x1B, 0x27, 0x28, 0x29, 0x2B, 0x33, 0x34, 0x35
    };

    /* Aqui estan los codigo ASCII para cada indice de misc_s. */
    static char misc_a [] =
    {
        0x08, 0x0D, 0x1B, 0x35, 0x2A, 0x2D, 0x2B, 0x2F, 0x20, 0x09, 0x2D,
        0x3D, 0x5B, 0x5D, 0x3B, 0x27, 0x60, 0x5C, 0x2C, 0x2E, 0x2F
    };

    /* Aqui estan los codigo ASCII para cada indice de misc_s cuando
       la tecla shift esta presionada. */

    static char shift_misc_a [] =
    {
        0x08, 0x0D, 0x1B, 0x35, 0x2A, 0x2D, 0x2B, 0x3F, 0x20, 0x00, 0x5F,
        0x2B, 0x7B, 0x7D, 0x3A, 0x22, 0x7E, 0x7C, 0x3C, 0x3E, 0x3F
    };

    /* Esta tabla es usada cuando se presiona SHIFT con un numero. */
    static char shift_num [] = ")!@#$%^&*(";

    /* Esta funcion escanea el caracter dado en el arreglo dado, si lo
       encuentra retorna el indice + 1, o cero si no fue encontrado. */

    static index_t buscar_elem (char *q, count_t n, char x)
    {
        index_t i = 0;

            while (n--) if (q [i++] == x) return i;
            return 0;
    }

    /* Retorna el numero de teclas en el buffer del teclado. */
    int kbhit (void)
    {
            /* Solo el proceso PRIMARIO tiene acceso al buffer
               del teclado. */

            if (!es_proceso_primario (process)) return 0;

            return kbd_tail;
    }

    /* Retorna un par SCANCODE:ASCII del buffer del teclado, si no hay
       nada, espera a que haya algo. */

    uint getch (void)
    {
        uint k; int i;

            /* Vamos a esperar que haya tecla y que el proceso actual
               sea el primario. */

            while (!es_proceso_primario (process) || !kbd_tail);

            /* Ahora solo hay que mover las teclas restantes a la
               izquierda una posicion. */

            for (k = kbd_buf [i = 0], kbd_tail--; i < kbd_tail; i++)
                kbd_buf [i] = kbd_buf [i + 1];

            /* Hora de retornar el par SCANCODE:ASCII. */
            return k;
    }

    /* Envia un dato al controlador de teclado (8042), si wait=1 entonces
       se esperara a que se retorne ACK (valor 0xFA). */

    static void kbd_out (uchar data, int wait)
    {
            while (inportb (0x64) & 0x02);
            outportb (0x60, data);

            while (wait)
            {
                while (!(inportb (0x64) & 0x01));
                if (inportb (0x60) == 0xFA) break;
            }
    }

    /* Esta es la ISR de la IRQ 1, que es una peticion de servicio a
       el teclado, se usara el controlador de teclado 8042, a quien
       le pertenecen los puertos 0x60, 0x61 y 0x64. */

    void irq1_handler (void)
    {
        uchar a = 0, s, x, flush = 0; index_t i;
        static int Mode = 0, _Fn;
        static process_s *p, *q;
        long l;

            /* Hay que chequear que la interrupcion haya sido generada
               por hardware, y no por software, si el software lo hizo
               entonces el 8042 no deberia tener datos listos para que
               el sistema los lea.
            */

            if (!(inportb (0x64) & 1)) return;

            /* Leer el scan code. */
            s = inportb (0x60);

            /* "x" es el scan code puro, sin bit 7, el cual denota si
               es un break code. */

            x = s & 0x7F;

            /* Antes de cualquier cosa vamos a ver si es una tecla de
               control de seguros (NumLock, ScrollLock, CapsLock). */

            /* NumLock. */
            if (x == 0x45)
            {
                if (s & 0x80)
                {
                    kbdf &= ~(flag_t)LOCK_NL;
                    goto nothing;
                }

                if (kbdf & (i = LOCK_NL)) goto nothing;

                if (kbdf & NUM_LOCK) kbdf &= ~NUM_LOCK;
                else kbdf |= NUM_LOCK;

            cambiar_luces:;
                kbd_out (0xED, 1);
                kbd_out ((kbdf >> FLAG_SHIFT) & FLAG_MASK, 1);

                kbdf |= i;

                goto nothing;
            }

            /* CapsLock. */
            if (x == 0x3A)
            {
                if (s & 0x80)
                {
                    kbdf &= ~(flag_t)LOCK_CL;
                    goto nothing;
                }

                if (kbdf & (i = LOCK_CL)) goto nothing;

                if (kbdf & CAPS_LOCK) kbdf &= ~(flag_t)CAPS_LOCK;
                else kbdf |= CAPS_LOCK;
                goto cambiar_luces;
            }

            /* ScrollLock. */
            if (x == 0x46)
            {
                if (s & 0x80)
                {
                    kbdf &= ~LOCK_SL;
                    goto nothing;
                }

                if (kbdf & (i = LOCK_SL)) goto nothing;

                if (kbdf & SCROLL_LOCK) kbdf &= ~SCROLL_LOCK;
                else kbdf |= SCROLL_LOCK;
                goto cambiar_luces;
            }

            /* Hay que ver si es una tecla de control, como ALT,
               CTRL, o SHIFT. */

            if (x == 0x2A) /* Shift Izquierdo */
            {
                if (s & 0x80) kbdf &= ~L_SHIFT; else kbdf |= L_SHIFT;
                goto done;
            }

            if (x == 0x36) /* Shift Derecho */
            {
                if (s & 0x80) kbdf &= ~R_SHIFT; else kbdf |= R_SHIFT;
                goto done;
            }

            if (x == 0x1D) /* Ctrl Izquierdo */
            {
                if (s & 0x80) kbdf &= ~L_CTRL; else kbdf |= L_CTRL;
                goto done;
            }

            if (x == 0x38) /* Alt Izquierdo */
            {
                if (s & 0x80) kbdf &= ~L_ALT; else kbdf |= L_ALT;
                goto done;
            }

            /* Si es un break code y no un make code, entonces vamos a
               retornar ahora mismo. */

            if (s & 0x80)
            { nothing:;
                Fn = 0;
                goto done;
            }

            /* Ahora hay que traducir el scan code en un caracter
               ASCII valido. */

            if (s >= 0x57 && s <= 0x58) /* Teclas F11..F12 */
            {
                Fn = s - 0x62;
                goto set_flush;
            }

            if (s >= 0x3B && s <= 0x44) /* Teclas F1..F10 */
            {
                Fn = (a = s) - 0x3A;

                if (kbdf & X_SHIFT) s = a + 0x54 - 0x3B;
                if (kbdf & X_CTRL) s = a + 0x5E - 0x3B;
                if (kbdf & X_ALT) s = a + 0x68 - 0x3B;

                a = 0;
                goto set_flush;
            }

            /* Si el scan code esta entre 2 y 11, entonces es un caracter
               numerico. */

            if (s >= 0x02 && s <= 0x0B)
            {
                if (s == 0x0B)
                    a = 0x30;
                else
                    a = 0x2F + s;

                if (kbdf & X_SHIFT) a = shift_num [a - 0x30];

                goto set_flush;
            }

            /* Si es un caracter alpha, entonces el ASCII es simplemente
               el indice mas 97. */

            if ((i = buscar_elem (alpha_s, array_size(alpha_s), s)) != 0)
            {
                a = (i + 96) & (kbdf & X_SHIFT ? 0xDF : 0xFF);

                if (kbdf & X_CTRL)
                    a = (a & 0xDF) - 0x40;

                if (kbdf & X_ALT)
                    a = 0;

                goto set_flush;
            }

            /* Si es un caracter que pertenece a un bloque que no tiene un
               patron ASCII definido, entonces el ASCII se encuentra en
               el arreglo misc_a. */

            if ((i = buscar_elem (misc_s, array_size(misc_s), s)) != 0)
            {
                if (kbdf & X_SHIFT)
                    a = shift_misc_a [i - 1];
                else
                    a = misc_a [i - 1];

                goto set_flush;
            }

            /* Si es un caracter extendido, entonces el ASCII es cero. */
            if ((i = buscar_elem (zero_s, array_size(zero_s), s)) != 0)
            {
                a = 0;

                if ((kbdf & NUM_LOCK) || (kbdf & X_SHIFT))
                    a = shift_zero_s [i - 1];

                goto set_flush;
            }

            goto done;

        set_flush:;
            flush = 1;

        done:;
            /* Ver si se presiono CTRL+C. */
            if ((kbdf & X_CTRL) && s == 0x2E)
            {
                /* Entrar a area critica. */
                enter_critical ();

                /* Habilitar Interrupciones. */
                enable ();

                /* Enviar EOI. */
                outportb (0x20, 0x20);

                if (send_sig (p_process, SIGINT) < 0)
                {
                    /* Eliminar proceso. */
                    die (p_process, "SIGINT");
                }

                /* Salir del area critica. */
                leave_critical ();

                return;
            }

            /* Aqui se chequean las teclas especiales. */
            if (Fn > 0 && Fn < 11 && (kbdf & L_SHIFT) && (kbdf & L_CTRL))
            {
                if (Mode != READ_FN)
                {
                    Mode = READ_FN;
                    _Fn = 0;
                }

                _Fn = (_Fn * 10) + (Fn % 10);

                flush = 0;
            }

            /* Aqui se verifican los estados. */
            if (!(kbdf & L_SHIFT) && !(kbdf & L_CTRL) && Mode == READ_FN)
            {
                Mode = 0;

                p = find_process (_Fn);

                if (!(p->flags & THREAD) && p != NULL)
                {
                    while (p->xst == X_STOPPED)
                    {
                        q = p->siguiente;
                        while (q->parent != p) q = q->siguiente;
                        p = q;
                    }

                    w_process = p;
                    kbd_tail = 0;
                }

                flush = 0;
            }

            if (kbd_tail > MAX_TECLAS - 1 || !flush)
            {
            }
            else
                kbd_buf [kbd_tail++] = (uint)s << 8 | a;

            /* Ahora solo hay que enviar el codigo de EOI (End of Interrupt)
               al controlador 8259, Programmable Interrupt Controller (PIC).
            */

            outportb (0x20, 0x20);
    }
