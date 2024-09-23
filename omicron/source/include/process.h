/*
    PROCESS.H

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __PROCESS
#define __PROCESS

    #ifndef __TYPES
    #include <types.h>
    #endif

    #ifndef __CONSOLE
    #include <console.h>
    #endif

    #ifndef __MM
    #include <mm.h>
    #endif

    #ifndef __FILE
    #include <file.h>
    #endif

    /* Valor predeterminado para las EFLAGS de un proceso. */
    #define DEF_EFLAGS  0x3202

    /* Los modos de ejecucion que se pasan a spawn. */
    #define P_NOWAIT        0x00    /* Bit 0 */
    #define P_WAIT          0x01
    #define P_LOADONLY      0x02    /* Bit 1 */

    /* El total de bytes en la pila automatica. */
    #define AUTO_STACK      4096

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

        vconsole_s  *vconsole;              /* Consola Virtual */

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

    /* El kernel es el primer proceso en la lista de multitareas. */
    extern process_s *kprocess;

    /* El ultimo proceso en la lista de multitareas. */
    extern process_s *lprocess;

    /* El proceso que se esta corriendo actualmente. */
    extern process_s *process;

    /* El proceso PRIMARIO y el proceso en espera corta. */
    extern process_s *p_process, *w_process;

    /* Este es el primer proceso residente, es una lista LIFO. */
    extern process_s *resident;

    /* Retorna TRUE si es proceso primario. */
    int es_proceso_primario (process_s *p);

    /* Retorna un puntero a el descriptor del proceso dado el indice. */
    process_s *find_process (uint pid);

    /* Igual a find_process pero tambien busca en la lista de procesos
       residentes. */

    process_s *ffind_process (uint pid);

    /* Retorna el puntero al proceso residente que tenga la misma id
       y la misma firma. */

    process_s *get_resident (char *id, ulong);

    /* En enfoca en el proceso dado. */
    void focus_process (process_s *);

    /* Crea una consola virtual para el proceso dado. */
    int new_vconsole (process_s *);

    /* Libera al proceso padre. */
    void release_parent (process_s *);

    /* Deslinkea (desenlaza ;) un proceso de la lista de multitareas. */
    void unlink_process (process_s *p);

    /* Crea un nuevo proceso y retorna un puntero al descriptor. */
    process_s *new_process (void);

    /* Enlaza el proceso dado a la lista de procesos a ejecutar. */
    void link_process (process_s *);

    /* Envia una se\xA4al al proceso indicado. */
    int send_sig (process_s *, int);

    /* Crea y opcionalmente corre un proceso hijo. */
    int fspawnv (FILE *fp, int mode, char *name, char *cmdtail, ulong *xlen);

    /* Le dice al kernel que vamos a entrar en un area critica donde
       NO se puede hacer multitasking, sirve para evitar que otros
       procesos corran cuando se entra a un area critica. */

    void enter_critical (void);

    /* Sale de un area critica, es decir, re-habilita el multitasking. */
    void leave_critical (void);

    /* Finaliza un proceso. */
    int kill_process (process_s *);

    /* Retorna el numero de areas criticas a las que se ha entrado
       recursivamente. */

    int critical (void);

    extern FILE *kstd_in, *kstd_out, *kstd_err;

    extern int lock;

    /*
      @Autor: Juan H. Rodas (juanrodas85@gmail.com)
      @Fecha: 16/07/08
    */
    
    /*busca un proceso por medio del atributo ID.*/
     process_s *find_process_id(const char*);

#endif
