/// +------------------------+
/// | Copyright (c) 2013 KVD |
/// |  All Rights Reserved.  |
/// |     Version 1.6.0      |
/// |  Internal: Version 19  |
/// +------------------------+

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define WINVER 0x0501
#include <signal.h>
#include <windows.h>

#define UP 0
#define DN 1
#define LF 2
#define RG 3

#define Unset   0
#define Black   0
#define White   1
#define Red     2
#define Green   3
#define Blue    4
#define Yellow  5
#define Cyan    6
#define Magenta 7

#define True    1
#define False   0

#define Read   -1
#define UnRead  1

#define Intro   0
#define GameMap 1

#define SP system("pause");
#define CP printf("CheckPoint\n");

BOOL WINAPI GetCurrentConsoleFont (HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFO lpConsoleCurrentFont);
COORD WINAPI GetConsoleFontSize (HANDLE hConsoleOutput, DWORD nFont);

void Envir_Error      (void);
void Initialization   (HDC DeviceContext, char *Argv);
void Draw_Line        (int X, int Y, int Direction, COLORREF COLOR, int Cnt, HDC DeviceContext);
void Draw_Square      (int X_1, int Y_1, int Direction_1, COLORREF COLOR_1, int X_2, int Y_2, int Direction_2, COLORREF COLOR_2,int Delay, HDC DeviceContext);
void Predict_Func     (int Direction);
void Shift_String     (int Str_Length);
void Change_String    (HDC DeviceContext, HWND ConsoleWindow);
void Erase_Shift_Draw (void);
int  Is_Snake         (short X, short Y);
int  Is_Block         (short X, short Y);
int  Is_Wormhole_In   (short X, short Y, char Set);
int  Is_Wormhole_Out  (short X, short Y);
int  Get_Direction    (short X_1, short Y_1, short X_2, short Y_2);
void Speed_Func       (int Direction);
void *Get_Key_Stroke  (HDC DeviceContext);

void Unfilled_Func    (void);
void Create_Cookie    (HDC DeviceContext);

void Clear_String     (void);
void Clear_Screen     (HDC DeviceContext);
void Redraw_Screen    (HDC DeviceContext);

int   Last_Key_Stroke;

int   Set_Wormhole_Num = 0;
int   Set_Head_Color = White;
int   Set_Tail_Color = Black;

short User_Def_Tag = 1;
short User_Def_Delay = 12;
short User_Def_Direction = UP;

unsigned char Flag_Pause     = False;
unsigned char Flag_Terminate = False;
unsigned char Flag_PlayAgain = False;

COLORREF Color_Set [16];

typedef struct Position
{
 short X;
 short Y;
} Pos;

Pos Unfilled [25*25+1];
int Unfilled_Length = 0;

Pos Block [25*25+1];
int Block_Length = 0;

Pos Wormhole [10];

typedef struct Move_Information
{
 short Condition;
 short Snake_Length;
 Pos   Position [25*25+1];
} Move_Info;

Move_Info Move;

typedef struct Position_Information
{
 short X;
 short Y;
 short Direction;
} Pos_Info;

Pos_Info Erase, Draw, Predict, Cookie;

int main (int argc, char *argv[]) 
{
 //Get a console handle
 HWND ConsoleWindow = GetConsoleWindow();
 
 //Get a STD handle
 HWND StdHandle = GetStdHandle (STD_OUTPUT_HANDLE);

 //Set cursor invisible
 CONSOLE_CURSOR_INFO CURSOR;
 CURSOR.dwSize = 1;
 CURSOR.bVisible = FALSE;
 SetConsoleCursorInfo (StdHandle, &CURSOR);
 
 //Get Current Font
 for (;;)
 {
  CONSOLE_FONT_INFO GETFONT;
  GetCurrentConsoleFont (StdHandle, FALSE, &GETFONT);
  COORD Fontsize = GetConsoleFontSize (StdHandle, GETFONT.nFont);
  SHORT Font_X = Fontsize.X;
  SHORT Font_Y = Fontsize.Y;
  if (Font_X != 8 || Font_Y != 12)
  {
   system ("chcp 437 > NUL");
   MessageBox (ConsoleWindow, "Please change Console Font to Raster Font  ( 8 x 12 ).", "Notice", MB_TOPMOST | MB_OK | MB_ICONINFORMATION);
   Envir_Error ();
   printf ("\rPress any key to continue...");
   system ("pause>nul");
  }
  else
   break;
 }
 
 //Change Settings
 //SetWindowLong (ConsoleWindow, GWL_STYLE, WS_THICKFRAME);
 //SetWindowLong (ConsoleWindow, GWL_STYLE, WS_CAPTION);
 //SetWindowPos  (ConsoleWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
 SetWindowPos  (ConsoleWindow, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_SHOWWINDOW);

 //Get a handle to device context
 HDC DeviceContext = GetDC (ConsoleWindow);
 
 //Color Set
 Color_Set[Black]   = RGB ( 0 , 0 , 0 );
 Color_Set[White]   = RGB (255,255,255);
 Color_Set[Red]     = RGB (255, 0 , 0 );
 Color_Set[Green]   = RGB ( 0 ,255, 0 );
 Color_Set[Blue]    = RGB ( 0 , 0 ,255);
 Color_Set[Yellow]  = RGB (255,255, 0 );
 Color_Set[Cyan]    = RGB ( 0 ,255,255);
 Color_Set[Magenta] = RGB (255, 0 ,255);
 
 //Title and Resize
 system ("mode con cols=75 lines=50");
 system ("title Console Snake     [ Developer: KVD 2013 ]");
 system ("color 0A");
 
 //Multi-Thread
 pthread_t Key_Stroke_Func;
 pthread_create (&Key_Stroke_Func, NULL, (void*)Get_Key_Stroke, DeviceContext);
 
 //Initialization
 Initialize:;
 Initialization (DeviceContext, argv[1]);
 
 //Special Events
 for (;;)
 {
  //Terminate / PlayAgain
  if (Flag_Terminate == True)
  {
   return 0;
  }
  else if (Flag_PlayAgain == True)
  {
   Flag_PlayAgain = False;
   Clear_Screen (DeviceContext);
   goto Initialize;
  }
  else if (Flag_Pause == True)
  {
   for(;;)
   {
    Sleep (100);
    if (Flag_Pause == False)
     break;
   }
  }
  
  //Predict
  if (Move.Condition == Read)
   Predict_Func (Draw.Direction);
  else
   Predict_Func (Last_Key_Stroke);
  Move.Condition = Read;
   
  //Change String
  Change_String (DeviceContext, ConsoleWindow);
  
  //Set Color
  if (Is_Wormhole_In (Draw.X, Draw.Y, False) == True)
   Set_Head_Color = Blue;
  if (Is_Wormhole_In (Erase.X, Erase.Y, False) == True)
   Set_Tail_Color = Blue;
  if (Is_Wormhole_Out (Draw.X, Draw.Y) == True)
   Set_Head_Color = Red;
  if (Is_Wormhole_Out (Erase.X, Erase.Y) == True)
   Set_Tail_Color = Red;
  
  //Draw Snake
  Draw_Square (Draw.X, Draw.Y, Draw.Direction, Color_Set[Set_Head_Color], Erase.X, Erase.Y, Erase.Direction, Color_Set[Set_Tail_Color], User_Def_Delay, DeviceContext);
  
  //Reset Color
  Set_Head_Color = White;
  Set_Tail_Color = Black;
  
  //End
  if (Is_Snake (Draw.X, Draw.Y) == True || Is_Block (Draw.X, Draw.Y) == True)
  {
   //Game Over
   Clear_String ();
   Clear_Screen (DeviceContext);
   goto Initialize;
  }
 }
 
 //End
 system ("pause>nul");
 return 0;
}

void Envir_Error (void)
{
 // Event
 #define KEYEVENTF_KEYDOWN 0x0000
 #define MAPVK_VK_TO_VSC   0x0000
 // ALT + SPACE
 keybd_event ( VK_MENU  , MapVirtualKey ( VK_MENU, MAPVK_VK_TO_VSC)  , KEYEVENTF_KEYDOWN, 0);
 keybd_event ( VK_SPACE , MapVirtualKey ( VK_SPACE, MAPVK_VK_TO_VSC) , KEYEVENTF_KEYDOWN, 0);
 keybd_event ( VK_SPACE , MapVirtualKey ( VK_SPACE, MAPVK_VK_TO_VSC) , KEYEVENTF_KEYUP  , 0);
 keybd_event ( VK_MENU  , MapVirtualKey ( VK_MENU, MAPVK_VK_TO_VSC)  , KEYEVENTF_KEYUP  , 0);
 // UP
 keybd_event ( VK_UP    , MapVirtualKey ( VK_UP, MAPVK_VK_TO_VSC)    , KEYEVENTF_KEYDOWN, 0);
 keybd_event ( VK_UP    , MapVirtualKey ( VK_UP, MAPVK_VK_TO_VSC)    , KEYEVENTF_KEYUP  , 0);
 // Enter
 keybd_event ( VK_RETURN, MapVirtualKey ( VK_RETURN, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP  , 0);
 keybd_event ( VK_RETURN, MapVirtualKey ( VK_RETURN, MAPVK_VK_TO_VSC), KEYEVENTF_KEYDOWN, 0);
}

void Initialization (HDC DeviceContext, char *Argv)
{
 //Clear
 Clear_String ();
 
 //Default
 Move.Condition = Read;
 Move.Snake_Length = 2;
 Move.Position[0].X = 12;
 Move.Position[0].Y = 12;
 Move.Position[1].X = 12;
 Move.Position[1].Y = 13;
 
 //Get Block File
 int  Num = 0;
 char Input;
 FILE *Fp = fopen (Argv, "rb");
 if (Fp)
 {
  for (;;)
  {
   fscanf (Fp, "[%hd:%hd]",&User_Def_Tag, &User_Def_Delay);
   fseek (Fp, 2, SEEK_CUR);
   int Cnt_Y = 0, Cnt_X = 0;
   if (User_Def_Tag == Intro)
   {
    for (Cnt_Y = 0; Cnt_Y < 25; Cnt_Y++)
    {
     for (Cnt_X = 0; Cnt_X < 25; Cnt_X++)
     {
      fread (&Input, 1, 1, Fp);
      if (Input >= '0' && Input <= '9')
       Draw_Square (Cnt_X, Cnt_Y, DN, Color_Set[Input-'0'], -1, -1, DN, Color_Set[Unset], 0, DeviceContext);
      else if (Input == ' ' || Input == '.')
       continue;
      else
       Cnt_X --;
     }
    }
    fseek (Fp, 2, SEEK_CUR);
    Sleep (User_Def_Delay*1000);
   }
   else if (User_Def_Tag == GameMap)
   {
    for (Cnt_Y = 0; Cnt_Y < 25; Cnt_Y++)
    {
     for (Cnt_X = 0; Cnt_X < 25; Cnt_X++)
     {
      fread (&Input, 1, 1, Fp);
      if (Input == '*')
      {
       Block[Num].X = Cnt_X;
       Block[Num].Y = Cnt_Y;
       Num++;
      }
      else if (Input >= '0' && Input <= '9')
      {
       Wormhole[Input-'0'].X = Cnt_X;
       Wormhole[Input-'0'].Y = Cnt_Y;
      }
      else if (Input == 'H')
      {
       Move.Position[0].X = Cnt_X;
       Move.Position[0].Y = Cnt_Y;
       Draw_Square (Cnt_X, Cnt_Y, UP, Color_Set[Set_Head_Color], -1, -1, UP, Color_Set[Set_Tail_Color], 0, DeviceContext);
      }
      else if (Input == 'T')
      {
       Move.Position[1].X = Cnt_X;
       Move.Position[1].Y = Cnt_Y;
       Draw_Square (Cnt_X, Cnt_Y, UP, Color_Set[Set_Head_Color], -1, -1, UP, Color_Set[Set_Tail_Color], 0, DeviceContext);
      }
      else if (Input == ' ' || Input == '.')
       continue;
      else
       Cnt_X --;
     }
    }
    break;
   }
   else
    break;
  }
 }
 Block_Length = Num;
 Draw.Direction = Get_Direction (Move.Position[0].X, Move.Position[0].Y, Move.Position[1].X, Move.Position[1].Y);
 Last_Key_Stroke = Get_Direction (Move.Position[0].X, Move.Position[0].Y, Move.Position[1].X, Move.Position[1].Y);;
 Create_Cookie (DeviceContext);
 Redraw_Screen (DeviceContext);
}

void Draw_Line (int X, int Y, int Direction, COLORREF COLOR, int Cnt, HDC DeviceContext)
{
 int Draw_X, Draw_Y;
 switch (Direction)
 {
  case UP:
  for (Draw_X=24*(X+0)+0; Draw_X <= 24*(X+1)-1; Draw_X++)
  SetPixel (DeviceContext, Draw_X, 24*(Y+1)-1-Cnt, COLOR);
  break;
  case DN:
  for (Draw_X=24*(X+0)+0; Draw_X <= 24*(X+1)-1; Draw_X++)
  SetPixel (DeviceContext, Draw_X, 24*(Y+0)-0+Cnt, COLOR);
  break;
  case LF:
  for (Draw_Y=24*(Y+0)+0; Draw_Y <= 24*(Y+1)-1; Draw_Y++)
  SetPixel (DeviceContext, 24*(X+1)-1-Cnt, Draw_Y, COLOR);
  break;
  case RG:
  for (Draw_Y=24*(Y+0)+0; Draw_Y <= 24*(Y+1)-1; Draw_Y++)
  SetPixel (DeviceContext, 24*(X+0)-0+Cnt, Draw_Y, COLOR);
  break;
 }
}

void Draw_Square (int X_1, int Y_1, int Direction_1, COLORREF COLOR_1, int X_2, int Y_2, int Direction_2, COLORREF COLOR_2,int Delay, HDC DeviceContext)
{
 int Cnt_1, Cnt_2;
 for (Cnt_1 = 0; Cnt_1 < 4; Cnt_1++)
 {
  for (Cnt_2 = 0; Cnt_2 < 6; Cnt_2++)
  {
   if (!(X_1 < 0 || X_1 > 24) || (Y_1 < 0 || Y_1 > 24))
   Draw_Line (X_1, Y_1, Direction_1, COLOR_1, Cnt_1*6+Cnt_2, DeviceContext);
   if (!(X_2 < 0 || X_2 > 24) || (Y_2 < 0 || Y_2 > 24))
   Draw_Line (X_2, Y_2, Direction_2, COLOR_2, Cnt_1*6+Cnt_2, DeviceContext);
  }
  Sleep (Delay);
 }
}

void Predict_Func (int Direction)
{
 switch (Direction)
 {
  case UP:
  Predict.X = Move.Position[0].X;
  Predict.Y = Move.Position[0].Y - 1;
  break;
  case DN:
  Predict.X = Move.Position[0].X;
  Predict.Y = Move.Position[0].Y + 1;
  break;
  case LF:
  Predict.X = Move.Position[0].X - 1;
  Predict.Y = Move.Position[0].Y;
  break;
  case RG:
  Predict.X = Move.Position[0].X + 1;
  Predict.Y = Move.Position[0].Y ;
  break;
 }
 Predict.X = (Predict.X+25)%25;
 Predict.Y = (Predict.Y+25)%25;
 Predict.Direction = Direction;
}

void Shift_String (int Str_Length)
{
 int Cnt;
 for (Cnt = Str_Length-2; Cnt+1 >= 0; Cnt--)
 {
  Move.Position[Cnt+1].X = Move.Position[Cnt].X;
  Move.Position[Cnt+1].Y = Move.Position[Cnt].Y;
 }
 Move.Position[0].X = '\0';
 Move.Position[Str_Length].X = '\0';
 Move.Position[0].Y = '\0';
 Move.Position[Str_Length].Y = '\0';
}

void Change_String (HDC DeviceContext, HWND ConsoleWindow)
{
 if (Is_Wormhole_In (Move.Position[0].X, Move.Position[0].Y, True) == True)
 {
  Predict.X = Predict.X + (Wormhole[Set_Wormhole_Num+1].X - Wormhole[Set_Wormhole_Num].X);
  Predict.Y = Predict.Y + (Wormhole[Set_Wormhole_Num+1].Y - Wormhole[Set_Wormhole_Num].Y);
 }
 //if (Is_Wormhole_In (Move.Position[1].X, Move.Position[1].Y, True) == True)
 //{
 // Move.Position[1].X = Wormhole[Set_Wormhole_Num+1].X;
 // Move.Position[1].Y = Wormhole[Set_Wormhole_Num+1].Y;
 //}
 //
 if (Predict.X == Cookie.X && Predict.Y == Cookie.Y)
 {
  Erase.X = -1;
  Erase.Y = -1;
  Move.Snake_Length++;
  Shift_String (Move.Snake_Length);
  Move.Position[0].X = Predict.X;
  Move.Position[0].Y = Predict.Y;
  Draw.X = Predict.X;
  Draw.Y = Predict.Y;
  Create_Cookie (DeviceContext);
  Draw.Direction = Predict.Direction;
 }
 else if (Is_Wormhole_Out (Predict.X, Predict.Y) == True)
 {
  // Keep Blank.
 }
 else
 {
  Erase_Shift_Draw ();
 }
}

void Erase_Shift_Draw (void)
{
 Erase.X = Move.Position[Move.Snake_Length-1].X;
 Erase.Y = Move.Position[Move.Snake_Length-1].Y;
 Erase.Direction = Get_Direction (Move.Position[Move.Snake_Length-2].X, Move.Position[Move.Snake_Length-2].Y, Move.Position[Move.Snake_Length-1].X, Move.Position[Move.Snake_Length-1].Y);
 //
 Shift_String (Move.Snake_Length);
 Move.Position[0].X = Predict.X;
 Move.Position[0].Y = Predict.Y;
 Draw.X = Predict.X;
 Draw.Y = Predict.Y;
 Draw.Direction = Predict.Direction;
}

int Is_Snake (short X, short Y)
{
 int Cnt;
 for (Cnt = 1; Cnt < Move.Snake_Length; Cnt++)
 {
  if (Move.Position[Cnt].X == X && Move.Position[Cnt].Y == Y)
   return True;
 }
 return False;
}

int Is_Block (short X, short Y)
{
 int Cnt;
 for (Cnt = 0; Cnt < Block_Length; Cnt++)
 {
  if (Block[Cnt].X == X && Block[Cnt].Y == Y)
   return True;
 }
 return False;
}

int Is_Wormhole_In (short X, short Y, char Set)
{
 int Cnt;
 for (Cnt = 0; Cnt <= 8; Cnt = Cnt + 2)
 {
  if (Wormhole[Cnt].X == X && Wormhole[Cnt].Y == Y)
  {
   if (Set == True)
    Set_Wormhole_Num = Cnt;
   return True;
  }
 }
  return False;
}

int Is_Wormhole_Out (short X, short Y)
{
 int Cnt;
 for (Cnt = 1; Cnt <= 9; Cnt = Cnt + 2)
 {
  if (Wormhole[Cnt].X == X && Wormhole[Cnt].Y == Y)
   return True;
 }
  return False;
}

int  Get_Direction (short X_1, short Y_1, short X_2, short Y_2)
{
 if ((X_1 == X_2 + 1 && Y_1 == Y_2) || (X_1 == 0 && X_2 == 24))
  return RG;
 if ((X_2 == X_1 + 1 && Y_1 == Y_2) || (X_2 == 0 && X_1 == 24))
  return LF;
 if ((Y_1 == Y_2 + 1 && X_1 == X_2) || (Y_1 == 0 && Y_2 == 24))
  return DN;
 if ((Y_2 == Y_1 + 1 && X_1 == X_2) || (Y_2 == 0 && Y_1 == 24))
  return UP;
}

void Speed_Func (int Direction)
{
 if (Direction == UP && User_Def_Delay > 1)
  User_Def_Delay--;
 if (Direction == DN && User_Def_Delay < 255 )
  User_Def_Delay++;
}

void *Get_Key_Stroke (HDC DeviceContext)
{
 unsigned char Key_Stroke_1, Key_Stroke_2;
 for(;;)
 {
  Key_Stroke_2 = getch ();
  if (Key_Stroke_2 == 0x20)
  {
   Flag_Pause = (Flag_Pause+1)%2;
  }
  else if (Key_Stroke_1 == 0xE0)
  {
   Move.Condition = UnRead;
   if      (Key_Stroke_2 == 0x48 && Draw.Direction != DN)
    Last_Key_Stroke = UP;
   else if (Key_Stroke_2 == 0x50 && Draw.Direction != UP)
    Last_Key_Stroke = DN;
   else if (Key_Stroke_2 == 0x4B && Draw.Direction != RG)
    Last_Key_Stroke = LF;
   else if (Key_Stroke_2 == 0x4D && Draw.Direction != LF)
    Last_Key_Stroke = RG;
   else
   {
    if (Key_Stroke_2 == 0x4F)
     Flag_Terminate = True;
    else if (Key_Stroke_2 == 0x47)
     Flag_PlayAgain = True;
    else if (Key_Stroke_2 == 0x49)
     Speed_Func (UP);
    else if (Key_Stroke_2 == 0x51)
     Speed_Func (DN);
    Move.Condition = Read;
    continue;
   }
  }
  else if (Key_Stroke_1 == 0x00)
  {
   if (Key_Stroke_2 == 0x3F)
    Redraw_Screen (DeviceContext);
  }
  Key_Stroke_1 = Key_Stroke_2;
 }
}

void Unfilled_Func (void)
{
 int Cnt_X, Cnt_Y, Cnt_2, Cnt_3 = 0, Flag = 1;
 for (Cnt_X = 0; Cnt_X < 25; Cnt_X++)
 {
  for (Cnt_Y = 0; Cnt_Y < 25; Cnt_Y++)
  {
   Flag = 1;
   //Snake
   for (Cnt_2 = 0 ; Cnt_2 < Move.Snake_Length; Cnt_2++)
    if (Move.Position[Cnt_2].X == Cnt_X && Move.Position[Cnt_2].Y == Cnt_Y)
     Flag = 0;
   //Block
   for (Cnt_2 = 0 ; Cnt_2 < Block_Length; Cnt_2++)
    if (Block[Cnt_2].X == Cnt_X && Block[Cnt_2].Y == Cnt_Y)
     Flag = 0;
   //Wormhole
   for (Cnt_2 = 0 ; Cnt_2 < 10; Cnt_2++)
    if (Wormhole[Cnt_2].X == Cnt_X && Wormhole[Cnt_2].Y == Cnt_Y)
     Flag = 0;
   if (Flag == 1)
   {
    Unfilled[Cnt_3].X = Cnt_X;
    Unfilled[Cnt_3].Y = Cnt_Y;
    Cnt_3++;
   }
  }
 }
 Unfilled_Length = Cnt_3;
}

void Create_Cookie (HDC DeviceContext)
{
 Unfilled_Func ();
 int Rand = clock()%Unfilled_Length;
 Cookie.X = Unfilled[Rand].X;
 Cookie.Y = Unfilled[Rand].Y;
 Draw_Square (Cookie.X, Cookie.Y, UP, Color_Set[Green], -1, -1, UP, Color_Set[Unset], 0, DeviceContext);
}

void Clear_String (void)
{
 memset(Block,    0, sizeof(Block));
 memset(Unfilled, 0, sizeof(Unfilled));
 memset(Wormhole,-1, sizeof(Wormhole));
 memset(&Erase,   0, sizeof(Erase));
 memset(&Draw,    0, sizeof(Draw));
 memset(&Predict, 0, sizeof(Predict));
 memset(&Cookie,  0, sizeof(Cookie));
 memset(&Move,    0, sizeof(Move));
}

void Clear_Screen (HDC DeviceContext)
{
 int Cnt_X, Cnt_Y;
 for (Cnt_Y = 0; Cnt_Y < 600; Cnt_Y++)
  for (Cnt_X = 0; Cnt_X < 600; Cnt_X++)
   SetPixel (DeviceContext, Cnt_X, Cnt_Y, Color_Set[Black]);
}

void Redraw_Screen (HDC DeviceContext)
{
 Flag_Pause = True;
 Sleep (User_Def_Delay*6);
 int Cnt;
 for (Cnt = 0; Cnt < Block_Length; Cnt++)
  Draw_Square (Block[Cnt].X, Block[Cnt].Y, DN, Color_Set[Yellow], -1, -1, DN, Color_Set[Unset], 0, DeviceContext);
 for (Cnt = 0; Cnt <= 8; Cnt = Cnt + 2)
  Draw_Square (Wormhole[Cnt].X, Wormhole[Cnt].Y, DN, Color_Set[Blue], -1, -1, DN, Color_Set[Unset], 0, DeviceContext);
 for (Cnt = 1; Cnt <= 9; Cnt = Cnt + 2)
  Draw_Square (Wormhole[Cnt].X, Wormhole[Cnt].Y, DN, Color_Set[Red], -1, -1, DN, Color_Set[Unset], 0, DeviceContext);
 Draw_Square (Cookie.X, Cookie.Y, DN, Color_Set[Green], -1, -1, DN, Color_Set[Unset], 0, DeviceContext);
 Flag_Pause = False;
}
