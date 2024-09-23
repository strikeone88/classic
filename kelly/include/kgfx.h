/*
    KGFX.H

    Kelly Graphics Engine Version 0.07 (640x480x16bpp)

    Copyright (C) 2008-2009 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __KGFX_H
#define __KGFX_H

    /* Builds up an RGB0 color, assumes that each component is <= 255. */
    #define RGB(r,g,b) ((gDword)(r) << 16 | (gDword)(g) | (gDword)(b) << 8)

    /* Returns the offset of a pixel given its coordinates. */
    #define PixelOffset(x,y) ((gDword)(x) + (gDword)(y) * ScreenWidth)

    /* Screen resolution. */
    #define ScreenWidth 640
    #define ScreenHeight 480

    /* Data types. */
    typedef unsigned long gDword;
    typedef long gLong;

    typedef unsigned int gBool, grBool, gEnum;
    typedef int gInt;

    typedef unsigned short gWord;
    typedef short gShort;

    typedef unsigned char gByte;
    typedef char gChar;

    typedef float gFloat;

/*#*/
    /* Starts the graphics mode, returns non-zero if there was
       an error, or zero if no error ocurred. */

    gInt StartGfx (void);

    /* Stops the graphics mode (returns to text mode). */
    void StopGfx (void);

/*#*/
    /* Returns the length in bytes of the frame buffer, this function
       is used to determine the size of the buffer to allocate memory
       for it, returns the size plus 15 bytes to allow a forced
       alignment with AlignBuffer. */

    gInt FrameBufferSize (void);

    /* Returns a 16-byte aligned buffer pointer, returns the same
       pointer plus an offset between zero and fifteen to make it
       paragraph aligned. */

    void *AlignBuffer (void *Buff);

    /* Sets an internal constant that points to the frame
       buffer, which must be paragraph aligned. */

    void SetFrameBuffer (void *FrameBuffer);

/*#*/
    /* Puts a pixel on the frame buffer at the given offset,
       the color is given in RGBA 32-bit format. */

    void PutPixel (gDword Offset, gDword RGBA);

    /* Reads a pixel from the frame buffer and returns it,
       the value is a 32-bit RGBA color. */

    gDword GetPixel (gDword Offset);

/*#*/
    /* Copies the source frame buffer into dest, both must be
       paragraph aligned and of the same size. */

    void CopyFrameBuffer (void *Dest, void *Source);

    /* Copies the frame buffer into video memory. */
    void FlipFrameBuffer (void);

    /* Sets the color that will be used when the frame buffer is cleared. */
    void ClearFrameBufferValue (gDword RGBA);

    /* Clears the frame buffer, as fast as possible. */
    void ClearFrameBuffer (void);

/*#*/
    /* Draws a line from (x1,y1) to (x2,y2) with color c. */
    void DrawLine (int x1, int y1, int x2, int y2, int c);

/*#*/
    /* Copies the source vector into dest. */
    void CopyVector (gFloat *Dest, gFloat *Src);

    /* Normalizes the given vector, stores the result in Dst. */
    void NormalizeVector (gFloat *Dst, gFloat *Src);

    /* Copies the source 4x4 matrix into dest. */
    void CopyMatrix (gFloat *Dest, gFloat *Src);

    /* Multiplies the given vector by the 4x4 matrix and the
       resulting vector is stored in R. */

    void MulVectMatrix (gFloat *R, gFloat *V, gFloat *M);

    /* Copies the zero matrix into M1. */
    void CopyZeroMatrix (gFloat *M1);

    /* Copies the identity matrix into M1. */
    void CopyIdentityMatrix (gFloat *M1);

    /* Copies a 3x3 identity matrix into M1 (which is 4x4). */
    void CopyIdentityMatrix3x3 (gFloat *M1);

    /* Masks M1 using bit mask values from M2. */
    void MaskMatrix (gFloat *M1, gFloat *M2);

    /* Multiplies the given matrix by the scalar. */
    void MulMatrixScalar (gFloat *R, gFloat *M1, gFloat S);

    /* Adds the given scalar to each component of the matrix. */
    void AddMatrixScalar (gFloat *R, gFloat *M1, gFloat S);

    /* Subtracts the given scalar from each component of M1. */
    void SubMatrixScalar (gFloat *R, gFloat *M1, gFloat S);

    /* Subtracts the each component from the given scalar. */
    void SubScalarMatrix (gFloat *R, gFloat S, gFloat *M1);

    /* Adds the given matrices, the result is stored in M1. */
    void AddMatrixMatrix (gFloat *M1, gFloat *M2);

    /* Subtracts M2 from M1 and the result is stored in M1. */
    void SubMatrixMatrix (gFloat *M1, gFloat *M2);

    /* Multiplies the given 4x4 matrices and stores the
       resulting matrix in M1. */

    void MulMatrixMatrix (gFloat *M1, gFloat *M2);

    /* Converts the given vector (in NDC) to a 2D point, H is the vector
       that contains the half-resolution of the view port, and S the
       starting position vector. */

    void CnvTo2D (gFloat *V, gFloat *H, gFloat *S);

#endif
