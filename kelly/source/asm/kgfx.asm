/*
    KGFX.ASM

    Kelly Graphics Engine Version 0.07 (640x480x16bpp)

    Copyright (C) 2008-2009 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

Section _TEXT PARA Public 'CODE'

Publics _
Bits 32

    /* Offsets of the internal buffer. */
    .COLOR_VALUE    = 000000h   ;16-bytes

[_TEXT]

    /*********************************************************************
    **                                                                  **
    **  @Prot:  gInt StartGfx (void);                                   **
    **                                                                  **
    **  @Desc:  Starts the graphics mode, returns non-zero if there was **
    **          an error, or zero if no error ocurred.                  **
    **                                                                  **
    *********************************************************************/

    _StartGfx:
        Push    Ebx
        Push    Esi
        Push    Edi

        Mov     Eax, 4F02h
        Mov     Ebx, 0111h
        Int     10h

        Cmp     Eax, 4Fh
        Setne   Al
        And     Eax, 01h

        Mov     Ecx, @InternalBuffer
        Add     Ecx, 0Fh
        And     Ecx, -10h
        Mov     [InternalBuffer], Ecx

        Pop     Edi
        Pop     Esi
        Pop     Ebx

        Ret


    /*******************************************************************
    **                                                                **
    **  @Prot: void StopGfx (void);                                   **
    **                                                                **
    **  @Desc: Stops the graphics mode (returns to text mode).        **
    **                                                                **
    *******************************************************************/

    _StopGfx:
        Push    Ebx
        Push    Esi
        Push    Edi

        Mov     Eax, 003h
        Int     10h

        Pop     Edi
        Pop     Esi
        Pop     Ebx
        Ret

    /*********************************************************************
    **                                                                  **
    **  @Prot:  gInt FrameBufferSize (void);                            **
    **                                                                  **
    **  @Desc:  Returns the length in bytes of the frame buffer, this   **
    **          function is used to determine the size of the buffer to **
    **          allocate memory for it, returns the size plus 15 bytes  **
    **          to allow a forced alignment with AlignBuffer.           **
    **                                                                  **
    *********************************************************************/

    _FrameBufferSize:
        Mov     Eax, 12C00Fh
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void *AlignBuffer (void *Buff);                         **
    **                                                                  **
    **  @Desc:  Returns a 16-byte aligned buffer pointer, returns the   **
    **          same pointer plus an offset between zero and fifteen to **
    **          make it paragraph aligned.                              **
    **                                                                  **
    *********************************************************************/

    _AlignBuffer:
        Mov     Eax, [Esp+04h]
        Add     Eax, 0Fh
        And     Eax, -10h
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void SetFrameBuffer (void *FrameBuffer);                **
    **                                                                  **
    **  @Desc:  Sets an internal constant that points to the frame      **
    **          buffer, which must be paragraph aligned.                **
    **                                                                  **
    *********************************************************************/

    _SetFrameBuffer:
        Mov     Eax, [Esp+04h]
        Mov     [FrameBuffer], Eax
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void PutPixel (gDword Offset, gDword RGBA);             **
    **                                                                  **
    **  @Desc:  Puts a pixel on the frame buffer at the given offset,   **
    **          the color is given in RGBA 32-bit format.               **
    **                                                                  **
    *********************************************************************/

    _PutPixel:
        Mov     Edx, [Esp+04h]
        Mov     Eax, [Esp+08h]

        Mov     Ecx, [FrameBuffer]
        Mov     [Edx*4+Ecx], Eax

        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  gDword GetPixel (gDword Offset);                        **
    **                                                                  **
    **  @Desc:  Reads a pixel from the frame buffer and returns it,     **
    **          the value is a 32-bit RGBA color.                       **
    **                                                                  **
    *********************************************************************/

    _GettPixel:
        Mov     Edx, [Esp+04h]
        Mov     Ecx, [FrameBuffer]

        Mov     Eax, [Edx*4+Ecx]
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void CopyFrameBuffer (void *Dest, void *Source);        **
    **                                                                  **
    **  @Desc:  Copies the source frame buffer into dest, both must be  **
    **          paragraph aligned and of the same size.                 **
    **                                                                  **
    *********************************************************************/

    _CopyFrameBuffer:
        Mov     Edx, [Esp+08h]
        Mov     Eax, [Esp+04h]
        Mov     Ecx, 2580h

    Copy.InnerLoop:
        Movaps  xmm0, [Edx+00h]
        Movaps  xmm1, [Edx+10h]
        Movaps  xmm2, [Edx+20h]
        Movaps  xmm3, [Edx+30h]
        Movaps  xmm4, [Edx+40h]
        Movaps  xmm5, [Edx+50h]
        Movaps  xmm6, [Edx+60h]
        Movaps  xmm7, [Edx+70h]

        Movntps [Eax+00h], xmm0
        Movntps [Eax+10h], xmm1
        Movntps [Eax+20h], xmm2
        Movntps [Eax+30h], xmm3
        Movntps [Eax+40h], xmm4
        Movntps [Eax+50h], xmm5
        Movntps [Eax+60h], xmm6
        Movntps [Eax+70h], xmm7

        Add     Edx, 80h
        Add     Eax, 80h

        Dec     Ecx
        Jnz     Copy.InnerLoop

        Sfence

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void FlipFrameBuffer (void);                            **
    **                                                                  **
    **  @Desc:  Copies the frame buffer into video memory.              **
    **                                                                  **
    *********************************************************************/

    _FlipFrameBuffer:
        Push    Esi
        Push    Edi
        Push    Ebx
        Push    Ebp

        Mov     Eax, 11h

        Mov     Esi, [FrameBuffer]
        Mov     Edx, 3CEh
        Mov     Ebx, 9

        Movq    mm6, [RedGreenMask]
        Movq    mm7, [BlueMask]

        Movq    mm4, [RedGreenMult]
        PXor    mm5, mm5

    Flip.Cycle:
        Out     Dx, Ax
        Mov     Edi, 0A0000h
        Add     Eax, 100h
        Mov     Ecx, 8192

    Flip.InnerLoop:
        Movq    mm0, [Esi+00h]
        Movq    mm2, [Esi+08h]

        Movq    mm1, mm0
        Movq    mm3, mm2

        PAnd    mm0, mm6
        PAnd    mm1, mm7

        PAnd    mm2, mm6
        PAnd    mm3, mm7

        pmaddwd mm0, mm4
        psrlw   mm1, 11

        pmaddwd mm2, mm4
        psrlw   mm3, 11

        paddd   mm0, mm1
        paddd   mm2, mm3

        pshufw  mm0, mm0, 11_11_10_00b
        pshufw  mm2, mm2, 10_00_11_11b

        POr     mm2, mm0
        Movq    [Edi], mm2

        Add     Edi, 08h
        Add     Esi, 10h

        Dec     Ecx
        Jnz     Flip.InnerLoop

        Dec     Ebx
        jnz     Flip.Cycle

        Out     Dx, Ax
        Mov     Edi, 0A0000h
        Mov     Ecx, 0C00h

    Flip.InnerLoopB:
        Movq    mm0, [Esi+00h]
        Movq    mm2, [Esi+08h]

        Movq    mm1, mm0
        Movq    mm3, mm2

        PAnd    mm0, mm6
        PAnd    mm1, mm7

        PAnd    mm2, mm6
        PAnd    mm3, mm7

        pmaddwd mm0, mm4
        psrlw   mm1, 11

        pmaddwd mm2, mm4
        psrlw   mm3, 11

        paddd   mm0, mm1
        paddd   mm2, mm3

        pshufw  mm0, mm0, 11_11_10_00b
        pshufw  mm2, mm2, 10_00_11_11b

        POr     mm2, mm0
        Movq    [Edi], mm2

        Add     Edi, 08h
        Add     Esi, 10h

        Dec     Ecx
        Jnz     Flip.InnerLoopB

        Emms

        Pop     Ebp
        Pop     Ebx
        Pop     Edi
        Pop     Esi

        Ret

    /*********************************************************************
    **                                                                  **
    **  @Prot:  void ClearFrameBufferValue (gDword RGBA);               **
    **                                                                  **
    **  @Desc:  Sets the color that will be used when the frame buffer  **
    **          is cleared.                                             **
    **                                                                  **
    *********************************************************************/

    _ClearFrameBufferValue:
        Movd    mm0, [Esp+04h]
        Mov     Edx, [InternalBuffer]

        pshufw  mm0, mm0, 01_00_01_00b

        Movq    [Edx+COLOR_VALUE+00h], mm0
        Movq    [Edx+COLOR_VALUE+08h], mm0

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void ClearFrameBuffer (void);                           **
    **                                                                  **
    **  @Desc:  Clears the frame buffer, as fast as possible.           **
    **                                                                  **
    *********************************************************************/

    _ClearFrameBuffer:
        Mov     Eax, [InternalBuffer]
        Mov     Edx, [FrameBuffer]

        Mov     Ecx, 2580h
        Movaps  xmm0, [Eax+COLOR_VALUE]

    ClearFrameBuffer.Loop:
        Movntps [Edx+00h], xmm0
        Movntps [Edx+10h], xmm0
        Movntps [Edx+20h], xmm0
        Movntps [Edx+30h], xmm0
        Movntps [Edx+40h], xmm0
        Movntps [Edx+50h], xmm0
        Movntps [Edx+60h], xmm0
        Movntps [Edx+70h], xmm0

        Add     Edx, 80h
        Dec     Ecx
        Jnz     ClearFrameBuffer.Loop

        Emms
        Ret

/*#*/

    /*********************************************************************
    **                                                                  **
    **  @Prot:  void DrawLine (int x1, int y1, int x2, int y2, int c);  **
    **                                                                  **
    **  @Desc:  Draws a line from (x1,y1) to (x2,y2) with color c.      **
    **                                                                  **
    *********************************************************************/

    _DrawLine:
        Push    Edi
        Push    Esi
        Push    Ebx

        Mov     Ebx, [Esp+14h]
        Mov     Eax, [Esp+10h]

        Mov     Edi, Ebx
        Lea     Eax, [Eax*4]

        Shl     Ebx, 11
        Add     Eax, [FrameBuffer]

        Shl     Edi, 9
        Add     Ebx, Eax

        Mov     Eax, [Esp+20h]
        Add     Edi, Ebx

        Mov     Ecx, [Esp+18h]
        Mov     Edx, [Esp+1Ch]

        Sub     Ecx, [Esp+10h]
        Sub     Edx, [Esp+14h]

        Cmp     Ecx, 0
        Jl      Short DrawLine.NegativeDX

        Mov     d[XInc], +4
        Jmp     Short DrawLine.CheckDY

    DrawLine.NegativeDX:
        Neg     Ecx
        Mov     d[XInc], -4

    DrawLine.CheckDY:
        Cmp     Edx, 0
        Jl      Short DrawLine.NegativeDY

        Mov     d[YInc], +0A00h
        Jmp     Short DrawLine.Prepare

    DrawLine.NegativeDY:
        Neg     Edx
        Mov     d[YInc], -0A00h

    DrawLine.Prepare:
        Inc     Ecx
        Inc     Edx

        Lea     Esi, [Ecx+Edx]
        Test    Esi, Esi
        Jnz     DrawLine.GoodLine

        Pop     Ebx
        Pop     Esi
        Pop     Edi

        Ret

    DrawLine.GoodLine:
        Cmp     Ecx, Edx
        Jae     Short DrawLine.Begin

        Mov     Esi, d[XInc]
        xchg    Ecx, Edx
        xchg    Esi, d[YInc]
        Mov     d[XInc], Esi

    DrawLine.Begin:
        Push    Ebp
        Sub     Esi, Esi

        Mov     Ebx, Ecx
        Mov     Ebp, d[XInc]

    DrawLine.Loop:
        Mov     [Edi], Eax

        Add     Esi, Edx
        Add     Edi, Ebp

        Cmp     Esi, Ecx
        Jb      Short DrawLine.Continue

        Sub     Esi, Ecx
        Add     Edi, d[YInc]

    DrawLine.Continue:
        Dec     Ebx
        Jnz     Short DrawLine.Loop

        Pop     Ebp
        Pop     Ebx
        Pop     Esi
        Pop     Edi

        Ret

/*#*/

    /*********************************************************************
    **                                                                  **
    **  @Prot:  void CopyVector (gFloat *Dest, gFloat *Src);            **
    **                                                                  **
    **  @Desc:  Copies the source vector into dest.                     **
    **                                                                  **
    *********************************************************************/

    _CopyVector:
        Mov     Edx, [Esp+08h]
        Mov     Eax, [Esp+04h]

        Movups  xmm0, [Edx]
        Movups  [Eax], xmm0

        Emms
        Ret

    /*********************************************************************
    **                                                                  **
    **  @Prot:  void NormalizeVector (gFloat *Dst, gFloat *Src);        **
    **                                                                  **
    **  @Desc:  Normalizes the given vector, stores the result in Dst.  **
    **                                                                  **
    *********************************************************************/

    _NormalizeVector:
        Mov     Edx, [Esp+08h]
        Mov     Eax, [Esp+04h]

        Movups  xmm0, [Edx]
        Movaps  xmm1, xmm0
        Mulps   xmm0, xmm0
        Movaps  xmm2, xmm0

        Shufps  xmm0, xmm0, 11_10_00_01b
        Movaps  xmm3, xmm2

        Shufps  xmm2, xmm2, 11_00_01_10b
        Addss   xmm3, xmm0

        Addss   xmm3, xmm2
        rsqrtss xmm3, xmm3

        Shufps  xmm3, xmm3, 00h
        Mulps   xmm1, xmm3

        Movups  [Eax], xmm1

        Emms
        Ret

    /*********************************************************************
    **                                                                  **
    **  @Prot:  void CopyMatrix (gFloat *Dest, gFloat *Src);            **
    **                                                                  **
    **  @Desc:  Copies the source 4x4 matrix into dest.                 **
    **                                                                  **
    *********************************************************************/

    _CopyMatrix:
        Mov     Edx, [Esp+08h]
        Mov     Eax, [Esp+04h]

        Movups  xmm0, [Edx+00h]
        Movups  xmm1, [Edx+10h]

        Movups  xmm2, [Edx+20h]
        Movups  xmm3, [Edx+30h]

        Movups  [Eax+00h], xmm0
        Movups  [Eax+10h], xmm1

        Movups  [Eax+20h], xmm2
        Movups  [Eax+30h], xmm3

        Emms
        Ret

    /*********************************************************************
    **                                                                  **
    **  @Prot:  void MulVectMatrix (gFloat *R, gFloat *V, gFloat *M);   **
    **                                                                  **
    **  @Desc:  Multiplies the given vector by the 4x4 matrix and the   **
    **          resulting vector is stored in R.                        **
    **                                                                  **
    *********************************************************************/

    _MulVectMatrix:
        Mov     Edx, [Esp+04h]
        Mov     Eax, [Esp+08h]

        Mov     Ecx, [Esp+0Ch]
        Nop

        Movups  xmm0, [Eax+00h]
        Movups  xmm4, [Ecx+00h]

        Movaps  xmm1, xmm0
        Movups  xmm5, [Ecx+10h]

        Movaps  xmm2, xmm0
        Movups  xmm6, [Ecx+20h]

        Movaps  xmm3, xmm0
        Movups  xmm7, [Ecx+30h]

        Shufps  xmm0, xmm0, 00_00_00_00b
        Shufps  xmm1, xmm1, 01_01_01_01b

        Shufps  xmm2, xmm2, 10_10_10_10b
        Shufps  xmm3, xmm3, 11_11_11_11b

        Mulps   xmm0, xmm4
        Mulps   xmm3, xmm7

        Mulps   xmm1, xmm5
        Addps   xmm0, xmm3

        Mulps   xmm2, xmm6
        Addps   xmm0, xmm1

        Addps   xmm0, xmm2
        Movups  [Edx], xmm0

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void CopyZeroMatrix (gFloat *M1);                       **
    **                                                                  **
    **  @Desc:  Copies the zero matrix into M1.                         **
    **                                                                  **
    *********************************************************************/

    _CopyZeroMatrix:
        Mov     Edx, [Esp+04h]
        Xorps   xmm0, xmm0

        Movups  [Edx+00h], xmm0
        Movups  [Edx+10h], xmm0

        Movups  [Edx+20h], xmm0
        Movups  [Edx+30h], xmm0

        Emms
        Ret

    /*********************************************************************
    **                                                                  **
    **  @Prot:  void CopyIdentityMatrix (gFloat *M1);                   **
    **                                                                  **
    **  @Desc:  Copies the identity matrix into M1.                     **
    **                                                                  **
    *********************************************************************/

    _CopyIdentityMatrix:
        Mov     Eax, 1
        Xorps   xmm0, xmm0

        Mov     Edx, [Esp+04h]

        Cvtsi2ss xmm0, Eax

        Movaps  xmm1, xmm0
        Movaps  xmm2, xmm0
        Movaps  xmm3, xmm0

        Shufps  xmm0, xmm0, 11_11_11_00b
        Shufps  xmm1, xmm1, 11_11_00_11b

        Shufps  xmm2, xmm2, 11_00_11_11b
        Shufps  xmm3, xmm3, 00_11_11_11b

        Movups  [Edx+00h], xmm0
        Movups  [Edx+10h], xmm1

        Movups  [Edx+20h], xmm2
        Movups  [Edx+30h], xmm3

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void CopyIdentityMatrix3x3 (gFloat *M1);                **
    **                                                                  **
    **  @Desc:  Copies a 3x3 identity matrix into M1 (which is 4x4).    **
    **                                                                  **
    *********************************************************************/

    _CopyIdentityMatrix3x3:
        Mov     Eax, 1
        Xorps   xmm0, xmm0

        Mov     Edx, [Esp+04h]
        Cvtsi2ss xmm0, Eax

        Xorps   xmm3, xmm3
        Movaps  xmm1, xmm0

        Movaps  xmm2, xmm0
        Shufps  xmm0, xmm0, 11_11_11_00b

        Shufps  xmm1, xmm1, 11_11_00_11b
        Shufps  xmm2, xmm2, 11_00_11_11b

        Movups  [Edx+00h], xmm0
        Movups  [Edx+10h], xmm1

        Movups  [Edx+20h], xmm2
        Movups  [Edx+30h], xmm3

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void MaskMatrix (gFloat *M1, gFloat *M2);               **
    **                                                                  **
    **  @Desc:  Masks M1 using bit mask values from M2.                 **
    **                                                                  **
    *********************************************************************/

    _MaskMatrix:
        Mov     Edx, [Esp+04h]
        Mov     Eax, [Esp+08h]

        Movups  xmm0, [Edx+00h]
        Movups  xmm1, [Edx+10h]

        Movups  xmm2, [Edx+20h]
        Movups  xmm3, [Edx+30h]

        Movups  xmm4, [Eax+00h]
        Movups  xmm5, [Eax+10h]

        Movups  xmm6, [Eax+20h]
        Movups  xmm7, [Eax+30h]

        Andps   xmm0, xmm4
        Andps   xmm1, xmm5

        Andps   xmm2, xmm6
        Andps   xmm3, xmm7

        Movups  [Edx+00h], xmm0
        Movups  [Edx+10h], xmm1

        Movups  [Edx+20h], xmm2
        Movups  [Edx+30h], xmm3

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void MulMatrixScalar (gFloat *R, gFloat *M1, gFloat S); **
    **                                                                  **
    **  @Desc:  Multiplies the given matrix by the scalar.              **
    **                                                                  **
    *********************************************************************/

    _MulMatrixScalar:
        Mov     Edx, [Esp+08h]
        Movups  xmm4, [Esp+0Ch]

        Movups  xmm0, [Edx+00h]
        Movups  xmm1, [Edx+10h]

        Shufps  xmm4, xmm4, 00h
        Mov     Ecx, [Esp+04h]

        Movups  xmm2, [Edx+20h]
        Movups  xmm3, [Edx+30h]

        Mulps   xmm0, xmm4
        Mulps   xmm1, xmm4

        Mulps   xmm2, xmm4
        Mulps   xmm3, xmm4

        Movups  [Ecx+00h], xmm0
        Movups  [Ecx+10h], xmm1

        Movups  [Ecx+20h], xmm2
        Movups  [Ecx+30h], xmm3

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void AddMatrixScalar (gFloat *R, gFloat *M1, gFloat V); **
    **                                                                  **
    **  @Desc:  Adds the given scalar to each component of the matrix.  **
    **                                                                  **
    *********************************************************************/

    _AddMatrixScalar:
        Mov     Edx, [Esp+08h]
        Movups  xmm4, [Esp+0Ch]

        Movups  xmm0, [Edx+00h]
        Movups  xmm1, [Edx+10h]

        Shufps  xmm4, xmm4, 00h
        Mov     Ecx, [Esp+04h]

        Movups  xmm2, [Edx+20h]
        Movups  xmm3, [Edx+30h]

        Addps   xmm0, xmm4
        Addps   xmm1, xmm4

        Addps   xmm2, xmm4
        Addps   xmm3, xmm4

        Movups  [Ecx+00h], xmm0
        Movups  [Ecx+10h], xmm1

        Movups  [Ecx+20h], xmm2
        Movups  [Ecx+30h], xmm3

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void SubMatrixScalar (gFloat *R, gFloat *M1, gFloat S); **
    **                                                                  **
    **  @Desc:  Subtracts the given scalar from each component of M1.   **
    **                                                                  **
    *********************************************************************/

    _SubMatrixScalar:
        Mov     Edx, [Esp+08h]
        Movups  xmm4, [Esp+0Ch]

        Movups  xmm0, [Edx+00h]
        Movups  xmm1, [Edx+10h]

        Shufps  xmm4, xmm4, 00h
        Movups  xmm2, [Edx+20h]

        Movups  xmm3, [Edx+30h]
        Mov     Edx, [Esp+04h]

        Subps   xmm0, xmm4
        Subps   xmm1, xmm4

        Subps   xmm2, xmm4
        Subps   xmm3, xmm4

        Movups  [Edx+00h], xmm0
        Movups  [Edx+10h], xmm1

        Movups  [Edx+20h], xmm2
        Movups  [Edx+30h], xmm3

        Emms
        Ret

    /*********************************************************************
    **                                                                  **
    **  @Prot:  void SubScalarMatrix (gFloat *R, gFloat S, gFloat *M1); **
    **                                                                  **
    **  @Desc:  Subtracts the each component from the given scalar.     **
    **                                                                  **
    *********************************************************************/

    _SubScalarMatrix:
        Mov     Edx, [Esp+0Ch]
        Movups  xmm4, [Esp+08h]

        Movups  xmm0, [Edx+00h]
        Movups  xmm1, [Edx+10h]

        Shufps  xmm4, xmm4, 00h
        Mov     Ecx, [Esp+04h]

        Movups  xmm2, [Edx+20h]
        Movups  xmm3, [Edx+30h]

        Movups  xmm5, xmm4
        Movups  xmm6, xmm4

        Movups  xmm7, xmm4
        Subps   xmm4, xmm0

        Subps   xmm5, xmm1
        Subps   xmm6, xmm2

        Subps   xmm7, xmm3
        Movups  [Ecx+00h], xmm4

        Movups  [Ecx+10h], xmm5
        Movups  [Ecx+20h], xmm6

        Movups  [Ecx+30h], xmm7

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void AddMatrixMatrix (gFloat *M1, gFloat *M2);          **
    **                                                                  **
    **  @Desc:  Adds the given matrices, the result is stored in M1.    **
    **                                                                  **
    *********************************************************************/

    _AddMatrixMatrix:
        Mov     Edx, [Esp+08h]
        Mov     Eax, [Esp+04h]

        Movups  xmm0, [Edx+00h]
        Movups  xmm1, [Edx+10h]

        Movups  xmm2, [Edx+20h]
        Movups  xmm3, [Edx+30h]

        Movups  xmm4, [Eax+00h]
        Movups  xmm5, [Eax+10h]

        Movups  xmm6, [Eax+20h]
        Movups  xmm7, [Eax+30h]

        Addps   xmm0, xmm4
        Addps   xmm1, xmm5

        Addps   xmm2, xmm6
        Addps   xmm3, xmm7

        Movups  [Eax+00h], xmm0
        Movups  [Eax+10h], xmm1

        Movups  [Eax+20h], xmm2
        Movups  [Eax+30h], xmm3

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void SubMatrixMatrix (gFloat *M1, gFloat *M2);          **
    **                                                                  **
    **  @Desc:  Subtracts M2 from M1 and the result is stored in M1.    **
    **                                                                  **
    *********************************************************************/

    _SubMatrixMatrix:
        Mov     Edx, [Esp+04h]
        Mov     Eax, [Esp+08h]

        Movups  xmm0, [Edx+00h]
        Movups  xmm1, [Edx+10h]

        Movups  xmm2, [Edx+20h]
        Movups  xmm3, [Edx+30h]

        Movups  xmm4, [Eax+00h]
        Movups  xmm5, [Eax+10h]

        Movups  xmm6, [Eax+20h]
        Movups  xmm7, [Eax+30h]

        Subps   xmm0, xmm4
        Subps   xmm1, xmm5

        Subps   xmm2, xmm6
        Subps   xmm3, xmm7

        Movups  [Edx+00h], xmm0
        Movups  [Edx+10h], xmm1

        Movups  [Edx+20h], xmm2
        Movups  [Edx+30h], xmm3

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void MulMatrixMatrix (gFloat *M1, gFloat *M2);          **
    **                                                                  **
    **  @Desc:  Multiplies the given 4x4 matrices and stores the        **
    **          resulting matrix in M1.                                 **
    **                                                                  **
    *********************************************************************/

    _MulMatrixMatrix:
        Mov     Eax, [Esp+04h]
        Mov     Ecx, [Esp+08h]

        Movups  xmm0, [Eax+00h]
        Movups  xmm4, [Ecx+00h]

        Movaps  xmm1, xmm0
        Movups  xmm5, [Ecx+10h]

        Movaps  xmm2, xmm0
        Movups  xmm6, [Ecx+20h]

        Movaps  xmm3, xmm0
        Movups  xmm7, [Ecx+30h]

        Shufps  xmm0, xmm0, 00_00_00_00b
        Shufps  xmm1, xmm1, 01_01_01_01b

        Shufps  xmm2, xmm2, 10_10_10_10b
        Shufps  xmm3, xmm3, 11_11_11_11b

        Mulps   xmm0, xmm4
        Mulps   xmm3, xmm7

        Mulps   xmm1, xmm5
        Addps   xmm0, xmm3

        Mulps   xmm2, xmm6
        Addps   xmm0, xmm1

        Movups  xmm3, [Eax+10h]
        Addps   xmm0, xmm2

        Movaps  xmm1, xmm3
        Movaps  xmm2, xmm3

        Movups  [Eax+00h], xmm0
        Movaps  xmm0, xmm3

        Shufps  xmm2, xmm2, 10_10_10_10b
        Shufps  xmm3, xmm3, 11_11_11_11b

        Shufps  xmm0, xmm0, 00_00_00_00b
        Shufps  xmm1, xmm1, 01_01_01_01b

        Mulps   xmm0, xmm4
        Mulps   xmm3, xmm7

        Mulps   xmm1, xmm5
        Addps   xmm0, xmm3

        Mulps   xmm2, xmm6
        Addps   xmm0, xmm1

        Movups  xmm3, [Eax+20h]
        Addps   xmm0, xmm2

        Movaps  xmm1, xmm3
        Movaps  xmm2, xmm3

        Movups  [Eax+10h], xmm0
        Movaps  xmm0, xmm3

        Shufps  xmm2, xmm2, 10_10_10_10b
        Shufps  xmm3, xmm3, 11_11_11_11b

        Shufps  xmm0, xmm0, 00_00_00_00b
        Shufps  xmm1, xmm1, 01_01_01_01b

        Mulps   xmm0, xmm4
        Mulps   xmm3, xmm7

        Mulps   xmm1, xmm5
        Addps   xmm0, xmm3

        Mulps   xmm2, xmm6
        Addps   xmm0, xmm1

        Movups  xmm3, [Eax+30h]
        Addps   xmm0, xmm2

        Movaps  xmm1, xmm3
        Movaps  xmm2, xmm3

        Movups  [Eax+20h], xmm0
        Movaps  xmm0, xmm3

        Shufps  xmm2, xmm2, 10_10_10_10b
        Shufps  xmm3, xmm3, 11_11_11_11b

        Shufps  xmm0, xmm0, 00_00_00_00b
        Shufps  xmm1, xmm1, 01_01_01_01b

        Mulps   xmm0, xmm4
        Mulps   xmm3, xmm7

        Mulps   xmm1, xmm5
        Mulps   xmm2, xmm6

        Addps   xmm0, xmm3
        Addps   xmm0, xmm1

        Addps   xmm0, xmm2
        Movups  [Eax+30h], xmm0

        Emms
        Ret


    /*********************************************************************
    **                                                                  **
    **  @Prot:  void CnvTo2D (gFloat *V, gFloat *H, gFloat *S);         **
    **                                                                  **
    **  @Desc:  Converts the given vector (in NDC) to a 2D point, H is  **
    **          the vector that contains the half-resolution of the     **
    **          view port, and S the starting position vector.          **
    **                                                                  **
    *********************************************************************/

    _CnvTo2D:
        Mov     Ecx, [Esp+04h]
        Mov     Eax, [Esp+08h]

        Movups  xmm1, [Ecx]         ;xmm1: Vw Vz Vy Vx
        Movups  xmm0, [Eax]         ;xmm0: Hw Hz Hy Hx

        Movaps  xmm3, xmm1          ;xmm3: Vw Vz Vy Vx
        Mov     Edx, 1

        Mov     Eax, [Esp+0Ch]
        Cvtsi2ss xmm3, Edx          ;xmm3: Vw Vz Vy 1.0

        Mulps   xmm1, xmm0          ;xmm1: Vw*Hw Vz*Hz Vy*Hy Vx*Hx
        Shufps  xmm3, xmm3, 03Fh    ;xmm3: 1.0 Vw Vw Vw

        Movups  xmm2, [Eax]         ;xmm2: Sw Sz Sy Sx
        Divps   xmm1, xmm3          ;xmm1: Vw*Hw/1 Vz*Hz/Vw Vy*Hy/Vw Vx*Hx/Vw

        Addps   xmm1, xmm2          ;xmm1: Sw+Vw*Hw/1 Sz+Vz*Hz/Vw Sy+Vy*Hy/Vw Sx+Vx*Hx/Vw
        Movups  [Ecx], xmm1

        Emms
        Ret

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    Align       4

    FrameBuffer     dd  00000000h
    InternalBuffer  dd  00000000h

    XInc            dd  00000000h
    YInc            dd  00000000h

    RedGreenMask    dq  00F8_00FC_00F8_00FCh
    BlueMask        dq  0000_F800_0000_F800h
    RedGreenMult    dq  0100_0008_0100_0008h

    @InternalBuffer db  32+16 dup (0)

[ENDS]
