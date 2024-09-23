/*
    MM.H

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __MM
#define __MM

    #ifndef __PROCESS
    #include <process.h>
    #endif

    #ifndef __FILE
    #include <file.h>
    #endif

    /* Direccion linear del inicio de la memoria libre. */
    #define INICIO_MEMORIA      (void *)(0x200000)

    /* El valor magico que TODOS los nodos VIVOS <DEBEN> tener. */
    #define VALOR_MAGICO_V      0x52A4

    /* El valor magico que los nodos MUERTOS <DEBEN> tener. */
    #define VALOR_MAGICO_M      0x53A5

    /* PID especial, indica TODOS LOS PROCESOS. */
    #define ALL_PROCESSES       (0xFFFEU)

    /* Formato de un nodo de bloque de memoria. */
    typedef struct memn_s
    {
        ushort      magic;          /* Valor para validar el bloque .*/
        count_t     paras;          /* Numero de parrafos en el bloque. */
        uint        process_id;     /* Proceso que creo el bloque. */
        count_t     ant_paras;      /* Parrafos en el bloque anterior. */
        struct      memn_s *_ant;   /* Nodo muerto anterior. */
        struct      memn_s *_sig;   /* Siguiente nodo muerto. */
    }
    memnode_s;

    /* Localiza un bloque de memoria. */
    void *kmalloc (lcount_t);

    /* Igualito a kmalloc pero nulifica el bloque. */
    void *kcalloc (count_t);

    /* Libera un bloque. */
    void kfree (void *);

    /* Retorna el total de memoria libre en el sistema. */
    size_t kcoreleft (void);

    /* Retorna el total de memoria libre en nodos muertos. */
    size_t kdcoreleft (void);

    /* Cambia el due\xA4o de un bloque de memoria. */
    void chblkown (void *, struct process_s *);

    /* Retorna el total de memoria utilizada por un proceso. */
    size_t kmemp_used (struct process_s *);

    /* Libera todos los bloques de memoria de un proceso. */
    void free_process_memory (struct process_s *p);

    /* Dumpea los nodos de control de memoria. */
    void dump_mem_nodes (FILE *, uint);

#endif
