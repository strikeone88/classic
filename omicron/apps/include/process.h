/*
    PROCESS.H
*/

#ifndef __PROCESS_H
#define __PROCESS_H

    #ifndef __BASICS_H
    #include <basics.h>
    #endif

    #ifndef __STDIO_H
    #include <stdio.h>
    #endif

    /* Retorna un puntero a el descriptor del proceso dado el indice. */
    #define find_process(x) (process_s *)KernelInterface (F__FIND_PROCESS, (int)(x))

    /* Igual a find_process pero tambien busca en la lista de procesos
       residentes. */

    #define ffind_process(x) (process_s *)KernelInterface (F__FFIND_PROCESS, (int)(x))

    /* Retorna el puntero al proceso residente que tenga la misma id
       y la misma firma. */

    #define get_resident(x,y) (process_s *)KernelInterface (F__GET_RESIDENT, (int)(x), (long)(y))

    /* Crea una consola virtual para el proceso dado. */
    #define new_console(x) (int)KernelInterface (F__NEW_CONSOLE, (process_s *)(x))

    /* Libera al proceso padre. */
    #define release_parent(x) (void)KernelInterface (F__RELEASE_PARENT, (process_s *)(x))

    /* Envia una se\xA4al al proceso indicado. */
    #define seng_sig(x,y) (int)KernelInterface (F__SEND_SIG, (process_s *)(x), (int)(y))

    /* Crea y opcionalmente corre un proceso hijo. */
    #define fspawnv(a,b,c,d,e) (int)KernelInterface (F__FSPAWNV, (FILE *)(a), (int)(b), (char *)(c), (char *)(d), (long *)(e))

    /* Finaliza un proceso. */
    #define kill_process(x) (int)KernelInterface (F__KILL_PROCESS, (process_s *)(x))

    /* Busca un proceso por medio del atributo ID. */
    #define find_process_id(x) (process_s *)KernelInterface (F__FIND_PROCESS_ID, (char *)(x))

    /* Termina el proceso actual. */
    #define exit(x) KernelInterface (F__EXIT, (int)(x))

    /* Bloquea el multiprocesamiento. */
    #define lock() KernelInterface (F__LOCK)

    /* Desbloquea el multiprocesamiento. */
    #define unlock() KernelInterface (F__UNLOCK)

    /* Makes the process resident. */
    #define setResident() selfProcess->xst = X_RESIDENT

    /* Los modos de ejecucion que se pasan a spawn. */
    #define P_NOWAIT        0x00    /* Bit 0 */
    #define P_WAIT          0x01
    #define P_LOADONLY      0x02    /* Bit 1 */

    /* Valores que se le pueden dar a process_s.xst */
    enum xst_vals
    {
        X_NORMAL,   /* Ejecucion normal. */
        X_STOPPED,  /* El proceso ha sido detenido. */
        X_ZOMBIE,   /* Deberia estar muerto, pero no lo esta. */
        X_RZOMBIE,  /* El proceso no esta muerto, deberia estarlo pero
                       se va a convertir en un proceso residente. */
        X_KILLED,   /* Ha sido eliminado. */
        X_WAITRUN,  /* El proceso esta esperando correr por 1ra vez. */
        X_RESIDENT  /* El proceso es un programa residente. */
    };

    /* Indicadores de un proceso. */
    enum process_flags
    {
        THREAD  =   0x01,       /* Indica si es un hilo. */
        VCON    =   0x02,       /* Posee consola virtual propia. */
        ACTIVE  =   0x04,       /* Ha sido activado por el usuario. */
    };

    /* La lista de se\xA4ales. */
    enum signals
    {
        SIGINT      =   0x02,   /* Ctrl+C Fue Presionada */
        SIGOFL      =   0x03,   /* Desbordamiendo */
        SIGILL      =   0x04,   /* Instruccion Ilegal */
        SIGABRT     =   0x06,   /* Abortar Ejecucion */
        SIGFPE      =   0x08,   /* Div. Entre Cero */
        SIGKILL     =   0x09,   /* Orden Eliminacion */
        SIGSEGV     =   0x0B,   /* Violacion de Limites de Segmento */
        SIGTERM     =   0x0F,   /* Peticion Terminacion */
        SIGUNFOCUS  =   0x10,   /* Deactivacion de Proceso Primario */
        SIGFOCUS    =   0x11,   /* Activacion de Proceso Primario */
    };

    typedef struct /* 28 bytes */
    {
        /* Estos son los registros de la arq. x86 32-bits. */

        /* Registros de Proposito General. */
        uint        EAX, ECX, EDX, EBX, EBP, ESI, EDI;

        /* Registros de Segmentos. */
        uint        DS, ES;

        /* Registros Utilizados por switch_task. */
        uint        ESP, SS, EIP, CS, EFLAGS;

        /* Todos son visibles al usuario excepto EIP y EFLAGS
           que no se pueden acceder directamente, pero aun
           asi son accesibles.
        */
    }
    state_s;

    /* La estructura de un descriptor de proceso. */
    typedef struct process_s
    {
        char        *id;                    /* Nombre del Proceso */

        /* xst: Estado de Ejecucion, sirve para controlar la ejecucion
           del proceso, por ahora los valores asignados estan en la
           enumeracion xst_vals. */

        char        xst;                    /* Estado de Ejecucion */

        /* max_ms: Maximo numero de milisegundos para correr el proceso,
           max_ticks: Identico a max_ms pero en TICKS no en ms,
           ticks: Numero de TICKS que se han utilizado. */

        uchar       max_ms, max_ticks, ticks;

        /* Los valores de max_ticks y ticks son controlados y
           asignados por el controlador de procesos. */

        int         exitcode;               /* Codigo de Salida */

        state_s     state;                  /* Estado del Proceso */

        void        *vconsole;              /* Consola Virtual */

        int         flags;                  /* Indicadores del proceso. */

        int         (*sig_handler) (int);   /* Controlador de Se\xA4ales */

        char        *cmdtail;               /* Linea de Comandos */

        /* Entrada/Salida Estandar y Salida de Errores Estandar. */
        FILE        *stdin, *stdout, *stderr;

        /* Esta es la firma (32-bits) del proceso residente. */
        ulong       res_sign;

        /* Identificacion del Proceso (valor numerico) */
        uint        pid;

        struct      process_s *parent;      /* Proceso Padre */
        struct      process_s *next_thr;    /* Siguiente Hilo */

        struct      process_s *anterior;    /* Proceso Anterior */
        struct      process_s *siguiente;   /* Siguiente Proceso */
    }
    process_s;

    /* Class Code 0004 */
    enum CC_0004
    {
        F__FIND_PROCESS=0x00040000, /* (int pid) : process_s * */
        F__FFIND_PROCESS,           /* (int pid) : process_s * */
        F__GET_RESIDENT,            /* (char *, int sign) : process_s * */
        F__NEW_CONSOLE,             /* (process_s *) : int */
        F__RELEASE_PARENT,          /* (process_s *) : void */
        F__SENG_SIG,                /* (process_s *, int) : int */
        F__FSPAWNV,                 /* (FILE *, int, char *, char *, int *) : int */
        F__KILL_PROCESS,            /* (process_s *) : int */
        F__FIND_PROCESS_ID,         /* (char *) : process_s * */
        F__EXIT,                    /* (int) : void */
        F__LOCK,                    /* (void) : void */
        F__UNLOCK,                  /* (void) : void */
    };

    extern process_s *selfProcess;

#endif
