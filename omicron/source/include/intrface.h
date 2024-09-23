/*
    INTRFACE.H

    Manejador de Interfaces

    Escrito por J. Palencia (ilina@bloodykisses.zzn.com)
    Escrito por J. Rodas (juanrodas85@gmail.com)
*/

#ifndef __INTRFACE_H
#define __INTRFACE_H

    #ifndef __PROCESS_H
    #include <process.h>
    #endif

    typedef struct intrfaceInfo_s
    {
        void        *(*intrface) (int, ...);

        const char  *id;
        uint        pid;

        struct      intrfaceInfo_s *prev;
        struct      intrfaceInfo_s *next;
    }
    intrfaceInfo_s;

    typedef void *(*ServiceInterface) (int Code, ...);

    /**/
    int registerInterface (char *name, void *iface);

    /**/
    int unregisterInterface (char *name);

    /**/
    void *getServiceInterface (char *name);

#endif
