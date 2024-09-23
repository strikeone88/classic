/*
    INTRFACE.C

    Manejador de Interfaces

    Escrito por J. Palencia (ilina@bloodykisses.zzn.com)
    Escrito por J. Rodas (juanrodas85@gmail.com)
*/

    #include <intrface.h>
    #include <string.h>
    #include <mm.h>

    /* El primer y ultimo nodo de informacion de interfaz. */
    static intrfaceInfo_s *first = NULL, *last = NULL;

    /* Retorna el nodo de information de servicio dada la id. */
    static intrfaceInfo_s *findInterface (const char *id)
    {
        intrfaceInfo_s *p;

            for (p = first; p; p = p->next)
                if (!strcmp (id, p->id)) return p;

            return NULL;
    }

    /**/
    int registerInterface (char *name, void *iface)
    {
        intrfaceInfo_s *q;
        int i;

            if (!name || !iface) return -PUNTERO_NULO;

            q = findInterface (name);
            if (q) return -ERROR_GENERAL;

            q = kmalloc (sizeof (intrfaceInfo_s));
            if (!q) return -NO_MEM;

            q->intrface = iface;

            q->id = kmalloc (i = strlen (name) + 1);
            if (!q->id)
            {
                kfree (q);
                return -NO_MEM;
            }

            memcpy (q->id, name, i);

            q->pid = process->pid;

            if (last)
                last->next = q;
            else
                first = q;

            q->prev = last;
            q->next = NULL;

            last = q;
            return 0;
    }

    /**/
    int unregisterInterface (char *name)
    {
        intrfaceInfo_s *q;

            if (!name) return -PUNTERO_NULO;

            q = findInterface (name);
            if (!q) return -NO_ENCONTRADO;

            if (q->pid != process->pid)
                return -E_DENIED;

            /*violet:check if being used!*/

            if (q->prev) q->prev->next = NULL; else first = q->next;
            if (q->next) q->next->prev = NULL; else last = q->prev;

            kfree (q->id);
            kfree (q);

            return 0;
    }

    /**/
    void *getServiceInterface (char *name)
    {
        intrfaceInfo_s *q;

            if (!name) return NULL;

            q = findInterface (name);
            if (!q) return NULL;

            return q->intrface;
    }
