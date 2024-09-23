/*
    K3D.C

    Kelly 3D Graphics Engine Version 0.07

    Copyright (C) 2008-2009 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <k3d.h>
    #include <math.h>
    #include <stdio.h>

    /* Maximum level of each matrix stack. */
    #define MAX_MATRIX_STCK_L 64

    /* Maximum level of the object stack. */
    #define MAX_OBJ_STCK_L 64

    /* Level of each stack (count of items pushed). */
    private gInt MViewMatStckLev, ProjMatStckLev, ObjStckLev, *StckLev;

    /* Model view matrix, projection matrix and object stacks. */
    private gMatrix *MViewMatStck, *MViewMatrix, *ProjMatStck, *ProjMatrix;
    private gObject *Object, **ObjStck;

    /* Indicates if K3D has been initiated. */
    private gInt Initiated = 0;

    /* Allocated color buffer. */
    private gByte *ColorBuffer;

    /* Pointer to the current matrix. */
    private gMatrix **Matrix;

    /* Identity matrix. */
    private gMatrix IdentityMatrix =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    /* Default projection matrix. */
    private gMatrix DefProjMatrix =
    {
        2.0/ScreenWidth, 0, 0, 0,
        0, -2.0/ScreenHeight, 0, 0,
        0, 0, 0.001, 0.001,
        0, 0, 0, 1,
    };

    /* Current viewport coordinates. */
    private gInt ViewportX1, ViewportY1, ViewportX2, ViewportY2;

    /* The half resolution vector. */
    private gFloat HalfRes [] =
    {
        0, 0, 1, 1
    };

    /* The starting position vector. */
    private gFloat StartPos [] =
    {
        0, 0, 0, 0
    };

/*#*/
    /* Initializes the KGfx and K3D engines. */
    grBool gStart (void)
    {
            if (StartGfx ()) return 1;

            MViewMatStck = (gMatrix *)gAlloc (MAX_MATRIX_STCK_L * sizeof(gMatrix));
            ProjMatStck = (gMatrix *)gAlloc (MAX_MATRIX_STCK_L * sizeof(gMatrix));

            ObjStck = (gObject **)gAlloc (MAX_OBJ_STCK_L * sizeof (gObject *));

            ColorBuffer = (gByte *)AlignBuffer (gAlloc (FrameBufferSize ()));

            if (!MViewMatStck || !ProjMatStck || !ObjStck || !ColorBuffer)
                return 1;

            MViewMatrix = MViewMatStck;
            ProjMatrix = ProjMatStck;

            MViewMatStckLev = ProjMatStckLev = ObjStckLev = 1;

            ObjStck [0] = Object = NULL;

            gMatrixMode (MODELVIEW_MATRIX);
            gLoadMatrixv (&IdentityMatrix);

            gMatrixMode (PROJECTION_MATRIX);
            gLoadMatrixv (&DefProjMatrix);

            gViewport (0, 0, ScreenWidth, ScreenHeight);

            SetFrameBuffer (ColorBuffer);

            return !(Initiated = 1);
    }

    /* De-initializes KGfx and K3D. */
    void gStop (void)
    {
            if (!Initiated) return;

            StopGfx ();

            gFree (MViewMatStck);
            gFree (ProjMatStck);

            Initiated = 0;
    }

/*#*/
    /* Sets the viewport given its position and dimension. */
    void gViewport (gFloat x, gFloat y, gFloat w, gFloat h)
    {
            HalfRes [0] = (w - 1) / 2.0;
            HalfRes [1] = (h - 1) / 2.0;

            StartPos [0] = HalfRes [0] + x;
            StartPos [1] = HalfRes [1] + y;

            ViewportX1 = x;
            ViewportX2 = x + w - 1;

            ViewportY1 = y;
            ViewportY2 = y + h - 1;
    }

    /* Copies the frame buffer into the video memory. */
    void gFlip (void)
    {
            FlipFrameBuffer ();
    }

    /* Clears the internal buffers selected by the bit mask. */
    void gClear (gByte BitMask)
    {
            if (BitMask & COLOR_BUFFER)
            {
                ClearFrameBuffer ();
            }
    }

    /* Sets the color used to clear the color buffer. */
    void gClearColor (gDword RGBA)
    {
            ClearFrameBufferValue (RGBA);
    }

/*#*/
    /* Changes the mode of the matrix operation module. */
    void gMatrixMode (gByte MatrixMode)
    {
            switch (MatrixMode)
            {
                case MODELVIEW_MATRIX:
                    StckLev = &MViewMatStckLev;
                    Matrix = &MViewMatrix;
                    break;

                case PROJECTION_MATRIX:
                    StckLev = &ProjMatStckLev;
                    Matrix = &ProjMatrix;
                    break;
            }
    }

    /* Pushes the current matrix in the stack. */
    gBool gPushMatrix (void)
    {
            if ((*StckLev) == MAX_MATRIX_STCK_L)
                return 0;

            (*StckLev)++, (*Matrix)++;

            return 1;
    }

    /* Pops a matrix from the stack and uses it as the current matrix. */
    gBool gPopMatrix (void)
    {
            if ((*StckLev) == 1)
                return 0;

            (*StckLev)--, (*Matrix)--;

            return 1;
    }

    /* Returns a pointer to the ith matrix of the stack. */
    gMatrix *gGetMatrix (gInt i)
    {
            if (i >= (*StckLev))
                return NULL;

            return (*Matrix) - i;
    }

    /* Loads the identity in the current matrix. */
    void gLoadIdentity (void)
    {
            CopyIdentityMatrix (*Matrix);
    }

    /* Loads the given matrix into the current matrix. */
    void gLoadMatrixv (gFloat *Ptr)
    {
            CopyMatrix (*Matrix, Ptr);
    }

    /* Loads the given matrix values into the current matrix. */
    void gLoadMatrixf (gFloat First, ...)
    {
            CopyMatrix (*Matrix, &First);
    }

    /* Multiplies the current matrix by the given matrix. */
    void gMulMatrixv (gFloat *Ptr)
    {
            MulMatrixMatrix ((gFloat *)*Matrix, Ptr);
    }

    /* Multiplies the current matrix by the given matrix. */
    void gMulMatrixf (gFloat First, ...)
    {
            MulMatrixMatrix ((gFloat *)*Matrix, &First);
    }

    /* Scales by the given factors respectively for each axis. */
    void gScalef (gFloat x, gFloat y, gFloat z)
    {
        static gFloat Temp[16] =
        {
            2, 0, 0, 0,
            0, 2, 0, 0,
            0, 0, 2, 0,
            0, 0, 0, 1,
        };

            Temp [0x00] = x;
            Temp [0x05] = y;
            Temp [0x0A] = z;

            MulMatrixMatrix ((gFloat *)*Matrix, &Temp);
    }

    /* Performs translation to the given coordinates. */
    void gTranslatef (gFloat x, gFloat y, gFloat z)
    {
        static gFloat Temp[16] =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            2, 2, 2, 1,
        };

            Temp [0x0C] = x;
            Temp [0x0D] = y;
            Temp [0x0E] = z;

            MulMatrixMatrix ((gFloat *)*Matrix, &Temp);
    }

    /* Rotates the given axises. */
    void gRotateAxis (gFloat rads, gInt Axis)
    {
        static gFloat X_Axis[] =
        {
            1, 0, 0, 0,
            0, 2, 2, 0,
            0, 2, 2, 0,
            0, 0, 0, 1,
        };

        static gFloat Y_Axis[] =
        {
            2, 0, 2, 0,
            0, 1, 0, 0,
            2, 0, 2, 0,
            0, 0, 0, 1,
        };

        static gFloat Z_Axis[] =
        {
            2, 2, 0, 0,
            2, 2, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
        };

        gFloat c = cos (rads), s = sin (rads);

            if (Axis & X_AXIS)
            {
                X_Axis [0x05] = X_Axis [0x0A] = c;
                X_Axis [0x06] = -s;
                X_Axis [0x09] = s;

                MulMatrixMatrix ((gFloat *)*Matrix, &X_Axis);
            }

            if (Axis & Y_AXIS)
            {
                Y_Axis [0x00] = Y_Axis [0x0A] = c;
                Y_Axis [0x02] = -s;
                Y_Axis [0x08] = s;

                MulMatrixMatrix ((gFloat *)*Matrix, &Y_Axis);
            }

            if (Axis & Z_AXIS)
            {
                Z_Axis [0x00] = Z_Axis [0x05] = c;
                Z_Axis [0x01] = s;
                Z_Axis [0x04] = -s;

                MulMatrixMatrix ((gFloat *)*Matrix, &Z_Axis);
            }
    }

    /* Rotates through the given axis. */
    void gRotatef (gFloat Factor, gFloat x, gFloat y, gFloat z)
    {
        static gFloat Temp1[16], Temp2[16], Temp3[16];
        static gFloat Vector[4];

            CopyZeroMatrix (&Temp1);
            CopyZeroMatrix (&Temp2);
            CopyZeroMatrix (&Temp3);

            NormalizeVector (&Vector, &x);

            /* (u.uT)(1 - Cos �) + (S)(Sin �) + I(Cos �) */

            Temp1[0] = Temp2[0] = Temp3[6] = Vector[0],
            Temp1[4] = Temp2[1] = Temp3[8] = Vector[1],
            Temp1[8] = Temp2[2] = Temp3[1] = Vector[2];

            Temp3[2] = -Vector[1],
            Temp3[4] = -Vector[2],
            Temp3[9] = -Vector[0];

            MulMatrixMatrix (&Temp1, &Temp2);

            CopyIdentityMatrix3x3 (&Temp2);

            MulMatrixScalar (&Temp1, &Temp1, 1.0 - cos (Factor));
            MulMatrixScalar (&Temp2, &Temp2, cos (Factor));
            MulMatrixScalar (&Temp3, &Temp3, sin (Factor));

            AddMatrixMatrix (&Temp1, &Temp2);
            AddMatrixMatrix (&Temp1, &Temp3);

            Temp1 [0x0F] = 1.0;

            MulMatrixMatrix ((gFloat *)*Matrix, &Temp1);
    }

    /* Multiplies the current matrix by a parallel projection matrix. */
    void gOrtho (gFloat Left, gFloat Right, gFloat Top, gFloat Bottom, gFloat Near, gFloat Far)
    {
        static gFloat Temp[16] =
        {
            2, 0, 0, 0,
            0, 2, 0, 0,
            0, 0, 2, 0,
            2, 2, 2, 1,
        };

            Temp[0x00] = 2 / (Right - Left);
            Temp[0x05] = 2 / (Bottom - Top);
            Temp[0x0A] = 2 / (Near - Far);

            Temp[0x0C] = -(Right + Left) / (Right - Left);
            Temp[0x0D] = -(Top + Bottom) / (Bottom - Top);
            Temp[0x0E] = -(Far + Near) / (Far - Near);

            MulMatrixMatrix ((gFloat *)*Matrix, &Temp);
    }

    /* Multiplies the current matrix by a perspective proj matrix. */
    void gPerspective (gFloat Fov, gFloat Aspect, gFloat Near, gFloat Far)
    {
        static gFloat Temp[] =
        {
            2, 0, 0, 0,
            0, 2, 0, 0,
            0, 0, 2, -1,
            0, 0, 2, 1,
        };

            Temp[0x00] = 1 / Aspect * tan (Fov/2);
            Temp[0x05] = -tan (Fov/2);
            Temp[0x0A] = (Near + Far) / (Near - Far);

            Temp[0x0E] = (2 * Near * Far) / (Near - Far);

            MulMatrixMatrix ((gFloat *)*Matrix, &Temp);
    }

/*#*/
    /* Begins the definition of an object of the given type. */
    grBool gBeginObjectDef (gEnum Type)
    {
            if (Type > MAX_OBJECT_TYPE || ObjStckLev == MAX_OBJ_STCK_L)
                return 1;

            switch (Type)
            {
                case POINT_OBJECT:
                    Object = (gObject *)gAllocC (sizeof (gPointObject));
                    break;

                case LINE_OBJECT:
                case POLYLINE_OBJECT:
                    Object = (gObject *)gAllocC (sizeof (gLineObject));
                    break;

                case TRI_OBJECT:
                    Object = (gObject *)gAllocC (sizeof (gTriObject));
                    break;
//violet
                case QUAD_OBJECT:
                    break;

                case MESH_OBJECT:
                    break;

                case COMPOSITE_OBJECT:
                    break;
            }

            ObjStck [ObjStckLev++] = Object;
            Object->Type = Type;

            return 0;
    }

    /* Returns a pointer to the object being defined. */
    gObject *gDefiningObject (void)
    {
            if (ObjStckLev == 1) return NULL;

            return ObjStck [ObjStckLev - 1];
    }

    /* Ends the definition of the current object, returns its pointer. */
    gObject *gEndObjectDef (void)
    {
            if (ObjStckLev == 1) return NULL;

            Object = ObjStck [ObjStckLev - 2];

            return ObjStck [--ObjStckLev];
    }

    /* Destroys a previosly created object. */
    void gDestroyObject (gObject *Ptr)
    {
        void *Vertex, *nVertex;

            if (!Ptr) return;

            switch (Ptr->Type)
            {
                case POINT_OBJECT:
                case LINE_OBJECT:
                case POLYLINE_OBJECT:

                    for (Vertex = ((gPointObject *)Object)->Top; Vertex;
                         Vertex = nVertex)
                    {
                        nVertex =  ((gVertexLC *)Vertex)->Next;
                        gFree (Vertex);
                    }

                    break;
//violet
                case TRI_OBJECT:

                    for (Vertex = ((gTriObject *)Object)->Top; Vertex;
                         Vertex = nVertex)
                    {
                        nVertex =  ((gVertexLCT *)Vertex)->Next;
                        gFree (Vertex);
                    }

                    break;

                case QUAD_OBJECT:
                    break;

                case MESH_OBJECT:
                    break;

                case COMPOSITE_OBJECT:
                    break;
            }

            gFree (Ptr);
    }

    /* Cancels the definition of the current object. */
    void gCancelObjectDef (void)
    {
            gDestroyObject (gEndObjectDef ());
    }

/*#*/
    /* Renders an object of type POINT_OBJECT. */
    private void RenderPointObject (gPointObject *Ptr)
    {
        gVertexLC *Vertex;
        gFloat Temp[4];
        gDword x, y;

            for (Vertex = Ptr->Top; Vertex; Vertex = Vertex->Next)
            {
                MulVectMatrix (&Temp, Vertex, MViewMatrix);
                MulVectMatrix (&Temp, &Temp, ProjMatrix);

                CnvTo2D (&Temp, &HalfRes, &StartPos);

                x = Temp[0];
                y = Temp[1];

                if (x >= ViewportX1 && x <= ViewportX2 &&
                    y >= ViewportY1 && y <= ViewportY2)
                {
                    PutPixel (PixelOffset (x, y), Vertex->Color);
                }
            }
    }

    /* Clips along the Z-Axis. */
    private void ClipZ (gFloat *P, gFloat *Q)
    {
        gFloat x, y;

            if (Q[3] < 0.001)
            {
                if (Q[3] != P[3])
                {
                    x = P[0] + ((Q[0] - P[0])/(Q[3] - P[3])) * (0.001 - P[3]);
                    y = P[1] + ((Q[1] - P[1])/(Q[3] - P[3])) * (0.001 - P[3]);

                    Q[0] = x;
                    Q[1] = y;
                }

                Q[3] = 0.001;
            }

            if (P[3] < 0.001)
            {
                if (Q[3] != P[3])
                {
                    x = P[0] + ((Q[0] - P[0])/(Q[3] - P[3])) * (0.001 - P[3]);
                    y = P[1] + ((Q[1] - P[1])/(Q[3] - P[3])) * (0.001 - P[3]);

                    P[0] = x;
                    P[1] = y;
                }

                P[3] = 0.001;
            }

    }

    /**/
    private void ClipXY (gFloat *P, gFloat *Q)
    {
        gFloat Temp;

            if (P[0] < ViewportX1 || P[0] > ViewportX2)
            {
                Temp = (P[0] < ViewportX1) ? ViewportX1 : ViewportX2 ;

                if (Q[0] != P[0])
                    P[1] = P[1] + ((Q[1] - P[1])/(Q[0] - P[0])) * (Temp - P[0]);

                P[0] = Temp;
            }

            if (P[1] < ViewportY1 || P[1] > ViewportY2)
            {
                Temp = (P[1] < ViewportY1) ? ViewportY1 : ViewportY2 ;

                if (Q[1] != P[1])
                    P[0] = P[0] + ((Q[0] - P[0])/(Q[1] - P[1])) * (Temp - P[1]);

                P[1] = Temp;
            }

            if (Q[0] < ViewportX1 || Q[0] > ViewportX2)
            {
                Temp = (Q[0] < ViewportX1) ? ViewportX1 : ViewportX2 ;

                if (Q[0] != P[0])
                    Q[1] = P[1] + ((Q[1] - P[1])/(Q[0] - P[0])) * (Temp - P[0]);

                Q[0] = Temp;
            }

            if (Q[1] < ViewportY1 || Q[1] > ViewportY2)
            {
                Temp = (Q[1] < ViewportY1) ? ViewportY1 : ViewportY2 ;

                if (Q[1] != P[1])
                    Q[0] = P[0] + ((Q[0] - P[0])/(Q[1] - P[1])) * (Temp - P[1]);

                Q[1] = Temp;
            }
    }

    /**/
    private int GetOutCode (gFloat *P)
    {
            return  (P[0] < ViewportX1 ? 1 : 0) +
                    (P[1] < ViewportY1 ? 2 : 0) +
                    (P[0] > ViewportX2 ? 4 : 0) +
                    (P[1] > ViewportY2 ? 8 : 0) ;
    }

    /* Renders an object of type LINE_OBJECT. */
    private void RenderLineObject (gLineObject *Ptr)
    {
        gFloat x1, y1, x2, y2; gInt Count, c1, c2;
        gVertexLC *Vertex1, *Vertex2;
        gFloat z1, z2, k, Temp1[4], Temp2[4];

            Vertex1 = Ptr->Top;

            for (Count = Ptr->Vertices / 2, Vertex1 = Ptr->Top; Count--; Vertex1 = Vertex2->Next)
            {
                Vertex2 = Vertex1->Next;

                MulVectMatrix (&Temp1, Vertex1, MViewMatrix);
                MulVectMatrix (&Temp2, Vertex2, MViewMatrix);

                MulVectMatrix (&Temp1, &Temp1, ProjMatrix);
                MulVectMatrix (&Temp2, &Temp2, ProjMatrix);

                if (Temp1[3] <= 0 && Temp2[3] <= 0)
                    continue;

                ClipZ (&Temp1, &Temp2);

                CnvTo2D (&Temp1, &HalfRes, &StartPos);
                CnvTo2D (&Temp2, &HalfRes, &StartPos);

                c1 = GetOutCode (&Temp1);
                c2 = GetOutCode (&Temp2);

                if (c1 & c2) continue;

                ClipXY (&Temp1, &Temp2);

                c1 = GetOutCode (&Temp1);
                c2 = GetOutCode (&Temp2);

                if (c1 || c2) continue;

                DrawLine (Temp1[0], Temp1[1], Temp2[0], Temp2[1], Vertex1->Color);
            }
    }

    /* Renders an object of type POLYLINE_OBJECT. */
    private void RenderPolylineObject (gLineObject *Ptr)
    {
        gFloat x1, y1, x2, y2; gInt Count, c1, c2;
        gVertexLC *Vertex1, *Vertex2;
        gFloat z1, z2, k, Temp1[4], Temp2[4];

            Vertex1 = Ptr->Top;

            for (Count = Ptr->Vertices - 1, Vertex1 = Ptr->Top; Count--; Vertex1 = Vertex1->Next)
            {
                Vertex2 = Vertex1->Next;

                MulVectMatrix (&Temp1, Vertex1, MViewMatrix);
                MulVectMatrix (&Temp2, Vertex2, MViewMatrix);

                MulVectMatrix (&Temp1, &Temp1, ProjMatrix);
                MulVectMatrix (&Temp2, &Temp2, ProjMatrix);

                if (Temp1[3] <= 0 && Temp2[3] <= 0)
                    continue;

                ClipZ (&Temp1, &Temp2);

                CnvTo2D (&Temp1, &HalfRes, &StartPos);
                CnvTo2D (&Temp2, &HalfRes, &StartPos);

                c1 = GetOutCode (&Temp1);
                c2 = GetOutCode (&Temp2);

                if (c1 & c2) continue;

                ClipXY (&Temp1, &Temp2);

                c1 = GetOutCode (&Temp1);
                c2 = GetOutCode (&Temp2);

                if (c1 || c2) continue;

                DrawLine (x1, y1, x2, y2, Vertex1->Color);
            }
    }

    /**/
    private void DrawTri (gFloat *P, gFloat *Q, gFloat *R)
    {
        gInt c1, c2, c3;

            c1 = GetOutCode (P);
            c2 = GetOutCode (Q);
            c3 = GetOutCode (R);

            if (c1 || c2 || c3) return;

            DrawLine (P[0], P[1], Q[0], Q[1], -1);
            DrawLine (Q[0], Q[1], R[0], R[1], -1);
            DrawLine (R[0], R[1], P[0], P[1], -1);
    }

    /* Renders an object of type TRI_OBJECT. */
    private void RenderTriObject (gTriObject *Ptr)
    {
        gInt i, Count, c1, c2, c3;
        gVertexLCT *Vertex1, *Vertex2, *Vertex3;
        gFloat Temp[6][4];

            Vertex1 = Ptr->Top;

            for (Count = Ptr->Vertices / 3, Vertex1 = Ptr->Top; Count--; Vertex1 = Vertex3->Next)
            {
                Vertex3 = (Vertex2 = Vertex1->Next)->Next;

                MulVectMatrix (&Temp[0], Vertex1, MViewMatrix);
                MulVectMatrix (&Temp[1], Vertex2, MViewMatrix);
                MulVectMatrix (&Temp[2], Vertex3, MViewMatrix);

                MulVectMatrix (&Temp[0], &Temp[0], ProjMatrix);
                MulVectMatrix (&Temp[1], &Temp[1], ProjMatrix);
                MulVectMatrix (&Temp[2], &Temp[2], ProjMatrix);

                if (Temp[0][3] <= 0 && Temp[1][3] <= 0 && Temp[1][3] <= 0)
                    continue;

                ClipZ (&Temp[0], &Temp[1]);
                ClipZ (&Temp[1], &Temp[2]);
                ClipZ (&Temp[2], &Temp[0]);

                CnvTo2D (&Temp[0], &HalfRes, &StartPos);
                CnvTo2D (&Temp[1], &HalfRes, &StartPos);
                CnvTo2D (&Temp[2], &HalfRes, &StartPos);

                if (Temp[1][1] <Temp[0][1])
                {
                    CopyVector (&Temp[3], &Temp[1]);
                    CopyVector (&Temp[1], &Temp[0]);
                    CopyVector (&Temp[0], &Temp[3]);
                }

                if (Temp[2][1] < Temp[0][1])
                {
                    CopyVector (&Temp[3], &Temp[2]);
                    CopyVector (&Temp[2], &Temp[0]);
                    CopyVector (&Temp[0], &Temp[3]);
                }

                if (Temp[2][1] < Temp[1][1])
                {
                    CopyVector (&Temp[3], &Temp[1]);
                    CopyVector (&Temp[1], &Temp[2]);
                    CopyVector (&Temp[2], &Temp[3]);
                }

                c1 = GetOutCode (&Temp[0]);
                c2 = GetOutCode (&Temp[1]);
                c3 = GetOutCode (&Temp[2]);

#if DEBUG
                for (i = 0; i < 3; i++)
                {
                    printf ("Vx%u = %10f: Vy%u = %10f\n",
                        i, Temp[i][0], i, Temp[i][1]);
                }
#endif

                CopyVector (&Temp[3], &Temp[2]);
                CopyVector (&Temp[4], &Temp[2]);

                CopyVector (&Temp[2], &Temp[1]);
                CopyVector (&Temp[5], &Temp[0]);

//                if (!(c1 & c2))
                    ClipXY (&Temp[0], &Temp[1]);
//avRil
                ClipXY (&Temp[2], &Temp[3]);
                ClipXY (&Temp[4], &Temp[5]);

#if DEBUG
                for (i = 0; i < 6; i++)
                {
                    printf ("x%u = %10f: y%u = %10f\n",
                        i, Temp[i][0], i, Temp[i][1]);
                }
#endif

                DrawTri (&Temp[0], &Temp[1], &Temp[2]);
                DrawTri (&Temp[0], &Temp[2], &Temp[3]);
                DrawTri (&Temp[0], &Temp[3], &Temp[4]);
                DrawTri (&Temp[0], &Temp[4], &Temp[5]);
            }
    }

    /* Renders the given object. */
    void gRenderObject (gObject *Ptr)
    {
            if (!Ptr) return;

            switch (Ptr->Type)
            {
                case POINT_OBJECT:
                    RenderPointObject ((gPointObject *)Ptr);
                    break;

                case LINE_OBJECT:
                    RenderLineObject ((gLineObject *)Ptr);
                    break;

                case POLYLINE_OBJECT:
                    RenderPolylineObject ((gLineObject *)Ptr);
                    break;

//violet
                case TRI_OBJECT:
                    RenderTriObject ((gTriObject *)Ptr);
                    break;

                case QUAD_OBJECT:
                    break;

                case MESH_OBJECT:
                    break;

                case COMPOSITE_OBJECT:
                    break;
            }
    }

/*#*/
    /* Adds a vertex to the current object. */
    private void AddVertex (gFloat x, gFloat y, gFloat z, gFloat w)
    {
        void *Vertex;

            switch (Object->Type)
            {
                case POINT_OBJECT:
                    Vertex = gAllocC (sizeof (gVertexLC));

                    CopyVector (Vertex, &x);
                    ((gVertexLC *)Vertex)->Color = ((gPointObject *)Object)->Color;

                    if (((gPointObject *)Object)->Bottom)
                        ((gPointObject *)Object)->Bottom->Next = Vertex;
                    else
                        ((gPointObject *)Object)->Top = Vertex;

                    ((gPointObject *)Object)->Bottom = Vertex;
                    break;

                case LINE_OBJECT:
                case POLYLINE_OBJECT:
                    Vertex = gAllocC (sizeof (gVertexLC));

                    CopyVector (Vertex, &x);
                    ((gVertexLC *)Vertex)->Color = ((gLineObject *)Object)->Color;

                    if (((gLineObject *)Object)->Bottom)
                        ((gLineObject *)Object)->Bottom->Next = Vertex;
                    else
                        ((gLineObject *)Object)->Top = Vertex;

                    ((gLineObject *)Object)->Bottom = Vertex;
                    ((gLineObject *)Object)->Vertices++;
                    break;

                case TRI_OBJECT:
                    Vertex = gAllocC (sizeof (gVertexLCT));

                    CopyVector (Vertex, &x);
                    ((gVertexLCT *)Vertex)->Color = ((gTriObject *)Object)->Color;

                    if (((gTriObject *)Object)->Bottom)
                        ((gTriObject *)Object)->Bottom->Next = Vertex;
                    else
                        ((gTriObject *)Object)->Top = Vertex;

                    ((gTriObject *)Object)->Bottom = Vertex;
                    ((gTriObject *)Object)->Vertices++;
                    break;
//violet
                case QUAD_OBJECT:
                    break;

                case MESH_OBJECT:
                    break;

                case COMPOSITE_OBJECT:
                    break;
            }
    }

    /* Adds the vertex (x, y, 0, 1) to the current object. */
    void gVertex2f (gFloat x, gFloat y)
    {
            if (!Object) return;
            AddVertex (x, y, 0, 1);
    }

    /* Adds the vertex (x, y, 0, 1) to the current object. */
    void gVertex2v (gFloat *Ptr)
    {
            if (!Object) return;
            AddVertex (Ptr[0], Ptr[1], 0, 1);
    }

    /* Adds the vertex (x, y, z, 1) to the current object. */
    void gVertex3f (gFloat x, gFloat y, gFloat z)
    {
            if (!Object) return;
            AddVertex (x, y, z, 1);
    }

    /* Adds the vertex (x, y, z, 1) to the current object. */
    void gVertex3v (gFloat *Ptr)
    {
            if (!Object) return;
            AddVertex (Ptr[0], Ptr[1], Ptr[2], 1);
    }

/*#*/
    /* Sets the color for the following vertices. */
    void gVertexColor (gDword RGBA)
    {
            if (!Object) return;

            switch (Object->Type)
            {
                case POINT_OBJECT:
                    ((gPointObject *)Object)->Color = RGBA;
                    break;

                case LINE_OBJECT:
                case POLYLINE_OBJECT:
                    ((gLineObject *)Object)->Color = RGBA;
                    break;

                case TRI_OBJECT:
                    ((gTriObject *)Object)->Color = RGBA;
                    break;
//violet
                case QUAD_OBJECT:
                    break;

                case MESH_OBJECT:
                    break;

                case COMPOSITE_OBJECT:
                    break;
            }
    }
