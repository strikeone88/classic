/*
    IFM.H
*/

#ifndef __IFM_H
#define __IFM_H

    #ifndef __BASICS_H
    #include <basics.h>
    #endif

    #ifndef __STDARG_H
    #include <stdarg.h>
    #endif

    /* Special macro to return values from a service. */
    #define returnv(x) return (void *)(x)

    /**/
    #define registerInterface(x,y) (int)KernelInterface (F__REGIFACE, (char *)(x), (void *)(y))

    /**/
    #define unregisterInterface(x) (int)KernelInterface (F__UNREGIFACE, (char *)(x))

    /**/
    #define getServiceInterface(x) (ServiceInterface)KernelInterface (F__GETSIFACE, (char *)(x))

    /* Class Code 0005 */
    enum CC_0005
    {
        F__REGIFACE = 0x00050000,   /* (char *name, void *func) : int */
        F__UNREGIFACE,              /* (char *name) : int */
        F__GETSIFACE,               /* (char *name) : void * */
    };

#endif
