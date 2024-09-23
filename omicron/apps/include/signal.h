/*
    PROCESS.H
*/

#ifndef __PROCESS_H
#define __PROCESS_H

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

#endif
