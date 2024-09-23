'From: iowrath@aol.com (IO Wrath)
'Newsgroups: comp.lang.basic.misc
'Subject: 3Dmodel.BAS
'Date: 29 Jun 1997 22:30:46 GMT

DEFINT A-Z

DECLARE SUB SetPal ()
DECLARE SUB Tri (i%, c%)
DECLARE SUB QwkSort (l%, R%)
DECLARE SUB MainLoop ()
DECLARE SUB DrawObject ()

'A simple example of 3D graphics by Joshua Dickerson (C) SEPTEMBER 1996
                      
DIM SHARED rX!, ry!, rZ!, tx, ty, tZ, nPts, nPl
READ nPts, nPl
DIM SHARED World(nPts, 2), Plane(nPl, 2), Index(nPl), mp!(nPl), pX(nPts)
DIM SHARED pY(nPts), Z!(nPts), Dst, frame

FOR i = 0 TO nPts
   FOR j = 0 TO 2
      READ World(i, j)
   NEXT
NEXT

FOR i = 0 TO nPl
   FOR j = 0 TO 2
      READ Plane(i, j)
   NEXT
NEXT

Dst = 1100
tZ = -1270

ty = -70
tx = 90
rZ! = 2.8

SCREEN 9
i = INP(&H3DA)
FOR i = 0 TO 15
   OUT &H3C0, i
   OUT &H3C0, i
NEXT
OUT &H3C0, 32

SetPal

MainLoop
END

'DATA 16,17
'DATA 0,0,-100, 15,0,-20, -15,0,-20, 0,-5,0, -15,0,-50, 15,0,-50, 40,0,-30
'DATA -40,0,-30, 41,0,5, -41,0,5, 42,0,-30, -42,0,-30, 40,0,-80, -40,0,-80
'DATA 140,0,-25, -140, 0,-25, 0,-15,-20

'DATA 0,1,2, 1,2,3, 0,1,5, 0,2,4, 1,5,6, 2,4,7, 1,6,8, 2,7,9, 6,8,10, 6,10,12
'DATA 8,10,14, 7,9,11, 7,11,13, 9,11,15, 0,2,16, 0,1,16, 1,3,16, 2,3,16

'DATA 32,44
'DATA 0,-50,0, -23,-47,0, -34,-32,0, -37,-6,0, -33,10,0, -12,35,0, 12,35,0
'DATA 33,10,0, 37,-6,0, 37,-18,0, 34,-32,0, 23,-47,0, -23,-27,10, -23,-23,12
'DATA -10,-18,2, -6,-25,8, -5,-4,10, -10,2,5, -23,-13,10, -23,-17,12
'DATA -37,-18,0, 0,14,7, 0,18,5, 0,24,5, 0,29,7, 10,2,2, 23,-13,10, 10,-18,2
'DATA 6,-25,8, 23,-27,10, 23,-23,12, 23,-17,12, 5,-4,10
'
'DATA 1,2,12, 1,12,0, 12,15,0, 0,15,28, 15,16,28, 28,16,32, 0,29,28, 0,29,11
'DATA 10,29,9, 2,20,12, 12,14,15, 27,28,29, 10,11,29, 20,12,13, 12,13,14
'DATA 27,29,30, 29,30,9, 20,3,18, 20,19,18, 18,19,14, 27,26,31, 26,31,9, 8,9,26
'DATA 14,17,16, 14,16,15, 28,32,25, 25,27,28, 18,14,17, 27,26,25, 3,18,17
'DATA 17,4,3, 25,26,8, 25,8,7, 17,25,16, 16,32,25, 17,21,25, 17,21,4, 25,21,7
'DATA 4,21,22, 21,22,7, 4,23,24, 23,24,7, 4,5,24, 5,6,24, 24,6,7

'BacterioPhage.
DATA 108, 185
DATA 0,70,0,  17,50,30, -17,50,30, -35,50,0, -17,50,-30, 17,50,-30
DATA 35,50,0,  17,-10,30, -17,-10,30, -35,-10,0, -17,-10,-30, 17,-10,-30
DATA 35,-10,0,  3,-40,5, -3,-40,5, -6,-40,0, -3,-40,-5, 3,-40,-5
DATA 6,-40,0,  6,-40,10, -6,-40,10, -12,-40,0, -6,-40,-10, 6,-40,-10, 12,-40,0
DATA 6,-45,10, -6,-45,10, -12,-45,0, -6,-45,-10, 6,-45,-10, 12,-45,0
DATA 3,-45,5, -3,-45,5, -6,-45,0, -3,-45,-5, 3,-45,-5, 6,-45,0
DATA 3,-50,5, -3,-50,5, -6,-50,0, -3,-50,-5, 3,-50,-5, 6,-50,0
DATA 6,-50,10, -6,-50,10, -12,-50,0, -6,-50,-10, 6,-50,-10, 12,-50,0
DATA 6,-160,10, -6,-160,10, -12,-160,0, -6,-160,-10, 6,-160,-10, 12,-160,0
DATA 3,-160,5, -3,-160,5, -6,-160,0, -3,-160,-5, 3,-160,-5, 6,-160,0
DATA 3,-165,5, -3,-165,5, -6,-165,0, -3,-165,-5, 3,-165,-5, 6,-165,0
DATA 6,-165,10, -6,-165,10, -12,-165,0, -6,-165,-10, 6,-165,-10, 12,-165,0

DATA 6,-170,10, -6,-170,10, -12,-170,0, -6,-170,-10, 6,-170,-10, 12,-170,0
DATA 3,-170,5, -3,-170,5, -6,-170,0, -3,-170,-5, 3,-170,-5, 6,-170,0
DATA 3,-175,5, -3,-175,5, -6,-175,0, -3,-175,-5, 3,-175,-5, 6,-175,0
DATA 33,-155,50, 27,-155,53, -27,-155,54, -33,-155,50, -60,-155,4, -60,-155,-4
DATA -33,-155,-50, -27,-155,-54, 27,-155,-54, 33,-155,-50, 60,-155,-4,60,-155,4
DATA 40,-260,69, -40,-260,69, -80,-260,0, -40,-260,-69, 40,-260,-69, 80,-260,0

DATA 0,1,2, 2,0,3, 3,0,4, 4,0,5, 5,0,6, 6,0,1, 6,12,1, 12,7,1, 7,1,2, 7,8,2
DATA 8,9,3, 2,3,8, 3,9,4, 9,10,4, 10,11,4, 11,4,5, 5,11,6, 11,12,6, 7,8,13
DATA 13,14,8, 8,9,14, 14,15,9, 9,15,10, 15,16,10, 16,10,11, 16,17,11, 17,18,12
DATA 17,12,11, 7,18,13, 7,18,12, 13,19,20, 14,20,13, 20,14,21, 14,21,15
DATA 21,15,22, 15,22,16, 16,23,22, 16,17,23, 17,23,24, 17,18,24, 18,24,19
DATA 13,18,19, 19,25,26, 19,20,26, 20,26,27, 20,21,27, 21,27,28, 21,22,28
DATA 22,28,29, 22,23,29, 23,29,30, 23,24,30, 24,25,30, 19,24,25, 25,36,31
DATA 25,30,36, 25,26,31, 26,31,32, 26,27,32, 27,32,33, 27,28,33, 28,33,34
DATA 28,29,34, 34,35,29, 30,35,36, 29,30,35, 37,38,32, 37,31,32, 32,38,33
DATA 33,38,39, 33,34,39, 39,34,40, 34,40,35, 40,41,35, 35,36,41, 41,42,36
DATA 31,37,42, 42,31,36, 37,43,38, 43,44,38, 38,44,39, 44,45,39, 45,39,40
DATA 45,40,46, 40,46,41, 46,47,41, 41,47,42, 47,42,48, 48,42,37, 37,43,48
DATA 49,43,50, 43,44,50, 44,50,51, 51,44,45, 45,51,52, 45,46,52, 46,52,53
DATA 46,47,53, 47,48,54, 47,54,53, 43,49,48, 48,49,54, 55,49,56, 49,50,56
DATA 56,57,50, 50,51,57, 51,57,58, 51,52,58, 52,58,59, 52,53,59, 53,59,60
DATA 53,54,60, 49,55,54, 55,54,60, 55,56,61, 61,62,56, 56,57,62, 57,62,63
DATA 57,63,58, 63,64,58, 58,59,64, 59,64,65, 65,59,60, 65,66,60, 66,60,55
DATA 66,55,61, 61,67,68, 61,62,68, 68,62,69, 62,63,69, 69,63,70, 63,64,70
DATA 64,70,71, 64,65,71, 71,65,72, 66,65,72, 66,72,67, 61,66,67, 67,68,73
DATA 68,73,74, 68,74,69, 69,74,75, 69,70,75, 70,75,76, 76,70,71, 71,76,77
DATA 71,77,72, 77,78,72, 72,78,77, 67,73,78, 73,79,74, 74,80,79, 74,75,80
DATA 75,80,81, 76,81,75, 81,82,76, 82,83,77, 82,76,77, 83,84,78, 83,77,78
DATA 73,79,84, 73,78,84, 85,79,86, 79,80,86, 86,87,80, 80,81,87, 81,87,88
DATA 81,88,82, 82,88,89, 82,83,89, 83,89,90, 83,84,90, 84,90,85, 85,84,79
DATA 91,92,73, 93,94,74, 95,96,75, 97,98,76, 99,100,77, 101,102,78, 91,92,103
DATA 93,94,104, 95,96,105, 97,98,106, 99,100,107, 101,102,108

SUB DrawObject
   'I will clean up any inaccuracies, shortcuts, and inefficiencies
   'in my third release...
  
   CrX! = COS(rX!)
   SrX! = SIN(rX!)
   CrY! = COS(ry!)
   SrY! = SIN(ry!)
   CrZ! = COS(rZ!)
   SrZ! = SIN(rZ!)
  
   'Pre-Construct a table of rotated/translated/projected points.
   FOR i = 0 TO nPts
      X! = CrX! * -World(i, 0) - SrX! * World(i, 2)
      Z! = SrX! * -World(i, 0) + CrX! * World(i, 2)
      y! = CrY! * World(i, 1) - SrY! * X!
      Z!(i) = (CrZ! * Z! - SrZ! * y!) + tZ
      pX(i) = (Dst * ((CrY! * X! + SrY! * World(i, 1)) + tx) / Z!(i)) + 320
      pY(i) = 175 - (Dst * ((SrZ! * Z! + CrZ! * y!) + ty) / Z!(i))
   NEXT

   'This provides the Z midpoint for a primative hidden surface removal
   'routine.

   FOR i = 0 TO nPl
      mp!(i) = (Z!(Plane(i, 0)) + Z!(Plane(i, 1)) + Z!(Plane(i, 2))) / 3
      Index(i) = i
   NEXT
  
   QwkSort 0, nPl

   FOR i = 0 TO nPl
      Tri i, (14 * (i / nPl)) + 1
      '      ^^^^^^^^^^^^^^^^^^^^
      'A novel shading algorythm I came up with. This is not entirely
      'accurate, but I thought it would provide an adaquate illusion.
   NEXT
END SUB

DEFSNG A-Z
SUB MainLoop
   DO
      rX! = rX! + .1
      IF rX! > 6.28 THEN rX! = 0
      Page = 1 - Page
      SCREEN 9, 1, 1 - Page, Page

      WAIT 986, 8
      LINE (0, 0)-(639, 349), 0, BF
 
      DrawObject
   LOOP UNTIL INKEY$ <> ""
SCREEN 0
END SUB

DEFINT A-Z
SUB QwkSort (l, R)
i = l
j = R
X! = mp!(Index((l + R) \ 2))

DO
   DO WHILE mp!(Index(i)) < X!
      i = i + 1
    LOOP
   DO WHILE X! < mp!(Index(j))
      j = j - 1
   LOOP
   IF i <= j THEN
      SWAP Index(i), Index(j)
      i = i + 1
      j = j - 1
   END IF
LOOP UNTIL i > j


IF l < j THEN QwkSort (l), (j)
IF i < R THEN QwkSort (i), (R)

END SUB

SUB SetPal
FOR i = 0 TO 15
   OUT &H3C8, i
   OUT &H3C9, i * 4
   OUT &H3C9, i * 4
   OUT &H3C9, i * 4
NEXT

END SUB

SUB Tri (i, c)
   DIM xpos(-100 TO 449, 1)
  
   x1 = pX(Plane(Index(i), 0))
   y1 = pY(Plane(Index(i), 0))
   x2 = pX(Plane(Index(i), 1))
   y2 = pY(Plane(Index(i), 1))
   x3 = pX(Plane(Index(i), 2))
   y3 = pY(Plane(Index(i), 2))

mny = y1
IF y2 < mny THEN mny = y2
IF y3 < mny THEN mny = y3
mxy = y1
IF y2 > mxy THEN mxy = y2
IF y3 > mxy THEN mxy = y3
s1 = ABS(y1 < y2) * 2 - 1
s2 = ABS(y2 < y3) * 2 - 1
s3 = ABS(y3 < y1) * 2 - 1

y = y1
IF y1 <> y2 THEN
   DO
      xpos(y, ABS(y1 < y2)) = (x2 - x1) * (y - y1) \ (y2 - y1) + x1
      y = y + s1
   LOOP UNTIL y = y2 + s1
ELSE
   xpos(y, ABS(y1 < y2)) = x1
END IF
y = y2
IF y2 <> y3 THEN
   DO
      xpos(y, ABS(y2 < y3)) = (x3 - x2) * (y - y2) \ (y3 - y2) + x2
      y = y + s2
   LOOP UNTIL y = y3 + s2
ELSE
   xpos(y, ABS(y2 < y3)) = x2
END IF
y = y3
IF y3 <> y1 THEN
   DO
      xpos(y, ABS(y3 < y1)) = (x1 - x3) * (y - y3) \ (y1 - y3) + x3
      y = y + s3
   LOOP UNTIL y = y1 + s3
ELSE
   xpos(y, ABS(y3 < y1)) = x3
END IF

'Polygon is broken up into horizontal lines (No painting nessesary this way)
FOR y = mny TO mxy
   IF xpos(y, 0) > xpos(y, 1) THEN SWAP xpos(y, 0), xpos(y, 1)
   IF xpos(y, 0) > 0 THEN LINE (xpos(y, 0), y)-(xpos(y, 1), y), c
NEXT
LINE (x1, y1)-(x2, y2), c - 1
LINE -(x3, y3), c - 1
LINE -(x1, y1), c - 1
END SUB
