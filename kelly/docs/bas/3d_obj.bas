'===========================================================================
' 3D OBJECTS 1.0 by Daniel Sanchez            Email: Daniel80@pacbell.net
'===========================================================================
'
' 3D Objects makes Buttons, Text Boxes, and List Boxes
' Check boxes, Radio buttons, and other objects will be in version 2,
' i kind of ran out of time for now :)
'
'
' If anyone out there makes improvements, please send me a copy thanxs !!
' Also, i didn't have time to Comment the program, but any novice qbasic
' programmer can figure the code out.
'
'
'
' CREDITS (thanks dudes)
'==========================================
'Fonts By ---------------- Bobby K (Krusty)
'Mouse Routines by ------- Kevin Wolfe
'
' Ok, on to the Explaining
'
'=======================================================
'            Creating A Button
'=======================================================
' Call procedure  mButton (nam, x, y, lenght, label$)
'
'   nam = Button ID -- Used to refer to a certain Button (number)
'     x = X position of button
'     y = Y position of button
'lenght = Lenght of Button (graphic-wise)
'label$ = Label of Button
'
'Note: The Variable ButtonFocus Contains The button pushed
'
'EXAMPLE: To see if button 1 is pressed:
'
'IF ButtonFocus = 1 THEN
'END IF
'
'
'
'=======================================================
'            Creating A Text Box
'=======================================================
' Call procedure  mTextBox (nam, x, y, lenght, label$)
'
'   nam = Text Box ID -- Used to refer to a certain Text Box (number)
'     x = X position of Text Box
'     y = Y position of Text Box
'lenght = Lenght of Text Box (graphic-wise)
'label$ = Label of Text Box
'
'Note: The Array TextBoxResult$ Contains The user's input
'
'EXAMPLE: To get and print the information on text box 1:
'
'Result$ = TextBoxResult$(1)
'PRINT Result$
'
'
'=======================================================
'            Creating A List Box
'=======================================================
' Call procedure  mListBox (nam, x, y, xlen, ylen, label$, num)
'
'   nam = List Box ID -- Used to refer to a certain List Box (number)
'     x = X position of List Box
'     y = Y position of List Box
'  xlen = Horizontal Lenght of List Box
'  ylen = Vertical Lenght of List Box
'label$ = Label of List Box
'
'Note: - The Array ListBoxContent$ Contains The List Box's Contents to Display
'      - ListBoxConten$(LB, ITEM), LB = List Box Number, ITEM = Item Number
'      - The Array SelectedLine Contains the selected line of The List Box
'           
'EXAMPLE: To Put 4 Items into List Box 1 (there is many ways it can be done)
'
'FOR xx = 1 TO 4
' READ Value
' ListBoxContent$(1, xx) = Value
'NEXT XX
'
'DATA "Item 1", "Item 2", "Item 3", "Item 4"
'
'
'
'
'===========================  BUGS =============
'
'  - Poor Text Box Max Character Detection
'  - Mouse Gets in the way of drawing (I really couln't fix it, i redrawed
'                                     them when mouse was out instead)
'
'
'  - No Text Box Cursor
'  - List Boxes can place items out of the Box's bounderies
'  - When using arrow keys on text box, it prints " M " and " K "
' 
' Most Of these bugs will be fixed in Version 2, and I'm probably, after
' i'm finished with version 2, will try to make a Visual Basic - Type of
' program, were you can move objects around with the mouse(no coding)
' and have them saved as .bas. Any questions, ideas, suggestions, comments,
' or anything email me at Daniel80@pacbell.net









DECLARE SUB DrawCheckedBox (posi%)
DECLARE SUB SetListBoxFocus (posi%, prev%)
DECLARE SUB CheckListBox ()
DEFINT A-Z
DECLARE SUB fprint (Text$, textx%, texty%, colour%, file%)
DECLARE SUB fopen (file$, file%)
DECLARE SUB redraw ()

'Subs Used by Text Boxes
'-------------------------
DECLARE SUB DrawTextBox (position%)
DECLARE SUB mTextBox (nam, x, y, lenght, label$)
DECLARE SUB PutCharOnBox (letter$)
DECLARE SUB PrintLabel (TextBoxNum, clr)

'Subs Used by Buttons
'-------------------------
DECLARE SUB DrawButton (position)
DECLARE SUB mButton (nam, x, y, lenght, label$)
DECLARE SUB PrintButtonLabel (ButtonNum, clr)
DECLARE SUB DrawButtonDown (position)

'Subs Used by List Boxes
'-------------------------
DECLARE SUB DrawListBox (position)
DECLARE SUB mListBox (nam, x, y, xlen, ylen, label$)
DECLARE SUB UpdateListBox (position, top, way)



DECLARE SUB SetNewFocus ()
DECLARE SUB PlayWarning ()

'=============== MOUSE STUFF ==============

DECLARE SUB Mouse (which)
DIM SHARED mouser$, MouseLeftButton, MouseRightButton, MousexPos, MouseyPos, AX%, bx%, cx%, dx%
Mouse 0

DATA 55,89,E5,8B,5E,0C,8B,07,50,8B,5E,0A,8B,07,50,8B
DATA 5E,08,8B,0F,8B,5E,06,8B,17,5B,58,1E,07,CD,33,53
DATA 8B,5E,0C,89,07,58,8B,5E,0A,89,07,8B,5E,08,89,0F
DATA 8B,5E,06,89,17,5D,CA,08,00

' ==========================================






TYPE TypeTextBox
 nam AS INTEGER
 x AS INTEGER
 y AS INTEGER
 lenght AS INTEGER
 label AS STRING * 20
 maxtext AS INTEGER
 readtext AS STRING * 100
END TYPE

TYPE TypeButton
 nam AS INTEGER
 x AS INTEGER
 y AS INTEGER
 lenght AS INTEGER
END TYPE

TYPE TypeListBox
 nam AS INTEGER
 x AS INTEGER
 xlen AS INTEGER
 y AS INTEGER
 ylen AS INTEGER
 show AS INTEGER
 top AS INTEGER
 selected AS INTEGER
 NumItems AS INTEGER
 scrollspeed AS INTEGER         'Bigger the slower
 speedcounter AS INTEGER
 redraw AS INTEGER
END TYPE

DIM SHARED ObjectFocus

DIM SHARED MaxTextBoxes, ActiveTextBoxes, TextBoxFocus
MaxTextBoxes = 10
DIM SHARED TextBoxResult$(MaxTextBoxes)
DIM SHARED TextBox(MaxTextBoxes) AS TypeTextBox


DIM SHARED MaxButtons, ActiveButtons, ButtonFocus
MaxButtons = 10
DIM SHARED Button(MaxButtons) AS TypeButton, ButtonLabel$(MaxButtons)

DIM SHARED MaxListBoxes, MaxListItems, ActiveListBoxes, ListBoxFocus, ListBoxScrollingSpeed
MaxListBoxes = 5
MaxListItems = 20
ListBoxScrollingSpeed = 300    'The Bigger the Slower the List Boxes will scroll,
                               'the smaller, well, like a flash.
DIM SHARED ListBox(MaxListBoxes) AS TypeListBox, ListBoxContent$(MaxListBoxes, MaxListItems), ListBoxLabel$(MaxListBoxes), SelectedLine(MaxListBoxes)



SCREEN 12
COLOR 1
PAINT (1, 1), 9, 9

fopen "C:\DOS\WINFONTS\font2b", 1
fopen "C:\DOS\WINFONTS\font3.qbf", 2


TextBoxFocus = 1
ButtonFocus = 0
ListBoxFocus = 1





'=========== CREATE YOUR 3D OBJECTS HERE ============

mButton 1, 10, 50, 80, "3d Button"
mTextBox 1, 150, 50, 100, "Text Box"
mTextBox 2, 300, 50, 100, "Text Box 2"
mTextBox 3, 450, 50, 100, "Text Box 3"

' Fill List Box 1
ListBox(1).NumItems = 20
ListBox(1).selected = ListBoxFocus
FOR ss = 1 TO 20
 READ one$
 ListBoxContent$(1, ss) = one$
NEXT ss
mListBox 1, 100, 200, 100, 100, "List Box"

SetListBoxFocus 1, ListBoxFocus




' Start Main Loop
'================
DO
 DO
  Mouse 3
  Mouse 5
  result$ = INKEY$
  redraw
 LOOP UNTIL result$ <> "" OR MouseLeftButton = -1

 IF MouseLeftButton = -1 THEN
  SetNewFocus
  CheckListBox
  
  '================= Check 3D Objects =================

  IF ButtonFocus = 1 THEN
   IF SelectedLine(1) = 5 THEN
    BEEP
   END IF
  END IF





 END IF
 SELECT CASE result$
  CASE CHR$(9): SetNewFocus
  CASE "q": SYSTEM
  CASE ELSE: PutCharOnBox (result$)    'Puts A Character into a Text box
 END SELECT
 ButtonFocus = 0   'resets value
LOOP



CLOSE #1
CLOSE #2


'Data For List Box 1
'-------------------
DATA "File1", "File2", "File3", "File4", "File5", "File6", "File7", "File8", "File9", "File10", "File1", "File2", "File3", "File4", "File5", "File6", "File7", "File8", "File9", "File10"

SUB CheckListBox

FOR xx = 1 TO ActiveListBoxes
 x = ListBox(xx).x + 2
 y = ListBox(xx).y + 3
 xlen = x + ListBox(xx).xlen - 2
 ylen = y + ListBox(xx).ylen - 2


 x1 = ListBox(xx).x + ListBox(xx).xlen
 y1 = ListBox(xx).y
 Xmax1 = x1 + 10
 Ymax1 = y1 + 15

 x2 = ListBox(xx).x + ListBox(xx).xlen
 y2 = ListBox(xx).y + ListBox(xx).ylen
 Xmax2 = ListBox(xx).x + x2
 Ymax2 = ListBox(xx).y + ListBox(xx).ylen - 15

 top = ListBox(xx).top
 scrollspeed = ListBox(xx).scrollspeed


 IF (MousexPos >= x1) AND (MousexPos <= Xmax1) AND (MouseyPos >= y1) AND (MouseyPos <= y2) THEN
  IF (MousexPos >= x1) AND (MousexPos <= Xmax1) AND (MouseyPos >= y1) AND (MouseyPos <= Ymax1) THEN
   IF ListBox(xx).speedcounter = scrollspeed THEN
    ListBox(xx).speedcounter = 0
    UpdateListBox xx, top, 1
    
    EXIT FOR
   ELSE
    ListBox(xx).speedcounter = ListBox(xx).speedcounter + 1
   END IF
  ELSE
   IF (MousexPos >= x2) AND (MousexPos <= Xmax2) AND (MouseyPos <= y2) AND (MouseyPos >= Ymax2) THEN
    IF ListBox(xx).speedcounter = scrollspeed THEN
     ListBox(xx).speedcounter = 0
     UpdateListBox xx, top, 0
    ELSE
     ListBox(xx).speedcounter = ListBox(xx).speedcounter + 1
    END IF
    EXIT SUB
   END IF
  END IF
 ELSE
  IF (MousexPos >= x) AND (MousexPos <= xlen) AND (MouseyPos >= y) AND (MouseyPos <= ylen) THEN
   IF ListBox(xx).speedcounter = scrollspeed THEN
    yposition = (MouseyPos - y)
    prev = ListBox(xx).selected
    ListBox(xx).selected = INT(yposition \ 10) + 1
    ListBox(xx).redraw = 1
    SetListBoxFocus xx, prev
   ELSE
    ListBox(xx).speedcounter = ListBox(xx).speedcounter + 1
   END IF
  END IF
 END IF
NEXT xx




END SUB

SUB DrawButton (posi)

x = Button(posi).x
y = Button(posi).y
lenght = Button(posi).lenght
Text$ = ButtonLabel$(posi)

LINE (x, y)-(x + lenght, y + 17), 9, BF
LINE (x, y)-(x + lenght, y + 17), 0, B
PAINT (x + 1, y + 1), 7, 0

LINE (x, y)-(x + lenght - 1, y), 15
LINE (x, y)-(x, y + 16), 15

lenght = INT(lenght / 2)
Middle = INT(((LEN(Text$))) * 6.7)
Middle = INT(Middle / 2)
position = lenght - Middle


fprint Text$, x + position, y + 3, 0, 2

END SUB

SUB DrawButtonDown (position)

posi = position
x = Button(posi).x
y = Button(posi).y
lenght = Button(posi).lenght
Text$ = ButtonLabel$(posi)

LINE (x, y)-(x + lenght, y + 17), 9, BF
LINE (x, y)-(x + lenght - 1, y + 16), 0, B
PAINT (x + 2, y + 2), 7, 0

lenght = INT(lenght / 2)
Middle = INT(((LEN(Text$))) * 6.7)
Middle = INT(Middle / 2)
Positionn = lenght - Middle


fprint Text$, x + Positionn + 2, y + 4, 0, 2




END SUB

SUB DrawListBox (posi)


x = ListBox(posi).x
y = ListBox(posi).y
xlen = ListBox(posi).xlen
ylen = ListBox(posi).ylen
Text$ = ListBoxLabel$(posi)
NumItems = ListBox(posi).NumItems
show = ListBox(posi).show
selected = ListBox(posi).selected
top = ListBox(posi).top


LINE (x, y)-(x + xlen, y + ylen), 0, B
LINE (x + 1, y + 1)-(x + xlen - 1, y + ylen - 1), 0, B

PAINT (x + 2, y + 2), 15, 0
LINE (x, y + ylen)-(x + xlen, y + ylen), 7
LINE (x + 1, y + ylen - 1)-(x + xlen, y + ylen - 1), 7

LINE (x + xlen, y)-(x + xlen, y + ylen), 7
LINE (x + xlen - 1, y + 1)-(x + xlen - 1, y + ylen), 7



fprint Text$, x + 2, y - 10, 0, 2


'Draw Arrow Buttons and Stuff
LINE (x + xlen + 1, y)-(x + xlen + 13, y + ylen), 8, BF

LINE (x + xlen + 5, y + 10)-(x + xlen + 8, y + 15), 7, BF
LINE (x + xlen + 2, y + 10)-(x + xlen + 7, y + 5), 7
LINE (x + xlen + 12, y + 10)-(x + xlen + 7, y + 5), 7
LINE (x + xlen + 2, y + 10)-(x + xlen + 12, y + 10), 7

PAINT (x + xlen + 7, y + 7), 7, 7


LINE (x + xlen + 5, y + (ylen - 10))-(x + xlen + 8, y + (ylen - 15)), 7, BF
LINE (x + xlen + 2, y + (ylen - 10))-(x + xlen + 7, y + (ylen - 5)), 7
LINE (x + xlen + 12, y + (ylen - 10))-(x + xlen + 7, y + (ylen - 5)), 7
LINE (x + xlen + 2, y + (ylen - 10))-(x + xlen + 12, y + (ylen - 10)), 7

PAINT (x + xlen + 7, y + (ylen - 7)), 7, 7




'Print Items On List Box

row = y
row = row + 3
IF top = 1 THEN
 FOR ss = 1 TO NumItems
  fprint ListBoxContent$(posi, ss), x + 5, row, 0, 2
  row = row + 10
  IF show = ss THEN EXIT FOR
 NEXT ss
ELSE
 
 FOR ss = top TO NumItems
  fprint ListBoxContent$(posi, ss), x + 5, row, 0, 2
  row = row + 10
  IF show = ss THEN EXIT FOR
 NEXT ss
END IF



END SUB

SUB DrawTextBox (position)


x = TextBox(position).x
y = TextBox(position).y
lenght = TextBox(position).lenght
Text$ = TextBox(position).label



LINE (x, y)-(x + lenght, y + 17), 8, B
LINE (x + 1, y + 1)-(x + lenght - 1, y + 16), 8, B
PAINT (x + 2, y + 2), 15, 8

LINE (x + 1, y + 16)-(x + lenght - 1, y + 16), 7
LINE (x, y + 17)-(x + lenght, y + 17), 7

LINE (x + lenght - 1, y + 1)-(x + lenght - 1, y + 16), 7
LINE (x + lenght, y)-(x + lenght, y + 17), 7


PrintLabel position, 0





END SUB

SUB fopen (file$, file%)

  OPEN file$ FOR RANDOM AS file% LEN = 2
  'Just to make it easier for those of you who dont
  'understand the OPEN command yet! ;)

END SUB

SUB fprint (Text$, textx%, texty%, colour%, file%)

  'lpi: lines per integer
  'fws: font word spacing
  'fls: font letter spacing
  'p% : pointer
  GET file%, 1, lpi%
  GET file%, 2, fws%
  GET file%, 3, fls%
  FOR count% = 1 TO LEN(Text$)
    m% = ASC(MID$(Text$, count%, 1)) - 29
    IF m% > 3 THEN
      GET file%, m%, a1%
      GET file%, m% + 1, a2%
      FOR n% = a1% TO a2% - 1 STEP lpi%
        FOR z% = 0 TO lpi% - 1
          GET file%, n% + z%, L%
           LINE (p% + textx%, (16 * z%) + texty%)-(p% + textx%, (16 * z%) + 15 + texty%), colour%, , L%
        NEXT z%
        p% = p% + 1
      NEXT n%
      p% = p% + fls%
    ELSE
      p% = p% + fws%
    END IF
  NEXT count%




END SUB

SUB mButton (nam, x, y, lenght, label$)

pointer = 1
DO
 IF Button(pointer).nam <> 0 THEN
  pointer = pointer + 1
 END IF
LOOP UNTIL Button(pointer).nam = 0

ActiveButtons = ActiveButtons + 1
Button(pointer).nam = nam
Button(pointer).x = x
Button(pointer).y = y
Button(pointer).lenght = lenght
ButtonLabel$(pointer) = label$
DrawButton pointer



END SUB

SUB mListBox (nam, x, y, xlen, ylen, label$)


pointer = 1
DO
 IF ListBox(pointer).nam <> 0 THEN
  pointer = pointer + 1
 END IF
LOOP UNTIL ListBox(pointer).nam = 0

ActiveListBoxes = ActiveListBoxes + 1
ListBox(pointer).nam = nam
ListBox(pointer).x = x
ListBox(pointer).y = y
ListBox(pointer).xlen = xlen
ListBox(pointer).ylen = ylen
ListBox(pointer).show = (INT(ylen \ 10)) - 1
ListBox(pointer).top = 1
ListBox(pointer).scrollspeed = ListBoxScrollingSpeed
ListBox(pointer).speedcounter = 0
ListBoxLabel$(pointer) = label$
ListBox(pointer).redraw = 0
SelectedLine(pointer) = 1
DrawListBox pointer





END SUB

SUB Mouse (which)
'0 = Mouse Init
'1 = Mouse Driver
'2 = Mouse Put
'3 = Mouse Status
'4 = Mouse Hide
'5 = Mouse Show
IF which = 0 THEN
  mouser$ = SPACE$(57)
  FOR I% = 1 TO 57
    READ A$
    H$ = CHR$(VAL("&H" + A$))
    MID$(mouser$, I%, 1) = H$
  NEXT I%
  AX% = 0
  bx% = 0
  cx% = 0
  dx% = 0
  Mouse 1
  MouseInit% = AX%
  Mouse 5
END IF
IF which = 1 THEN
  DEF SEG = VARSEG(mouser$)
  mouser% = SADD(mouser$)
  CALL Absolute(AX%, bx%, cx%, dx%, mouser%)
END IF
IF which = 2 THEN
  AX% = 4
  cx% = x%
  dx% = y%
  bx% = 0
  Mouse 1
END IF
IF which = 3 THEN
  AX% = 3
  Mouse 1
  MouseLeftButton = ((bx% AND 1) <> 0)
  MouseRightButton = ((bx% AND 2) <> 0)
  MousexPos = cx%
  MouseyPos = dx%
END IF
IF which = 4 THEN
 AX% = 2
 bx% = 0
 cx% = 0
 dx% = 0
 Mouse 1
END IF
IF which = 5 THEN
  AX% = 1
  bx% = 0
  cx% = 0
  dx% = 0
  Mouse 1
END IF



END SUB

SUB mTextBox (nam, x, y, lenght, label$)

pointer = 1
DO
 IF TextBox(pointer).nam <> 0 THEN
  pointer = pointer + 1
 END IF
LOOP UNTIL TextBox(pointer).nam = 0

ActiveTextBoxes = ActiveTextBoxes + 1
TextBox(pointer).nam = nam
TextBox(pointer).x = x
TextBox(pointer).y = y
TextBox(pointer).lenght = lenght
TextBox(pointer).label = label$
TextBox(pointer).maxtext = INT(lenght / 8)
DrawTextBox pointer


END SUB

SUB PlayWarning


SOUND 600, .05
SOUND 700, .05


END SUB

SUB PrintButtonLabel (ButtonNum, clr)


x = Button(posi).x
y = Button(posi).y
lenght = Button(posi).lenght
Text$ = ButtonLabel$(posi)

lenght = INT(lenght / 2)
Middle = INT(((LEN(Text$))) * 6.7)
Middle = INT(Middle / 2)
position = lenght - Middle


fprint Text$, x + position, y + 3, clr, 2


END SUB

SUB PrintLabel (TextBoxNum, clr)

IF ActiveTextBoxes <> 0 THEN
 x = TextBoxNum
 Text$ = TextBox(x).label
 fprint Text$, TextBox(x).x + 2, TextBox(x).y - 10, clr, 2
END IF

END SUB

SUB PutCharOnBox (result$)

IF ActiveTextBoxes <> 0 THEN
 Text$ = TextBoxResult$(TextBoxFocus)
 L = LEN(Text$)


 IF CHR$(8) = result$ THEN
  IF L = 0 THEN
  ELSE
   fprint Text$, TextBox(TextBoxFocus).x + 4, TextBox(TextBoxFocus).y, 15, 1
   Text$ = LEFT$(Text$, L - 1)
   fprint Text$, TextBox(TextBoxFocus).x + 4, TextBox(TextBoxFocus).y, 0, 1
  END IF
 ELSE
  IF LEN(Text$) >= TextBox(TextBoxFocus).maxtext THEN
   PlayWarning
  ELSE
   Text$ = Text$ + result$
   TextBoxResult$(TextBoxFocus) = Text$
   fprint Text$, TextBox(TextBoxFocus).x + 4, TextBox(TextBoxFocus).y, 0, 1
  END IF
 END IF

 TextBoxResult$(TextBoxFocus) = Text$
END IF

END SUB

SUB redraw


FOR xx = 1 TO ActiveListBoxes
 x = ListBox(xx).x
 y = ListBox(xx).y
 x2 = x + ListBox(xx).xlen + 13
 y2 = y + ListBox(xx).ylen
 draww = ListBox(xx).redraw
 prev = ListBox(xx).selected

 IF draww = 1 THEN
  IF (MousexPos >= x2) OR (MousexPos <= x) OR (MouseyPos >= y2) OR (MouseyPos <= y) THEN
   ListBox(xx).redraw = 0
   DrawListBox xx
   SetListBoxFocus xx, prev
  END IF
 END IF
NEXT xx
END SUB

SUB SetListBoxFocus (posi, prev)

row = ListBox(posi).y + 3
x = ListBox(posi).x
xlen = x + ListBox(posi).xlen
y = ListBox(posi).y
top = ListBox(posi).top
selected = ListBox(posi).selected
old$ = ListBoxContent$(posi, prev + top - 1)
new$ = ListBoxContent$(posi, selected + top - 1)
NumItems = ListBox(posi).NumItems
SelectedLine(posi) = (selected + top - 1)


IF prev = 1 THEN
 LINE (x + 2, row + 2)-(xlen - 2, row + 9), 15, BF
 fprint old$, x + 5, row, 0, 2
ELSE
 FOR ss = 1 TO NumItems
  IF ss = prev THEN
   LINE (x + 2, row + 2)-(xlen - 2, row + 9), 15, BF
   fprint old$, x + 5, row, 0, 2
  ELSE
   row = row + 10
  END IF
 NEXT ss
END IF


row = ListBox(posi).y + 3
IF selected = 1 THEN
 LINE (x + 2, row + 2)-(xlen - 2, row + 9), 1, BF
 fprint old$, x + 5, row, 15, 2
ELSE
 FOR ss = 1 TO NumItems
  IF ss = selected THEN
   LINE (x + 2, row + 2)-(xlen - 2, row + 9), 1, BF
   fprint old$, x + 5, row, 15, 2
  ELSE
   row = row + 10
  END IF
 NEXT ss
END IF





END SUB

SUB SetNewFocus



 IF MouseLeftButton = -1 THEN
  FOR xx = 1 TO ActiveTextBoxes
   IF (MousexPos >= TextBox(xx).x) AND (MousexPos <= TextBox(xx).x + TextBox(xx).lenght) AND (MouseyPos >= TextBox(xx).y) AND (MouseyPos <= TextBox(xx).y + 17) THEN
    PrintLabel TextBoxFocus, 0
    TextBoxFocus = xx
    PrintLabel TextBoxFocus, 15
   END IF
  NEXT xx
 ELSE
  IF TextBoxFocus = ActiveTextBoxes THEN
   PrintLabel TextBoxFocus, 0
   TextBoxFocus = 1
   PrintLabel TextBoxFocus, 15
  ELSE
   PrintLabel TextBoxFocus, 0
   TextBoxFocus = TextBoxFocus + 1
   PrintLabel TextBoxFocus, 15
  END IF
 END IF


 IF MouseLeftButton = -1 THEN
  FOR xx = 1 TO ActiveButtons
   IF (MousexPos >= Button(xx).x) AND (MousexPos <= Button(xx).x + Button(xx).lenght) AND (MouseyPos >= Button(xx).y) AND (MouseyPos <= Button(xx).y + 17) THEN
    DrawButtonDown xx
    ButtonFocus = xx
    DO WHILE MouseLeftButton = -1 AND (MousexPos >= Button(xx).x) AND (MousexPos <= Button(xx).x + Button(xx).lenght) AND (MouseyPos >= Button(xx).y) AND (MouseyPos <= Button(xx).y + 17)
     Mouse 3
    LOOP
    DrawButton xx
   END IF
  NEXT xx
 END IF

END SUB

SUB UpdateListBox (posi, top, way)

x = ListBox(posi).x
y = ListBox(posi).y
xlen = ListBox(posi).xlen
ylen = ListBox(posi).ylen
Text$ = ListBoxLabel$(posi)
NumItems = ListBox(posi).NumItems
show = ListBox(posi).show
top = ListBox(posi).top
prev = ListBox(posi).selected

IF way = 0 THEN
 IF ((top + show) - 1) = NumItems THEN
  PlayWarning
 ELSE
  top = top + 1
  LINE (x + 2, y + 2)-(x + xlen - 2, y + ylen - 2), 15, BF
  row = y
  row = row + 3
  ctr = 0
  FOR ff = 1 TO show
   fprint ListBoxContent$(posi, top + ctr), x + 5, row, 0, 2
   ctr = ctr + 1
   row = row + 10
  NEXT ff
 END IF
 ListBox(posi).top = top
 SetListBoxFocus posi, prev
 EXIT SUB
END IF

IF way = 1 THEN
 IF top = 1 THEN
  PlayWarning
 ELSE
  top = top - 1
  LINE (x + 2, y + 2)-(x + xlen - 2, y + ylen - 2), 15, BF
  row = y
  row = row + 3
  ctr = 0
  FOR ff = 1 TO show
   fprint ListBoxContent$(posi, top + ctr), x + 5, row, 0, 2
   ctr = ctr + 1
   row = row + 10
  NEXT ff
 END IF
 ListBox(posi).top = top
 SetListBoxFocus posi, prev
END IF

END SUB

