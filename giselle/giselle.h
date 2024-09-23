/*
    GISELLE.H

    Giselle Code Generation Engine Version 0.01

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __GISELLE_H
#define __GISELLE_H

    #ifndef __GUTILS_H
    #include <gutils.h>
    #endif

    #ifndef __GLINK_H
    #include <glink.h>
    #endif

    /* Values returned by a GCore to indicate NO-ERROR or ERROR. */
    #define GCError ((void *)(1))
    #define GCOk    ((void *)(0))

    /* Symbol Flags. */
    typedef enum
    {
        GSYPrivate, GSYPublic, GSYExternal, /* 0-2 */
    }
    GSYFlags;

    /* Section flags. */
    typedef enum
    {
        GSByte = 0x00, GSWord = 0x01, GSDword = 0x02, GSPara = 0x03,
        GSPage256 = 0x04, GSPage = 0x05, /* 0-2 */
        GDByte = 0x00, GDWord = 0x08, GDDword = 0x10, GDPara = 0x18,
        GDPage256 = 0x20, GDPage = 0x28, /* 3-5 */
        GSPublic = 0x00, GSPrivate = 0x40, /* 6 */
        GSExport = 0x80, GSImport = 0x100, /* 7-8 */
    }
    GSFlags;

    /* Data type (for both data and code items). */
    typedef enum
    {
        GDHollow, GDConstant, GDSectionBase, GDSectionLength,
        GDSymbolOffset, GDCode
    }
    GDType;

    /* Code generation core commands. */
    typedef enum
    {
        GCIdentifier, GCEnableCap, GCDisableCap, GCCreateContext,
        GCDestroyContext, GCOutputFile, GCBegin
    }
    GCommand;

    /* Code Generation Core Interface. */
    typedef void *GCore (GCommand, ...);

    /* Section Information Structure. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;

        list_t      *Symbols;

        unsigned    char flags;
        unsigned    index;

        unsigned    long offset;

        char        *class;
        char        *name;

        char        *gname;
        char        *bin;
    }
    GSection;

    /* Symbol Information Structure. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t      link;

        GSection        *Section;

        unsigned long   offset;
        unsigned char   flags;

        unsigned long   index;
        char            *name;
    }
    GSymbol;

    /* Giselle Context Structure. */
    typedef struct
    {
        list_t      *Sections;

        GSection    *Selected;
        FILE        *Output;

        char        *CoreId;
        void        *CoreCt;

        GCore       *Core;

        unsigned    long SymbolC;
    }
    GContext;

/********/

    /* Creates a context structure. */
    GContext *GCreate (void);

    /* Destroys the context structure. */
    void GDestroy (GContext *);

    /* Adds a section node to the list. */
    GSection *GASection (GContext *, char *, char *, GSFlags);

    /* Exports the section using the given global name. */
    GSection *GExpSection (GSection *S, char *gname);

    /* Imports the section using the given global name. */
    GSection *GImpSection (GSection *S, char *gname);

    /* Removes a section node from the list. */
    void GRSection (GContext *, GSection *);

    /* Searches for a section and returns its node. */
    GSection *GSSection (GContext *, char *);

    /* Searches for a section given its index. */
    GSection *GSISection (GContext *Ct, unsigned index);

    /* Adds a symbol node to the list. */
    GSymbol *GASymbol (GContext *, char *, GSection *, GSYFlags);

    /* Searches for a Symbol and returns its node. */
    GSymbol *GSSymbol (GContext *, char *);

    /* Searches for a Symbol given its index. */
    GSymbol *GSISymbol (GContext *, unsigned long);

    /* Updates a symbol offset given its name. */
    GSymbol *GUSymbol (GContext *Ct, char *name);

    /* Updates a symbol offset given its name. */
    GSymbol *GUISymbol (GContext *Ct, unsigned long index);

    /* Opens and selects the section for output. */
    int GOpenOutput (GContext *, GSection *);

    /* Closes the currently selected section. */
    void GCloseOutput (GContext *);

    /* Writes a data item to the selected section. */
    int GDataItem (GContext *, GDType, unsigned long, void *, unsigned);

    /* Writes a hollow data item to the selected section. */
    int GHollowDataItem (GContext *, unsigned long);

    /* Selects the target machine code generation core. */
    int GSelectCore (GContext *, GCore *);

    /* Enables a capability of the Core. */
    int GEnableCap (GContext *, unsigned);

    /* Disables a capability of the Core. */
    int GDisableCap (GContext *, unsigned);

    /* Indicates that the generation must be finished. */
    int GFinish (GContext *);

#endif
