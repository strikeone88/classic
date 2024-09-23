/*
    PROCESS.C

    Control de Procesos

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <process.h>
    #include <string.h>
    #include <kiface.h>
    #include <timer.h>
    #include <dev.h>

    /* El kernel es el primer proceso en la lista de multitareas. */
    process_s *kprocess;

    /* El ultimo proceso en la lista de multitareas. */
    static process_s *lprocess;

    /* El proceso que se esta corriendo actualmente. */
    process_s *process;

    /* El proceso PRIMARIO y el proceso en espera corta. */
    process_s *p_process, *w_process;

    /* Este es el primer proceso residente, es una lista LIFO. */
    process_s *resident = NULL;

    /* Esta variable dice si se puede cambiar el proceso o no. */
    static int AREA_CRITICA = 0;

    /* La siguiente identificacion libre. */
    static uint next_pid = 1;

    /* Multitaskign lock. */
    int lock = 0;

    /* Inicializa el proceso principal (kernel). */
    int iniciar_proceso_k (void)
    {
            process = resident = NULL;

            kprocess = (process_s *)kcalloc (sizeof(process_s));
            if (kprocess == NULL) return -1;

            kprocess->id = "Kernel";
            kprocess->pid = 0;

            kprocess->vconsole = (vconsole_s *)kmalloc (sizeof(vconsole_s));
            if (kprocess->vconsole == NULL) return -1;

            init_vconsole (kprocess->vconsole);
            set_vconsole (kprocess->vconsole);

            kprocess->flags = VCON | ACTIVE;
            kprocess->max_ms = 2;
            kprocess->ticks = 0;
            kprocess->max_ticks = ms2ticks (kprocess->max_ms);

            kprocess->siguiente = kprocess->anterior = kprocess;

            p_process = lprocess = process = kprocess;
            w_process = NULL;

            return 0;
    }

    /* Retorna verdadero (1) si el proceso dado es parte del proceso
       principal, ya que puede que sea un hilo. */

    int es_proceso_primario (process_s *p)
    {
            if (p == NULL) return 0;

            if (p == p_process) return 1;

            if (!(p->flags & THREAD)) return 0;

            if (p->parent == p_process) return 1;

            return 0;
    }

    /* Retorna un puntero al proceso que tenga id pid. */
    process_s *find_process (uint pid)
    {
        process_s *p = kprocess;

            enter_critical ();

            do
            {
                if (p->pid == pid)
                {
                    leave_critical ();
                    return p;
                }

                p = p->siguiente;
            }
            while (p != kprocess);

            leave_critical ();

            return NULL;
    }

    /* Igual a find_process pero tambien busca en la lista de procesos
       residentes. */

    process_s *ffind_process (uint pid)
    {
        process_s *p;

            enter_critical ();

            for (p = resident; p != NULL; p = p->siguiente)
            {
                if (p->pid == pid)
                {
                    leave_critical ();
                    return p;
                }
            }

            leave_critical ();

            return find_process (pid);
    }

    /* Retorna el puntero al proceso residente que tenga la misma id
       y la misma firma. */

    process_s *get_resident (char *id, ulong sign)
    {
        process_s *p;

            enter_critical ();

            for (p = resident; p != NULL; p = p->siguiente)
            {
                if (p->res_sign == sign && !strcmp (p->id, id))
                {
                    leave_critical ();
                    return p;
                }
            }

            leave_critical ();

            return NULL;
    }

    /* Se enfoca en un proceso (modifica p_process). */
    void focus_process (process_s *p)
    {
            if (p == NULL || p == p_process) return;

            enter_critical ();

            p_process->flags &= ~ACTIVE;

            send_sig (p_process, SIGUNFOCUS);

            set_vconsole (p->vconsole);
            p_process = p;

            p_process->flags |= ACTIVE;

            send_sig (p_process, SIGFOCUS);

            leave_critical ();
    }

    /* Crea un nuevo proceso vacio y retorna un puntero a su estructura. */
    process_s *new_process (void)
    {
        process_s *p;

            /* Un hilo no puede crear un proceso. */
            if (process->flags & THREAD) return NULL;

            p = (process_s *)kcalloc (sizeof(process_s));
            if (p == NULL) return NULL;

            while (1)
            {
                p->pid = next_pid++;
                if (ffind_process (next_pid - 1) == NULL) break;
            }

            p->stdin = kstd_in;
            p->stdout = kstd_out;
            p->stderr = kstd_err;

            p->vconsole = process->vconsole;

            p->max_ms = 2;
            p->state.EFLAGS = DEF_EFLAGS;

            return p;
    }

    /* Crea una consola virtual para el proceso dado. */
    int new_vconsole (process_s *p)
    {
        vconsole_s *v;

            if (p->flags & VCON) return 0;

            v = (vconsole_s *)kcalloc (sizeof(vconsole_s));
            if (v == NULL) return -NO_MEM;

            enter_critical ();

            p->flags |= VCON;
            p->vconsole = v;

            init_vconsole (v);

            if (p == p_process) set_vconsole (v);

            leave_critical ();

            return 0;
    }

    /* Libera al proceso padre para que siga ejecutandose. */
    void release_parent (process_s *p)
    {
            enter_critical ();

            if (p->parent != NULL && p->parent->xst == X_STOPPED)
            {
                p->parent->xst = X_NORMAL;
                p->parent = NULL;
            }

            leave_critical ();
    }

    /* Deslinkea un proceso de la lista de multitareas. */
    void unlink_process (process_s *p)
    {
        process_s *a, *s;

            if (p == NULL || p->siguiente == NULL || p->anterior == NULL
                || (p->xst != X_KILLED && p->xst != X_RESIDENT)) return;

            enter_critical ();

            a = p->anterior;
            s = p->siguiente;

            if (a->siguiente != p || s->anterior != p) return;

            a->siguiente = s;
            s->anterior = a;

            if (p == lprocess) lprocess = a;

            /* Liberar al proceso padre (si es necesario). */
            release_parent (p);

            /* Liberar la memoria utilizada por el proceso, solo
               si no es un programa residente. */

            if (p->xst != X_RESIDENT) free_process_memory (p);
            else
            {
                /* Si es un proceso residente, lo vamos a enlazar
                   en la lista de residentes. */

                if ((p->siguiente = resident) != NULL)
                    resident->anterior = p;

                p->anterior = NULL;

                /* El primer proceso residente es el ultimo en
                   solicitar residencia (forma LIFO). */

                resident = p;
            }

            leave_critical ();
    }

    /* Linkea el proceso dado a la lista (CIRCULAR!!) de multitareas. */
    void link_process (process_s *p)
    {
            if (p == NULL || p->siguiente != NULL || p->anterior != NULL)
                return;

            enter_critical ();

            p->ticks = 0;
            p->max_ticks = ms2ticks (p->max_ms);

            lprocess->siguiente = p;

            p->anterior = lprocess;
            p->siguiente = kprocess;

            kprocess->anterior = lprocess = p;

            p->parent = process;
            p->xst = X_WAITRUN;

            if (!(p->flags & THREAD))
            {
                w_process = p;

                if (!(p->flags & VCON)) process->xst = X_STOPPED;

                leave_critical ();

                while (p->xst == X_WAITRUN);
            }
            else
            {
                p->next_thr = process->next_thr;
                process->next_thr = p;

                leave_critical ();
            }
    }

    /* Elimina un proceso, si el proceso a eliminar es el que se esta
       ejecutando o es el principal, este se convierte en zombie para
       que sea eliminado por el cazador de zombies (timer). */

    int kill_process (process_s *p)
    {
            /* Si se intenta eliminar el proceso del kernel o si se
               provee un puntero nulo, no se va a hacer nada. */

            if (p == NULL || p == kprocess) return -1;

            /* Si el proceso actual es un hilo, no se hace nada, ya
               que NINGUN hilo puede finalizar su proceso padre. */

            if (process->flags & THREAD) return -1;

            /* Si se intenta matar a un proceso muerto o a un residente
               pues, vamos a retorna ahora mismo. */

            if (p->xst == X_KILLED) return -1;

            /* Darle oportunidad de finalizar (al proceso). */
            send_sig (p, SIGKILL);

            enter_critical ();

            /* Primero se van a eliminar todos los hilos del proceso,
               eso lo hare de manera recursiva, solo para divertirme. */

            kill_process (p->next_thr);

            /* Los hilos de eliminan directamente. */
            if (p->flags & THREAD)
            {
                p->xst = X_KILLED;
                unlink_process (p);

                leave_critical ();
                return 0;
            }

            /* Ver si se puede eliminar el proceso directamente. */
            if (p != process && p != p_process)
            {
                if (p->xst != X_RESIDENT) p->xst = X_KILLED;
                unlink_process (p);
            }
            else
            {
                /* Cambiar de proceso principal, si el que va a morir
                   era el principal. */

                if (p_process == p)
                {
                    if ((w_process = p->parent) == NULL)
                        w_process = p->siguiente;
                }

                /* Ahora hay que crear un zombie o un zombie residente. */
                if (p->xst != X_RESIDENT) p->xst = X_ZOMBIE;
                else p->xst = X_RZOMBIE;

                /* Liberar el proceso padre,... */
                release_parent (p);

                /* estar seguros de que las INT estan activadas,... */
                enable ();

                /* y hacer que el cazador no sea detenido por nadie. */
                AREA_CRITICA = 0;

                /* Ahora solo hay que esperar un ratito. */
                while (1);
            }

            leave_critical ();

            return 0;
    }

    /* Envia una se\xA4al al proceso dado. */
    int send_sig (process_s *p, int sig)
    {
            if (p == NULL) return -ERROR_GENERAL;

            if (p->sig_handler != NULL)
                return p->sig_handler (sig);
            else
                return -NO_SIG_HANDLER;
    }

    /* Entra al area critica (donde no se puede hacer multitask). */
    void enter_critical (void)
    {
            AREA_CRITICA++;
    }

    /* Sale del area critica. */
    void leave_critical (void)
    {
            if (AREA_CRITICA > 0) AREA_CRITICA--;
    }

    /* Retorna el numero de areas criticas a las que se ha entrado
       recursivamente. */

    int critical (void)
    {
            return AREA_CRITICA;
    }

    /* Crea y opcionalmente corre un proceso hijo. */
    int fspawnv (FILE *fp, int mode, char *name, char *cmdtail, ulong *xlen)
    {
        ulong image_len, fixups, stack_offs, stack_len, ioffs, entry, t;
        void *stack = NULL, *buf = NULL;
        process_s *ps;
        int i;

            ps = new_process ();
            if (ps == NULL)
            { no_mem:;
                i = -NO_MEM;

            reti:;
                kfree (stack);

                if (ps)
                {
                    kfree (ps->id);
                    kfree (ps);
                }

                kfree (buf);

                return i;
            }

            chblkown (ps, ps);

            if (!name) name = "UNKPS";

            ps->id = (char *)kmalloc (i = strlen (name) + 1);
            if (ps->id == NULL) goto no_mem;

            memcpy (ps->id, name, i);
            strupr (ps->id);

            chblkown (ps->id, ps);

            if (fgetd (fp) != 'CFX!')
            {
                i = -INV_EXE;
                goto reti;
            }

            image_len = fgetd (fp);
            fixups = fgetd (fp);

            stack_offs = fgetd (fp);
            stack_len = fgetd (fp);

            entry = fgetd (fp);
            ioffs = fgetd (fp);

            if (xlen) *xlen = image_len + ioffs;

            fgetd (fp);

            if (!stack_len)
            {
                stack = kmalloc (AUTO_STACK);
                if (stack == NULL) goto no_mem;

                chblkown (stack, ps);
            }

            buf = kmalloc (image_len);
            if (buf == NULL) goto no_mem;

            fseek (fp, ioffs, SEEK_SET);

            if (fread (fp, image_len, buf) != image_len)
            {
                i = -E_LECTURA;
                goto reti;
            }

            if (mode == P_NOWAIT)
            {
                i = new_vconsole (ps);
                if (i < 0) goto reti;

                chblkown (ps->vconsole, ps);
            }

            fseek (fp, 0x20, SEEK_SET);

            while (fixups--)
            {
                t = fgetd (fp) + (ulong)buf;
                (*(ulong *)t) += (ulong)buf;
            }

            if (stack_len)
                ps->state.ESP = stack_offs + stack_len + (ulong)buf;
            else
                ps->state.ESP = AUTO_STACK + (ulong)stack;

            ps->state.EIP = entry + (ulong)buf;

            ps->state.SS = 0x10;
            ps->state.CS = 0x08;

            ps->state.DS = 0x10;
            ps->state.ES = 0x10;

            ps->state.EBX = (ulong)&Kernel_Interface;
            ps->state.EAX = (ulong)ps;

            ps->cmdtail = cmdtail;

            chblkown (buf, ps);

            if (!(mode & P_LOADONLY)) link_process (ps);

            return 0;
    }

    /*
      @Autor: Juan H. Rodas (juanrodas85@gmail.com)
      @Fecha: 16/07/08
    */
    
    /*busca un proceso por medio del atributo ID*/
     process_s *find_process_id(const char* process_name)
     {
       process_s *p=kprocess;
       do
       {
         if(strcmp(process_name,p->id)==0) break;   
         p=p->siguiente; 
       }while(p!=kprocess); 
       if(p==lprocess) return NULL;
       return p;
     }
