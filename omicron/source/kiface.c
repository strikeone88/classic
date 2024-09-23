/*
    KIFACE.C

    Interfaz del Kernel

    Escrito por J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <console.h>
    #include <process.h>
    #include <kiface.h>
    #include <portio.h>
    #include <keyb.h>
    #include <time.h>
    #include <timer.h>
    #include <mm.h>

    void *Kernel_Interface (int Code, ...)
    {
        va_list args = get_valist (Code);
        void *p, *q;

            switch (Code)
            {
/****************************************************************************/
/*** Class Code 0000 ********************************************************/
/****************************************************************************/

                case F__CLRSCR:
                    clrscr ();
                    break;

                case F__C_CLRSCR:
                    c_clrscr (va_arg (args, int));
                    break;

                case F__SETCOLOR:
                    setcolor (va_arg (args, int));
                    break;

                case F__SETBGCOLOR:
                    setbgcolor (va_arg (args, int));
                    break;

                case F__SETCURSOR:
                    setcursor (va_argi (args, int, 0), va_argi (args, int, 1));
                    break;

                case F__GOTOXY:
                    gotoxy (va_argi (args, int, 0), va_argi (args, int, 1));
                    break;

                case F__WHEREX:
                    returnv (wherex ());

                case F__WHEREY:
                    returnv (wherey ());

                case F__KBHIT:
                    returnv (kbhit ());

                case F__GETCH:
                    returnv (getch ());

/****************************************************************************/
/*** Class Code 0001 ********************************************************/
/****************************************************************************/

                case F__CLONEDESC:
                    returnv (clonar_descriptor (va_arg (args, FILE *)));

                case F__GET_DEV:
                    returnv (get_dev (va_arg (args, char *)));

                case F__REGISTER_DEV:
                    returnv (register_dev (va_arg (args, dev_s *)));

                case F__UNREGISTER_DEV:
                    returnv (unregister_dev (va_arg (args, dev_s *)));

                case F__DEVOPEN:
                    returnv (devopen (va_argi (args, char *, 0), va_argi (args, int, 1)));

                case F__FREAD:
                    returnv (fread (va_argi (args, FILE *, 0), va_argi (args, size_t, 1), va_argi (args, void *, 2)));

                case F__FWRITE:
                    returnv (fwrite (va_argi (args, FILE *, 0), va_argi (args, size_t, 1), va_argi (args, void *, 2)));

                case F__FSEEK:
                    returnv (fseek (va_argi (args, FILE *, 0), va_argi (args, long, 1), va_argi (args, int, 2)));

                case F__FCLOSE:
                    returnv (fclose (va_argi (args, FILE *, 0)));

                case F__FPUTC:
                    returnv (fputc (va_argi (args, FILE *, 0), va_argi (args, int, 1)));

                case F__FGETC:
                    returnv (fgetc (va_argi (args, FILE *, 0)));

                case F__FGETW:
                    returnv (fgetw (va_argi (args, FILE *, 0)));

                case F__FGETD:
                    returnv (fgetd (va_argi (args, FILE *, 0)));

                case F__FPUTS:
                    returnv (fputs (va_argi (args, FILE *, 0), va_argi (args, char *, 1)));

                case F__VPRINTF:
                    returnv (vprintf (va_argi (args, void *, 0), va_argi (args, void *, 1)));

                case F__VFPRINTF:
                    returnv (vfprintf (va_argi (args, void *, 0), va_argi (args, void *, 1), va_argi (args, void *, 2)));

/****************************************************************************/
/*** Class Code 0002 ********************************************************/
/****************************************************************************/

                case F__MALLOC:
                    returnv (kmalloc (va_arg (args, unsigned)));

                case F__CALLOC:
                    returnv (kcalloc (va_arg (args, unsigned)));

                case F__FREE:
                    kfree (va_arg (args, void *));
                    break;

                case F__CORELEFT:
                    returnv (kcoreleft ());

                case F__CHBLKOWN:
                    chblkown (va_argi (args, void *, 0), va_argi (args, process_s *, 1));
                    break;

                case F__PSMEM:
                    returnv (kmemp_used (va_arg (args, process_s *)));

                case F__FREEPSMEM:
                    free_process_memory (va_arg (args, process_s *));
                    break;

                case F__DUMPMEMNODES:
                    dump_mem_nodes (va_argi (args, FILE *, 0), va_argi (args, uint, 1));
                    break;

/****************************************************************************/
/*** Class Code 0003 ********************************************************/
/****************************************************************************/

                case F__OUTPORTB:
                    outportb (va_argi (args, int, 0), va_argi (args, int, 1));
                    break;

                case F__OUTPORTW:
                    outportw (va_argi (args, int, 0), va_argi (args, int, 1));
                    break;

                case F__OUTPORTD:
                    outportd (va_argi (args, int, 0), va_argi (args, int, 1));
                    break;

                case F__INPORTB:
                    returnv (inportb (va_arg (args, int)));

                case F__INPORTW:
                    returnv (inportw (va_arg (args, int)));

                case F__INPORTD:
                    returnv (inportd (va_arg (args, int)));

                case F__SETVECT:
                    setvect (va_argi (args, int, 0), va_argi (args, void *, 1), va_argi (args, int, 2));
                    break;

                case F__GETVECT:
                    returnv (getvect (va_arg (args, int)));

                case F__GTIME:
                    returnv (gtime ());

                case F__PACKTIME:
                    returnv (packTime (va_arg (args, tm *)));

                case F__UNPACKTIME:
                    returnv (unpackTime (va_argi (args, int, 0), va_argi (args, tm *, 1)));

                case F__V86INT:
                    v86int (va_argi (args, int, 0), va_argi (args, void *, 1));
                    break;

                case F__CLOCK:
                    returnv (tick_counter);

/****************************************************************************/
/*** Class Code 0004 ********************************************************/
/****************************************************************************/

                case F__FIND_PROCESS:
                    returnv (find_process (va_arg (args, int)));

                case F__FFIND_PROCESS:
                    returnv (ffind_process (va_arg (args, int)));

                case F__GET_RESIDENT:
                    returnv (get_resident (va_argi (args, char *, 0), va_argi (args, int, 1)));

                case F__NEW_CONSOLE:
                    returnv (new_vconsole (va_arg (args, process_s *)));

                case F__RELEASE_PARENT:
                    release_parent (va_arg (args, process_s *));
                    break;

                case F__SENG_SIG:
                    returnv (send_sig (va_argi (args, process_s *, 0), va_argi (args, int, 1)));

                case F__FSPAWNV:
                    returnv (fspawnv (
                        va_argi (args, FILE *, 0),
                        va_argi (args, int, 1),
                        va_argi (args, char *, 2),
                        va_argi (args, char *, 3),
                        va_argi (args, int *, 4)
                    ));

                case F__KILL_PROCESS:
                    returnv (kill_process (va_arg (args, process_s *)));

                case F__FIND_PROCESS_ID:
                    returnv (find_process_id (va_arg (args, char *)));

                case F__EXIT:
                    process->exitcode = va_arg (args, int);
                    kill_process (process);
                    break;

                case F__LOCK:
                    lock = 1;
                    break;

                case F__UNLOCK:
                    lock = 0;
                    break;

/****************************************************************************/
/*** Class Code 0005 ********************************************************/
/****************************************************************************/

                case F__REGIFACE:
                    returnv (registerInterface (va_argi (args, char *, 0), va_argi (args, void *, 1)));

                case F__UNREGIFACE:
                    returnv (unregisterInterface (va_arg (args, char *)));

                case F__GETSIFACE:
                    returnv (getServiceInterface (va_arg (args, char *)));
            }

            return NULL;
    }
