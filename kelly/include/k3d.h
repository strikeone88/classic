/*
    K3D.H

    Kelly 3D Graphics Engine Version 0.07 - Part of KGfx

    Copyright (C) 2008-2009 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#define DEBUG 1

#ifndef __K3D_H
#define __K3D_H

    /* Bits that identify each internal buffer. */
    #define COLOR_BUFFER        0x01
    #define DEPTH_BUFFER        0x02
    #define ACCUM_BUFFER        0x04

    /* Allowed matrix modes. */
    #define MODELVIEW_MATRIX    0x00
    #define PROJECTION_MATRIX   0x01

    /* Object types. */
    #define POINT_OBJECT        0x00
    #define LINE_OBJECT         0x01
    #define POLYLINE_OBJECT     0x02
    #define TRI_OBJECT          0x03
    #define QUAD_OBJECT         0x04
    #define MESH_OBJECT         0x05
    #define COMPOSITE_OBJECT    0x06
    #define MAX_OBJECT_TYPE     COMPOSITE_OBJECT

    /* Bit of each axis. */
    #define X_AXIS              0x01
    #define Y_AXIS              0x02
    #define Z_AXIS              0x04

    /* Allocates siz bytes of memory and returns a pointer or NULL. */
    #define gAlloc(siz) malloc (siz)

    /* Allocates a block of memory of siz bytes, and clears it. */
    #define gAllocC(siz) calloc (siz, 1)

    /* Releases a previously allocated block of memory. */
    #define gFree(blk) free (blk)

    /* Macro for private functions and variables. */
    #define private static

    #ifndef __KGFX_H
    #include <kgfx.h>
    #endif

    /* K3D Data Types. */
    typedef gFloat gMatrix [4][4];

    /* Vertex structure (location and color). */
    typedef struct _gVertexLC
    {
        gFloat x, y, z, w;
        gDword Color;

        struct _gVertexLC *Next;
    }
    gVertexLC;

    /* Vertex structure (location, color and texture coordinates). */
    typedef struct _gVertexLCT
    {
        gFloat x, y, z, w;
        gDword Color;

        gFloat u, v;

        struct _gVertexLCT *Next;
    }
    gVertexLCT;

    /* Object structure. */
    typedef struct _gObject
    {
        struct      _gObject *Next;
        gInt        Type;
    }
    gObject;

    /* Point object structure. */
    typedef struct
    {
        gObject     Object;
        gVertexLC   *Top, *Bottom;

        gDword      Color;
    }
    gPointObject;

    /* Line object structure. */
    typedef struct
    {
        gObject     Object;
        gVertexLC   *Top, *Bottom;

        gDword      Color;
        gInt        Vertices;
    }
    gLineObject;

    /* Tri object structure. */
    typedef struct
    {
        gObject     Object;
        gVertexLCT  *Top, *Bottom;

        gDword      Color;
        gInt        Vertices;
    }
    gTriObject;

/*#*/
    /* Initializes the KGfx and K3D engines. */
    grBool gStart (void);

    /* De-initializes KGfx and K3D. */
    void gStop (void);

/*#*/
    /* Sets the viewport given its position and dimension. */
    void gViewport (gFloat x, gFloat y, gFloat w, gFloat h);

    /* Copies the color buffer into the video memory. */
    void gFlip (void);

    /* Clears the internal buffers selected by the bit mask. */
    void gClear (gByte BitMask);

    /* Sets the color used to clear the color buffer. */
    void gClearColor (gDword RGBA);

/*#*/
    /* Changes the mode of the matrix operation module. */
    void gMatrixMode (gByte MatrixMode);

    /* Pushes the current matrix in the stack. */
    gBool gPushMatrix (void);

    /* Pops a matrix from the stack and uses it as the current matrix. */
    gBool gPopMatrix (void);

    /* Returns a pointer to the ith matrix of the stack. */
    gMatrix *gGetMatrix (gInt i);

    /* Loads the identity in the current matrix. */
    void gLoadIdentity (void);

    /* Loads the given matrix into the current matrix. */
    void gLoadMatrixv (gFloat *Ptr);

    /* Loads the given matrix values into the current matrix. */
    void gLoadMatrixf (gFloat First, ...);

    /* Multiplies the current matrix by the given matrix. */
    void gMulMatrixv (gFloat *Ptr);

    /* Multiplies the current matrix by the given matrix. */
    void gMulMatrixf (gFloat First, ...);

    /* Scales by the given factors respectively for each axis. */
    void gScalef (gFloat x, gFloat y, gFloat z);

    /* Performs translation to the given coordinates. */
    void gTranslatef (gFloat x, gFloat y, gFloat z);

    /* Rotates the given axises. */
    void gRotateAxis (gFloat rads, gInt Axis);

    /* Rotates through the given axis. */
    void gRotatef (gFloat rads, gFloat x, gFloat y, gFloat z);

    /* Multiplies the current matrix by a parallel projection matrix. */
    void gOrtho (gFloat Left, gFloat Right, gFloat Top, gFloat Bottom, gFloat Near, gFloat Far);

    /* Multiplies the current matrix by a perspective proj matrix. */
    void gPerspective (gFloat Fov, gFloat Aspect, gFloat Near, gFloat Far);

/*#*/
    /* Begins the definition of an object of the given type. */
    grBool gBeginObjectDef (gEnum Type);

    /* Returns a pointer to the object being defined. */
    gObject *gDefiningObject (void);

    /* Ends the definition of the current object, returns its pointer. */
    gObject *gEndObjectDef (void);

    /* Destroys a previosly created object. */
    void gDestroyObject (gObject *Ptr);

    /* Cancels the definition of the current object. */
    void gCancelObjectDef (void);

    /* Renders the given object. */
    void gRenderObject (gObject *Ptr);

/*#*/
    /* Adds the vertex (x, y, 0, 1) to the current object. */
    void gVertex2f (gFloat x, gFloat y);

    /* Adds the vertex (x, y, 0, 1) to the current object. */
    void gVertex2v (gFloat *Ptr);

    /* Adds the vertex (x, y, z, 1) to the current object. */
    void gVertex3f (gFloat x, gFloat y, gFloat z);

    /* Adds the vertex (x, y, z, 1) to the current object. */
    void gVertex3v (gFloat *Ptr);

/*#*/
    /* Sets the color for the following vertices. */
    void gVertexColor (gDword RGBA);

#endif
