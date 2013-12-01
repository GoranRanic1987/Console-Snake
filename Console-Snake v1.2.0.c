/// +------------------------+
/// | Copyright (c) 2013 KVD |
/// |  All Rights Reserved.  |
/// |     Version: 1.2.0     |
/// |  Internal: Version 11  |
/// +------------------------+

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define WINVER 0x0501
#include <signal.h>
#include <windows.h>

#define UP -1
#define DN  1
#define LF -3
#define RG  3

#define True    1
#define False   0

#define Read   -1
#define UnRead  1

#define CP printf("CheckPoint\n");
#define SP system("pause");

BOOL WINAPI GetCurrentConsoleFont (HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFO lpConsoleCurrentFont);
COORD WINAPI GetConsoleFontSize (HANDLE hConsoleOutput, DWORD nFont);

void Envir_Error     (void);
void Initialization  (HDC DeviceContext, char *Argv);
void Draw_Line       (int X, int Y, int Direction, COLORREF COLOR, int Cnt, HDC DeviceContext);
void Draw_Square     (int X_1, int Y_1, int Direction_1, COLORREF COLOR_1, int X_2, int Y_2, int Direction_2, COLORREF COLOR_2,int Delay, HDC DeviceContext);
void Predict_Func    (int Direction);
void Move_String     (int Str_Length);
void Change_Str      (HDC DeviceContext, HWND ConsoleWindow);
int  Is_Snake        (short X, short Y);
void Speed_Func      (int Direction);
void *Get_Key_Stroke (void);

void Unfilled_Func   (void);
void Create_Cookie   (HDC DeviceContext);

void Clear_String    (void);
void Clear_Screen    (HDC DeviceContext);

unsigned char  User_Def_Delay = 20;

typedef struct Position
{
 short X;
 short Y;
} Pos;

Pos Unfilled[25*25+1];
int Unfilled_Length = 0;

Pos Block[25*25+1];
int Block_Length = 0;

typedef struct Move_Information
{
 short Condition;
 short Direction;
 short Snake_Length;
 Pos   Position[25*25+1];
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
 
 //Color
 COLORREF Color_White = RGB (255,255,255);
 COLORREF Color_Black = RGB ( 0 , 0 , 0 );
 
 //Title and Resize
 system ("mode con cols=75 lines=50");
 system ("title Console Snake     [ Developer: KVD 2013 ]");
 
 //Multi-Thread
 pthread_t Key_Stroke_Func;
 pthread_create (&Key_Stroke_Func, NULL, (void*)Get_Key_Stroke, NULL);
 
 //Initialization
 Initialize:;
 Initialization (DeviceContext, argv[1]);
 
 //Main
 for (;;)
 {
  if (Move.Condition == UnRead)
  {
   Move.Condition = Read;
   Change_Str (DeviceContext, ConsoleWindow);
   Draw_Square (Draw.X, Draw.Y, Draw.Direction, Color_White, Erase.X, Erase.Y, Erase.Direction, Color_Black, User_Def_Delay, DeviceContext);
  }
  else
  {
   Predict_Func (Draw.Direction);
   Change_Str (DeviceContext, ConsoleWindow);
   Draw_Square (Draw.X, Draw.Y, Draw.Direction, Color_White, Erase.X, Erase.Y, Erase.Direction, Color_Black, User_Def_Delay, DeviceContext);
  }
  //
  if (Is_Snake (Draw.X, Draw.Y) == True)
  {
   MessageBox (ConsoleWindow, "GAME OVER", "Notice", MB_TOPMOST | MB_OK | MB_ICONSTOP);
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
 //Get Block File
 char Block_Str[25*25+1] = {0};
 FILE *Fp = fopen (Argv, "r");
 fread (Block_Str, sizeof (char), 625, Fp);
 int Cnt = 0, Num = 0;
 for (Cnt = 0; Cnt < 625; Cnt++)
 {
  if (Block_Str[Cnt] == '1')
  {
   Block[Num].X = Cnt%25;
   Block[Num].Y = (short)(Cnt/25);
   Num++;
  }
 }
 Block_Length = Num;
 
 //
 Clear_String ();
 Move.Condition = Read;
 Move.Snake_Length = 2;
 Move.Position[1].X = 12;
 Move.Position[1].Y = 13;
 Draw_Square (12, 13, UP, RGB(255,255,255), -1, -1, UP, RGB(255,255,255), User_Def_Delay, DeviceContext);
 Move.Position[0].X = 12;
 Move.Position[0].Y = 12;
 Draw_Square (12, 12, UP, RGB(255,255,255), -1, -1, UP, RGB(255,255,255), User_Def_Delay, DeviceContext);
 Draw.Direction = UP;
 Create_Cookie (DeviceContext);
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

void Move_String (int Str_Length)
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

void Change_Str (HDC DeviceContext, HWND ConsoleWindow)
{
 if (Predict.X == Cookie.X && Predict.Y == Cookie.Y)
 {
  Move.Snake_Length++;
  Move_String (Move.Snake_Length);
  Move.Position[0].X = Predict.X;
  Move.Position[0].Y = Predict.Y;
  Draw.X = Predict.X;
  Draw.Y = Predict.Y;
  Create_Cookie (DeviceContext);
 }
 else
 {
  Erase.X = Move.Position[Move.Snake_Length-1].X;
  Erase.Y = Move.Position[Move.Snake_Length-1].Y;
 if      (Move.Position[Move.Snake_Length-1].X == 0 && Move.Position[Move.Snake_Length-2].X == 24)
  Erase.Direction = LF;
 else if (Move.Position[Move.Snake_Length-1].X == 24 && Move.Position[Move.Snake_Length-2].X == 0)
  Erase.Direction = RG;
 else if (Move.Position[Move.Snake_Length-1].Y == 0 && Move.Position[Move.Snake_Length-2].Y == 24)
  Erase.Direction = UP;
 else if (Move.Position[Move.Snake_Length-1].Y == 24 && Move.Position[Move.Snake_Length-2].Y == 0)
  Erase.Direction = DN;
 else if (Move.Position[Move.Snake_Length-1].X > Move.Position[Move.Snake_Length-2].X)
  Erase.Direction = LF;
 else if (Move.Position[Move.Snake_Length-2].X > Move.Position[Move.Snake_Length-1].X)
  Erase.Direction = RG;
 else if (Move.Position[Move.Snake_Length-1].Y > Move.Position[Move.Snake_Length-2].Y)
  Erase.Direction = UP;
 else if (Move.Position[Move.Snake_Length-2].Y > Move.Position[Move.Snake_Length-1].Y)
  Erase.Direction = DN;
  Move_String (Move.Snake_Length);
  Move.Position[0].X = Predict.X;
  Move.Position[0].Y = Predict.Y;
  Draw.X = Predict.X;
  Draw.Y = Predict.Y;
 }
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

void Speed_Func (int Direction)
{
 if (Direction == UP && User_Def_Delay > 1)
  User_Def_Delay--;
 if (Direction == DN && User_Def_Delay < 255 )
  User_Def_Delay++;
}

void *Get_Key_Stroke (void)
{
 unsigned char Key_Stroke_1, Key_Stroke_2;
 for(;;)
 {
  Key_Stroke_2 = getch ();
  if (Key_Stroke_1 == 0xE0)
  {
   Move.Condition = UnRead;
   if      (Key_Stroke_2 == 0x48 && Draw.Direction != DN)
    Predict_Func (UP);
   else if (Key_Stroke_2 == 0x50 && Draw.Direction != UP)
    Predict_Func (DN);
   else if (Key_Stroke_2 == 0x4B && Draw.Direction != RG)
    Predict_Func (LF);
   else if (Key_Stroke_2 == 0x4D && Draw.Direction != LF)
    Predict_Func (RG);
   else
   {
    if (Key_Stroke_2 == 0x4F)
     exit (EXIT_SUCCESS);
    else if (Key_Stroke_2 == 0x49)
     Speed_Func (UP);
    else if (Key_Stroke_2 == 0x51)
     Speed_Func (DN);
    Move.Condition = Read;
    continue;
   }
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
   for (Cnt_2 = 0 ; Cnt_2 < Move.Snake_Length; Cnt_2++)
   {
    if (Move.Position[Cnt_2].X == Cnt_X && Move.Position[Cnt_2].Y == Cnt_Y)
    {
     Flag = 0;
     break;
    }
   }
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
 Draw_Square (Cookie.X, Cookie.Y, UP, RGB( 0 ,255, 0 ), -1, -1, UP, RGB(255,255,255), 0, DeviceContext);
}

void Clear_String (void)
{
 memset (&Block   , 0, sizeof (Pos));
 memset (&Unfilled, 0, sizeof (Pos));
 memset (&Erase   , 0, sizeof (Pos_Info));
 memset (&Draw    , 0, sizeof (Pos_Info));
 memset (&Predict , 0, sizeof (Pos_Info));
 memset (&Cookie  , 0, sizeof (Pos_Info));
 memset (&Move    , 0, sizeof (Move_Info));
}

void Clear_Screen (HDC DeviceContext)
{
 int Cnt_X, Cnt_Y;
 for (Cnt_X = 0; Cnt_X < 600; Cnt_X++)
  for (Cnt_Y = 0; Cnt_Y < 600; Cnt_Y++)
   SetPixel (DeviceContext, Cnt_X, Cnt_Y, RGB (0,0,0));
}
