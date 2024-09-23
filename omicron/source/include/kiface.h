/*
    KIFACE.H

    Interfaz del Kernel

    Escrito por J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __KIFACE_H
#define __KIFACE_H

    /* Special macro to return values from a service. */
    #define returnv(x) return (void *)(x)

    /* Class Code 0000 */
    enum CC_0000
    {
        F__CLRSCR = 0x00000000,     /* (void) : void */
        F__C_CLRSCR,                /* (int color) : void */
        F__SETCOLOR,                /* (int color) : void */
        F__SETBGCOLOR,              /* (int color) : void */
        F__SETCURSOR,               /* (int start, int end) : void */
        F__GOTOXY,                  /* (int x, int y) : void */
        F__WHEREX,                  /* (void) : int */
        F__WHEREY,                  /* (void) : int */
        F__KBHIT,                   /* (void) : int */
        F__GETCH,                   /* (void) : int */
    };

    /* Class Code 0001 */
    enum CC_0001
    {
        F__CLONEDESC = 0x00010000,  /* (FILE *) : FILE * */
        F__GET_DEV,                 /* (char *) : dev_s * */
        F__REGISTER_DEV,            /* (dev_s *) : int */
        F__UNREGISTER_DEV,          /* (dev_s *) : int */
        F__DEVOPEN,                 /* (char *, int flags) : FILE * */
        F__FREAD,                   /* (FILE *, int, void *) : int */
        F__FWRITE,                  /* (FILE *, int, void *) : int */
        F__FSEEK,                   /* (FILE *, int, int) : int */
        F__FCLOSE,                  /* (FILE *) : int */
        F__FPUTC,                   /* (FILE *, int) : int */
        F__FGETC,                   /* (FILE *) : int */
        F__FGETW,                   /* (FILE *) : int */
        F__FGETD,                   /* (FILE *) : int */
        F__FPUTS,                   /* (FILE *) : int */
        F__VPRINTF,                 /* (char *, void *) : int */
        F__VFPRINTF,                /* (FILE *, char *, void *) : int */
    };

    /* Class Code 0002 */
    enum CC_0002
    {
        F__MALLOC = 0x00020000,     /* (unsigned) : void * */
        F__CALLOC,                  /* (unsigned) : void * */
        F__FREE,                    /* (void *) : void */
        F__CORELEFT,                /* (void) : int */
        F__CHBLKOWN,                /* (void *, process_s *) : void */
        F__PSMEM,                   /* (process_s *) : int */
        F__FREEPSMEM,               /* (process_s *) : void */
        F__DUMPMEMNODES,            /* (FILE *, int) : void */
    };

    /* Class Code 0003 */
    enum CC_0003
    {
        F__OUTPORTB = 0x00030000,   /* (int, int) : void */
        F__OUTPORTW,                /* (int, int) : void */
        F__OUTPORTD,                /* (int, int) : void */
        F__INPORTB,                 /* (int) : int */
        F__INPORTW,                 /* (int) : int */
        F__INPORTD,                 /* (int) : int */
        F__SETVECT,                 /* (int, void *) : void */
        F__GETVECT,                 /* (int) : void * */
        F__GTIME,                   /* (void) : time_t * */
        F__PACKTIME,                /* (tm *) : time_t * */
        F__UNPACKTIME,              /* (time_t, tm *) : tm * */
        F__V86INT,                  /* (int, REGPACK *) : void */
        F__CLOCK,                   /* (void) : time_t */
    };

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

    /* Class Code 0005 */
    enum CC_0005
    {
        F__REGIFACE = 0x00050000,   /* (char *name, void *func) : int */
        F__UNREGIFACE,              /* (char *name) : int */
        F__GETSIFACE,               /* (char *name) : void * */
    };

    void *Kernel_Interface (int Code, ...);

#endif
