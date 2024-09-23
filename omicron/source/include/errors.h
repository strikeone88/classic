/*
    ERRORS.H

    Lista de errores, cuando ha habido un error algunas funciones
    retornan un numero negativo, el cual, cuando se vuelve positivo
    describe un error que esta en esta lista.

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __ERRORS
#define __ERRORS

    enum errores
    {
        ERROR_GENERAL   = 0x01,
        E_LECTURA,
        E_ESCRITURA,
        NO_SIG_HANDLER,
        NO_MEM,
        NO_ENCONTRADO,
        INV_EXE,
        PUNTERO_NULO,
        NO_SOPORTADO,
        E_DENIED,
    };

#endif
