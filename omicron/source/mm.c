/*
    MM.C

    Gestor de Memoria

    Este modulo es uno de las mas importantes, este es el que controla
    la gestion de memoria. Note que cuando el procesador esta en modo
    real (16-bits) solo se tiene acceso a 1 MB de memoria sin importar
    cuanto mas se tenga, de ese mega, 384 KB son del BIOS, asi que en
    realidad solo tenemos 640 KB, pero ya he 64 KB son HMA y no se va
    a utilizar nos quedan exactamente 576 KB libres.

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <vfprintf.h>
    #include <string.h>
    #include <mm.h>

    /* El primer y ultimo nodo de bloque de memoria. */
    static memnode_s *primero = NULL, *ultimo = NULL;

    /* El primer nodo de la lista de nodos muertos. */
    static memnode_s *dead_list = NULL;

    void *FIN_MEMORIA;

    /* Inicializa el manejador de memoria. */
    void *start_mm (size_t totalMem)
    {
            FIN_MEMORIA = (void *)((char *)totalMem - 1);
    }

    /* Intenta localizar un bloque de memoria que contenga el numero de
       bytes deseados, retorna un puntero a el bloque de memoria o NULL
       si hubo algun error o no hay memoria suficiente. */

    void *kmalloc (count_t bytes)
    {
        count_t temp, paras = align (bytes + sizeof (memnode_s), 16) >> 4;
        memnode_s *p, *q;
        offset_t offs;

            /* Hay que verificar que el numero de bytes no sea cero. */
            if (!bytes) return NULL;

            /* Si todavia no hay bloques localizados, entonces el que
               vamos a localizar va a ser el primero. */

            if (primero == NULL || ultimo == NULL)
            {
                primero = p = INICIO_MEMORIA;
                p->ant_paras = 0;

            actualizar:;
                ultimo = p;

            retornar:;
                p->magic = VALOR_MAGICO_V;
                p->paras = paras;

                p->process_id = process == NULL ? 0 : process->pid;

                return (void *)((char *)p + sizeof (memnode_s));
            }

            /* Ahora vamos a ver si uno de los nodos que fueron liberados
               usando kfree (nodos muertos) nos sirve para re-utilizarlo. */

            for (p = dead_list; p != NULL; p = (q = p->_sig) ==
                                           dead_list ? NULL : q)
            {
                /* Primero hay que verificar que la lista de nodos muertos
                   este perfecta, para hacer eso hay que ver que todos los
                   nodos muertos tengan el valor magico VALOR_MAGICO_M. */

                if (p->magic != VALOR_MAGICO_M)
                {
                    /* Si no es asi entonces hay que invalidar toda la
                       lista de nodos muertos para evitar futuros errores
                       de localizacion de memoria. */

                    dead_list = NULL;
                    break;
                }

                /* Si el numero de parrafos contenidos por el bloque de
                   este nodo muerto es menor que el numero de parrafos
                   necesarios entonces, veamos el siguiente. */

                if (p->paras < paras) continue;

                /* Si el nodo muerto tiene el mismo numero de parrafos
                   que necesitamos entonces tenemos un ENCAJE PERFECTO. */

                if (p->paras == paras)
                { reavivar:;
                    /* Para reaviviar el nodo solo hay que des-enlazarlo
                       de la lista de nodos muertos y ponerle el valor
                       magico VALOR_MAGICO_V. */

                    if (p->_ant != p)
                    {
                        p->_sig->_ant = p->_ant;
                        p->_ant->_sig = p->_sig;

                        if (p == dead_list) dead_list = p->_sig;
                    }
                    else
                        dead_list = NULL;

                    goto retornar;
                }

                /* Si llegamos a este punto entonces eso quiere decir que
                   el nodo muerto tiene mas parrafos de lo que necesitamos,
                   por lo tanto ahora vamos a ver si se puede quebrar el
                   nodo en dos partes.
                */

                if (p->paras - paras < 2)
                {
                    /* Si no se puede quebrar el nodo, entonces solo hay
                       que tomarlo, para eso hay que reavivarlo. */

                    paras = p->paras;
                    goto reavivar;
                }
                else
                {
                    /* Para quebrar el nodo lo que vamos a hacer es reducir
                       el total de parrafos y asi dejar un espacio libre
                       abajo del nodo, alli es donde vamos a localizar el
                       nuevo nodo.
                    */

                    temp = (p->paras -= paras);

                    /* El nuevo nodo empieza luego del nodo reducido p. */
                    p = (memnode_s *)(((char *)p) + (temp << 4));
                    p->ant_paras = temp;

                    /* Actualizar el nodo siguiente a p. */
                    q = (memnode_s *)(((char *)p) + (paras << 4));
                    q->ant_paras = paras;

                    goto retornar;
                }
            }

            /* Si llegamos a este punto esto quiere decir que: (1) El nodo
               que vamos a localizar no es el primero, (2) No hay lista de
               nodos muertos o no hay un nodo muerto que sea reutilizable,
               (3) La lista de nodos muertos ha sido invalidada.

               Lo unico que hay que hacer ahora es localizar el nuevo nodo
               inmediatamente despues del ultimo.
            */

            temp = ultimo->paras;

            p = (memnode_s *)(((char *)ultimo) + (temp << 4));

            /* Verificar que haya suficiente memoria libre. */
            if ((((char *)p) + (paras << 4)) > FIN_MEMORIA)
                return NULL;

            p->ant_paras = temp;

            goto actualizar;
    }

    /* Funcionalmente identico a "kmalloc" pero nulifica el bloque. */
    void *kcalloc (count_t x)
    {
        void *p = kmalloc (x);

            /* Si kmalloc fallo, hay que hacercelo saber al que llamo. */
            if (p == NULL) return NULL;

            /* Hora de limpiar el bloque. */
            memset (p, 0, x);

            /* Retornamos el puntero al bloque de memoria NULIFICADO. */
            return p;
    }

    /* Retorna un puntero al nodo anterior a p. */
    static memnode_s *nodo_ant (memnode_s *p)
    {
            if (p->ant_paras)
                return (memnode_s *)(((char *)p) - (p->ant_paras << 4));
            else
                return NULL;
    }

    /* Retorna un puntero al nodo que sigue despues de p. */
    static memnode_s *nodo_sig (memnode_s *p)
    {
            if (p != ultimo)
                return (memnode_s *)(((char *)p) + (p->paras << 4));
            else
                return NULL;
    }

    /* Libera un bloque de memoria que fue previamente localizado. */
    void kfree (void *p)
    {
        memnode_s *r, *q;

            /* No vamos a liberar bloques nulos. */
            if (p == NULL) return;

            /* Hay que recordar que kmalloc retorna un puntero a el
               bloque localizado NO al nodo, para obtener el puntero
               al nodo solo hay que restarle al puntero el total de
               bytes que tiene el nodo (16).
            */

            q = (memnode_s *)(((char *)p) - sizeof (memnode_s));

            /* Si el nodo no tiene el valor magico de un nodo VIVO
               entonces quiere decir que: (1) Esta muerto o, (2)
               No es un nodo valido. */

            if (q->magic != VALOR_MAGICO_V) return;

            /* Bueno, ahora hay que poner el nuevo valor magico. */
            q->magic = VALOR_MAGICO_M;

            /* Y por ultimo hay que agregar el nodo que acaba de
               morir a la lista de zombies, hehehe. */

            if (dead_list == NULL)
                q->_sig = q->_ant = dead_list = q;
            else
            {
                q->_sig = dead_list->_sig;
                q->_ant = dead_list;

                dead_list->_sig->_ant = q;
                dead_list->_sig = q;
            }

            /* Ahora vamos a combinar todos los nodos muertos que esten
               inmediatamente arriba de q. */

            while ((r = nodo_ant (q)) != NULL && r->magic == VALOR_MAGICO_M)
            {
                q->_sig->_ant = q->_ant;
                q->_ant->_sig = q->_sig;

                if (dead_list == q) dead_list = q->_ant;

                r->paras += q->paras;

                if (ultimo == q) ultimo = r;

                if ((q = nodo_sig (q)) != NULL)
                    q->ant_paras = r->paras;

                q = r;
            }

            /* El ultimo nodo NO puede estar muerto, si lo esta
               entonces hay que subir el ultimo a un nodo vivo. */

            if (ultimo->magic == VALOR_MAGICO_M)
            {
                /* Pero primero, hay que desenlazar al nodo muerto. */
                if (ultimo->_ant != ultimo)
                {
                   ultimo->_sig->_ant = ultimo->_ant;
                   ultimo->_ant->_sig = ultimo->_sig;

                   if (ultimo == dead_list) dead_list = ultimo->_sig;
                }
                else
                    dead_list = NULL;

                ultimo = nodo_ant (ultimo);
            }
    }

    /* Retorna el total de memoria libre. */
    size_t kcoreleft (void)
    {
        memnode_s *q = dead_list;
        size_t r;

            /* El total de memoria libre es: el total de memoria menos el
               total de memoria utilizada. */

            if (ultimo != NULL)
            {
                /* El total de memoria localizada es el segmento del ultimo
                   nodo menos el segmento del primer nodo, luego hay que
                   sumarle el total de parrafos en el ultimo nodo. */

                r = ultimo - INICIO_MEMORIA + (ultimo->paras << 4);

                /* Ya sabemos que r es el total de memoria localizada,
                   pero hay que restarle a r el total de parrafos de
                   cada nodo muerto, ya que son espacios libres. */

                if (q != NULL)
                {
                    do
                    {
                        r -= q->paras << 4;
                    }
                    while ((q = q->_sig) != dead_list);
                }
            }
            else
                r = 0;

            return (long)FIN_MEMORIA - (long)INICIO_MEMORIA + 1 - r;
    }

    /* Retorna el total de memoria libre en nodos muertos. */
    size_t kdcoreleft (void)
    {
        memnode_s *q = dead_list;
        size_t r = 0;

            if (q) do r += q->paras << 4;
            while ((q = q->_sig) != dead_list);

            return r;
    }

    /* Cambia el due\xA4o de un bloque de memoria. */
    void chblkown (void *p, struct process_s *x)
    {
        memnode_s *q;

            /* No se aceptan punteros nulos. */
            if (p == NULL || x == NULL) return;

            /* Convertir de bloque a nodo. */
            q = (memnode_s *)(((char *)p) - sizeof (memnode_s));

            /* Asegurarnos de que esta vivo. */
            if (q->magic != VALOR_MAGICO_V) return;

            /* Cambiar due\xA4o. */
            q->process_id = x->pid;
    }

    /* Retorna el total de memoria usada por un proceso. */
    size_t kmemp_used (struct process_s *p)
    {
        memnode_s *q = primero;
        size_t c = 0;

            while (q != NULL)
            {
                if (q->magic != VALOR_MAGICO_V &&
                    q->magic != VALOR_MAGICO_M) break;

                if (q->magic == VALOR_MAGICO_M) goto _continue;

                if (q->process_id == p->pid) c += q->paras;

            _continue:;
                q = nodo_sig (q);
            }

            return c * 16UL;
    }

    /* Libera todos los bloques de memoria de un proceso. */
    void free_process_memory (struct process_s *p)
    {
        memnode_s *q = primero;
        uint pid = p->pid;

            while (q != NULL)
            {
                if (q->magic != VALOR_MAGICO_V &&
                    q->magic != VALOR_MAGICO_M) break;

                if (q->magic == VALOR_MAGICO_M) goto _continue;

                if (q->process_id == pid)
                    kfree ((memnode_s *)(((char *)q) + sizeof (memnode_s)));

            _continue:;
                q = nodo_sig (q);
            }
    }

    /* Dumpea los nodos de control de memoria. */
    void dump_mem_nodes (FILE *fp, uint pid)
    {
        memnode_s *q = primero;
        size_t t = 0;
        int lives;

            while (q != NULL)
            {
                if (q->magic != VALOR_MAGICO_V &&
                    q->magic != VALOR_MAGICO_M) break;

                if (pid != ALL_PROCESSES) if (q->process_id != pid) goto _cont;

                lives = q->magic == VALOR_MAGICO_V;

                fprintf (fp, "\n%08lX %5Fs %6lu %02X %Fs",
                    q, lives ? "ALIVE" : "DEAD", q->paras * 16UL,
                    lives ? q->process_id : 0,
                    lives ? ffind_process (q->process_id)->id : "NULL");

                t += q->paras << 4;

                if (q == ultimo) fprintf (fp, " *");

            _cont:;
                q = nodo_sig (q);
            }

            printf ("\nTotal %lu bytes (%lu kb)", t, t >> 10);
    }
