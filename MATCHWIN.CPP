// OWLCVT 12/25/93 15:08:09
 /***********************************************************************
 * Match Sticks for Windows NT.                                         *
 * Version 1.0.                                                         *
 * Copyright (c) 1993 by R. Bowyer.                                     *
 ***********************************************************************/


/***********************************************************************
 * Includes, defines + global varables.							  	        *
 ***********************************************************************/

#pragma hdrfile "STD.CSM"
  //Owl includes.
  #include "owl\compat.h"
  #include <bwcc.h>
//#include <owl\owlall.h>

  //Standard library includes.
  #pragma hdrfile "STD.CSM"
  #include <iostream.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <math.h>
  #include <time.h>
  #include <owl\applicat.h>
  #include <owl\framewin.h>
  #include <owl\dialog.h>
  #include <owl\dc.h>
  #include <owl\button.h>
  #include <owl\radiobut.h>
  #include <owl\menu.h>
  #include <typeinfo.h>
#pragma hdrstop

//My includes.
#include "Matcheng.h"
#include "MatchRes.h"

//Rewrite runtime error routine to work with windows.
#ifdef debug
  HWND hMainWin;
  void WinError(int err_no, const char err_msg[])
  {
    ::MessageBox(hMainWin, err_msg, "Error", MB_ICONEXCLAMATION |
                                           MB_APPLMODAL | MB_OK);
    PostQuitMessage(err_no);
  }
  TErrorHandler WinErr(WinError);
#endif

const int Origin_X = 100;        //}Top left corner 
const int Origin_Y = 100;        //}of the display.
const int Between_Piles = 100;   //Distance between piles of matches.
const int Between_Matches = 10;  //Distance between each.


/***********************************************************************
 * Classes.							       *
 ***********************************************************************/

class TGameGui: public TGame
//We add functions to TGame to create a GUI for it.
{
  TWindow _FAR * Parent;       //Window who owns the object.
  HBITMAP hMatchBitmap;  //Bitmap used to draw matches.
  int Position_Selected; //Represents the pile which has matches inverted.
  int Match1_Selected;   //First match in pile to be inverted.
  int Match2_Selected;   //Last match in pile to be inverted.
public:
  BOOL Matches_Inverted; //True if matches are inverted.
  TGameGui(TWindow _FAR *, int);
  virtual void Resign();
  void Draw(HDC) const;
  void Draw() const;
  BOOL Clicked_a_Match(const POINT&);
  void Dragging(const POINT&);
  BOOL Clicked_Inverted(const POINT&, TMove&) const;
};

class TBitmapX
//Abstracts some of the Win API Bitmap functions.
{
  HDC hdc, hdcMemory;
  HBITMAP hbmpBitmap, hbmpOld;
  BITMAP bm;
public:
  TBitmapX(HDC, HBITMAP);
  TBitmapX(HBITMAP);
  ~TBitmapX();
  void Draw(int, int, DWORD);
  int Height() const { return bm.bmHeight; };
  int Width() const { return bm.bmWidth; };
};

class TMatchApp: public TApplication
{
public:
  HICON hMatchIcon;
  HBITMAP hMatchBitmap;

  TMatchApp(LPSTR, HINSTANCE, HINSTANCE, LPSTR, int);
  virtual ~TMatchApp();
  virtual void InitMainWindow();
};

class TMatchWin: public TFrameWindow
//My main window.
{
  TGameGui* MatchGame;        
  BOOL GameRunning;           //True if a game is being played.
  BOOL SelectingMatches;      //True if matches are being inverted.
  void PlayGame(TMove Move);  
public:
  TMatchWin(TWindow *, LPSTR, TModule _FAR * AModule = NULL);
  virtual ~TMatchWin();
  virtual void Paint(TDC&, BOOL, TRect&);
  virtual BOOL CanClose() {return !GameRunning;};
  virtual void GetWindowClass(WNDCLASS&);

  void CMGameStart();
  void CMGameSkillLevel();
  void CMGameResign();
  void CMGameExit();
  void CMHelpAbout();
  void EvLButtonDown( UINT, TPoint& );
  void EvMouseMove( UINT, TPoint& );
  void EvLButtonUp( UINT, TPoint& );

  void GameOver();
DECLARE_RESPONSE_TABLE( TMatchWin );
};

class TDialogGoesFirst: public TDialog
//A dialog box with a yes and no buttons.
{
public:
  TDialogGoesFirst(TWindow * AParent, int ResourceId)
		  :TDialog(AParent, ResourceId) {};
  void Yes( UINT );
  void No( UINT );
DECLARE_RESPONSE_TABLE( TDialogGoesFirst );
};


 /***********************************************************************
 * Functions							       *
 ***********************************************************************/

void Pause(int s)
//Pause for s seconds.
{
  time_t GrabbedTime = time(NULL);

  while (time(NULL) <= GrabbedTime + s) {};
}

/***********************************************************************
 * Methods							       *
 ***********************************************************************/

//Methods for TGameGui.

TGameGui::TGameGui(TWindow _FAR * P, int Skill = 1): TGame(Skill)
{
  TMatchApp* PMatchApp = (TMatchApp*) (Parent = P)->GetApplication();
  hMatchBitmap = PMatchApp->hMatchBitmap;
  Matches_Inverted = FALSE;
}

void TGameGui::Resign()
{
  TGame::Resign();
  Matches_Inverted = FALSE;
}
void TGameGui::Draw(HDC dc) const
//Draws the game onto the window using MatchBitmap to represent each
//match.
{
  TBitmapX Bitmap(dc, hMatchBitmap);
  TPosition Pos;
  int i, j, TopX, a, b, c;

  Get_Position(Pos);

//Draw all the matches and put blanks over removed matches.
  for (i = 0; i <= Max_No_Piles - 1; i++)
  for (j = 1; j <= 7 - 2*i ; j++)
  {
	 TopX = Origin_X + Between_Piles * i + Between_Matches * j;
	 if (j <= Pos[i])
		Bitmap.Draw(TopX, Origin_Y, SRCCOPY);
	 else
		Bitmap.Draw(TopX, Origin_Y, WHITENESS);
  }

//If any matches are inverted we had better invert them.
  if (Matches_Inverted)
  {
	 a = Match1_Selected; b = Match2_Selected;
	 if (b < a) { c = b; b = a; a = c; }
	 for (j = 1; j <= Pos[Position_Selected]; j++)
	 if((a <= j) && (j <= b))
	 Bitmap.Draw(Origin_X + Between_Piles * Position_Selected
					 + Between_Matches * j, Origin_Y, NOTSRCCOPY);
  }
}

void TGameGui::Draw() const
{
  HDC dc = GetDC(Parent->HWindow);

  Draw(dc);
  ReleaseDC(Parent->HWindow, dc);
}

BOOL TGameGui::Clicked_a_Match(const POINT& Pt)
//Given a point in the window decides if, the point is on a match.
//If this is the case hilights the match and returns true.
{
  HDC dc = GetDC(Parent->HWindow);
  TBitmapX Bitmap(dc, hMatchBitmap);
  TPosition Pos;
  BOOL ClickedMatch = FALSE;
  RECT Rect;
  int i, j;

  Get_Position(Pos);

  for (i = 0; i <= Max_No_Piles - 1; i++)
  for (j = 1; j <= Pos[i]; j++)
  {
    Rect.left = Origin_X + Between_Piles * i + Between_Matches * j;
    Rect.top = Origin_Y;
    Rect.bottom = Rect.top + Bitmap.Height();
    Rect.right = Rect.left + Bitmap.Width();
    if (PtInRect(&Rect, Pt))
    {
		ClickedMatch = TRUE;
      Position_Selected = i;
      Match2_Selected = Match1_Selected = j;
      Bitmap.Draw(Rect.left, Rect.top, NOTSRCCOPY);
    }
  }

  ReleaseDC(Parent->HWindow, dc);

  return ClickedMatch;
}

void TGameGui::Dragging(const POINT& Pt)
//Given a point in the window which is the location of the cursor.
//Hilights a number of matches, starting from the match which was
//first clicked and ending at the location of the cursor.
{
  HDC dc = GetDC(Parent->HWindow);
  TBitmapX Bitmap(dc, hMatchBitmap);
  TPosition Pos;
  DWORD Action;
  int j, a, b, c;
  float Temp;

  Get_Position(Pos);

  a = Match1_Selected;
  Temp = Pt.x - Origin_X - (Between_Piles * Position_Selected);
  Temp = Temp - Between_Matches; //Position rel. start of group.
  Temp = Temp - (a - 1) * Between_Matches; //Position rel. a.                                 
  if (Temp < 0) Temp = Temp - Bitmap.Width();
  Temp = Temp  / Between_Matches;  //Position rel. a in between_matches.
  c = (Temp > 0) ? floor(Temp) : ceil(Temp);
  b = a + c;
  if (b > Pos[Position_Selected]) b = Pos[Position_Selected];
  if (b < 1) b = 1;
  Match2_Selected = b;

  if (b < a) {c = b; b = a; a = c;}

//Draws the pile with inverted matches.
  for (j = 1; j <= Pos[Position_Selected]; j++)
  { 
    Action = ((a <= j) && (j <= b)) ? NOTSRCCOPY : SRCCOPY;
	 Bitmap.Draw(Origin_X + Between_Piles * Position_Selected
                + Between_Matches * j, Origin_Y, Action);
  }

  ReleaseDC(Parent->HWindow, dc);
}

BOOL TGameGui::Clicked_Inverted(const POINT& Pt, TMove& Move) const
//Given a point in the window which is the loaction of the cursor.
//Decides if this on the inverted area. If this is the case returns
//the users move.
{
  HDC dc = GetDC(Parent->HWindow);
  TBitmapX Bitmap(dc, hMatchBitmap);
  RECT Rect;
  int a, b, c, Temp;

  a = Match1_Selected;
  b = Match2_Selected;
  if (b < a) {c = b; b = a; a = c;}
  Temp = Origin_X + Position_Selected * Between_Piles;
  Rect.top = Origin_Y;
  Rect.bottom = Rect.top + Bitmap.Height();
  Rect.left  = Temp + a * Between_Matches;
  Rect.right = Temp + b * Between_Matches + Bitmap.Width(); 
  Move.Assign(Position_Selected, b - a + 1);

  ReleaseDC(Parent->HWindow, dc);
  return ( PtInRect(&Rect, Pt) );
}


//Methods for TBitmapX.

TBitmapX::TBitmapX(HDC h, HBITMAP b)
{
  hdc = h; hbmpBitmap = b;

  GetObject(hbmpBitmap, sizeof(BITMAP), &bm);
  hdcMemory = CreateCompatibleDC(hdc);
  hbmpOld = (HBITMAP) SelectObject(hdcMemory, hbmpBitmap);
}

TBitmapX::~TBitmapX()
{
  SelectObject(hdcMemory, hbmpOld);
  DeleteDC(hdcMemory);
}


void TBitmapX::Draw(int x, int y, DWORD code)
//Draws a bitmap onto the window at (x, y) using operation given
//by code.
{
  BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, hdcMemory, 0, 0, code);
}


//Methods for TMatchApp.

TMatchApp::TMatchApp(LPSTR AName, HINSTANCE hInstance, HINSTANCE
	    hPrevInstance, LPSTR lpCmdLine, int nCmdShow) :
	    TApplication(AName, hInstance, hPrevInstance, lpCmdLine,
	    nCmdShow)
//Load int bitmap, icon and accelerator.
{
  hMatchIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(MatchIcon));
  HAccTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(MatchAccelerator));
  hMatchBitmap = ::LoadBitmap(hInstance, MAKEINTRESOURCE(MatchBitMap));
}

TMatchApp::~TMatchApp()
{
  DestroyIcon(hMatchIcon);   //Must  check for errors.
  DeleteObject(hMatchBitmap); //Must check for errors.
}


void TMatchApp::InitMainWindow()
{
  (void) BWCCGetVersion();
  SetMainWindow( new TMatchWin(NULL, "Matchsticks for Windows NT"));
}


//Methods for TmatchWin.

TMatchWin::TMatchWin(TWindow * AParent, LPSTR ATitle,
		 TModule _FAR *): TFrameWindow(AParent, ATitle),
										  TWindow(AParent, ATitle)
{
#ifdef debug
  hMainWin = HWindow;
#endif

  AssignMenu(MatchMenu);
  MatchGame = new TGameGui(this);
  GameRunning = FALSE;
  SelectingMatches = FALSE;
}

TMatchWin::~TMatchWin()
{
  delete MatchGame;
}

void TMatchWin::Paint(TDC& dc, BOOL, TRect&)
//Updates the main window.
{
  if (GameRunning == TRUE) MatchGame->Draw( dc );
}

void TMatchWin::GetWindowClass(WNDCLASS& AWndClass)
{
  TFrameWindow::GetWindowClass(AWndClass);

  TMatchApp* PMatchApp = dynamic_cast<TMatchApp*>(GetApplication());

#ifdef debug
  if (PMatchApp == 0) throw Bad_cast();
#endif

//Sets the windows icon.
  AWndClass.hIcon =  PMatchApp->hMatchIcon;
}


void TMatchWin::CMGameStart()
{
  int Result;
  TDialogGoesFirst* ADialog;

  EnableMenuItem(::GetMenu(HWindow), GameResign, MF_ENABLED);
  EnableMenuItem(::GetMenu(HWindow), GameStart, MF_GRAYED);
  EnableMenuItem(::GetMenu(HWindow), GameExit, MF_GRAYED);

  GameRunning = TRUE;
  MatchGame->Reset();
  MatchGame->Draw();

  ADialog = new TDialogGoesFirst(this, GoesFirstDialogBor);
  new TButton(ADialog, IDYES, NULL);
  new TButton(ADialog, IDNO, NULL);
  Result = GetModule()->ExecDialog(ADialog);
  if (Result == IDNO)
  {
	 MatchGame->Play(MatchGame->Get_Best_Move());
	 MatchGame->Draw();
  }
}

void TMatchWin::CMGameSkillLevel()
{
  int i;
  struct { UINT RadioState[4]; } TransferStruct;
  TDialog* ADialog = new TDialog(this, SkillLevelDialog);

  new TRadioButton(ADialog, RadioBeginner, NULL);
  new TRadioButton(ADialog, RadioIntermediate, NULL);
  new TRadioButton(ADialog, RadioAdvanced, NULL);
  new TRadioButton(ADialog, RadioExpert, NULL);
  ADialog->SetTransferBuffer (&TransferStruct);

  for (i = 1; i <= Max_Skill_Level; i++)
	 TransferStruct.RadioState[i-1] = (MatchGame->Get_Skill_Level() == i)
  	 ? BF_CHECKED : BF_UNCHECKED;

  if (GetModule()->ExecDialog(ADialog) == IDOK)
	for (i = 1; i<= Max_Skill_Level ; i++)
		if (TransferStruct.RadioState[i-1] == BF_CHECKED)
		 MatchGame->Set_Skill_Level(i);
}

void TMatchWin::CMGameResign()
{

  MatchGame->Resign();
  MatchGame->Draw();
  GameOver();
}

void TMatchWin::CMGameExit()
{

  CloseWindow();
}

void TMatchWin::CMHelpAbout()
{

  GetModule()->ExecDialog(new TDialog(this, AboutDialog));
}

void TMatchWin::PlayGame(TMove Move)
//Given a move from the user, plays it and then responds,
//checking to see if the game has finished.
{
  MatchGame->Draw();
  MatchGame->Play(Move);
  MatchGame->Draw();
//Pause(1);
  if ( MatchGame->Has_Lost() )
  {
	 GetModule()->ExecDialog(new TDialog(this, WonDialogBor));
	 GameOver();
  }
  else
  {
	 MatchGame->Play( MatchGame->Get_Best_Move() );
	 MatchGame->Draw();
	 if ( MatchGame->Has_Lost() )
	 {
		GetModule()->ExecDialog(new TDialog(this, LostDialogBor));
		GameOver();
	 }
  }
}

void TMatchWin::EvLButtonDown( UINT, TPoint& )
{
	TMessage Msg = __GetTMessage();
if (GameRunning)
{
  POINT MouseDown = { Msg.LP.Lo, Msg.LP.Hi };
  TMove Move;

  if (MatchGame->Matches_Inverted)
  {
	 MatchGame->Matches_Inverted = FALSE;
	 MatchGame->Draw();
	 if (MatchGame->Clicked_Inverted(MouseDown, Move)) PlayGame(Move);
  }
  else if (MatchGame->Clicked_a_Match(MouseDown))
  {
	SelectingMatches = TRUE;
	::SetCapture(HWindow);
  }
};}


void TMatchWin::EvMouseMove( UINT, TPoint& )
{
	TMessage Msg = __GetTMessage();
if (GameRunning && SelectingMatches)
{
  POINT MousePos = { Msg.LP.Lo, Msg.LP.Hi };

  MatchGame->Dragging(MousePos);
};}


void TMatchWin::EvLButtonUp( UINT, TPoint& )
{

  if (GameRunning && SelectingMatches)
  {
	 MatchGame->Matches_Inverted = TRUE;
	 ReleaseCapture();
	 SelectingMatches = FALSE;
  }
}

void TMatchWin::GameOver()
//Enables and disables menu items.
{
  GameRunning = FALSE;
//EnableMenuItem(::GetMenu(HWindow), GameStart, MF_ENABLED);
//EnableMenuItem(::GetMenu(HWindow), GameExit, MF_ENABLED);
//EnableMenuItem(::GetMenu(HWindow), GameResign, MF_GRAYED);
  EnableMenuItem(GetMenu(), GameStart, MF_ENABLED | MF_BYCOMMAND);
  EnableMenuItem(GetMenu(), GameExit, MF_ENABLED | MF_BYCOMMAND);
  EnableMenuItem(GetMenu(), GameResign, MF_GRAYED | MF_BYCOMMAND);
}

//Methods for TDialogGoesFirst.

void TDialogGoesFirst::Yes( UINT )
{

  CloseWindow(IDYES);
}

void TDialogGoesFirst::No( UINT )
{

  CloseWindow(IDNO);
}


/***********************************************************************
 * Top Level of the application.											        *
 ***********************************************************************/

WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
			LPSTR lpCmdLine, int nCmdShow)
{
  TMatchApp MatchApp  ("MatchApp", hInstance, hPrevInstance,
							  lpCmdLine, nCmdShow);
  MatchApp.Run();
  return MatchApp.Status;
}


DEFINE_RESPONSE_TABLE1( TMatchWin, TFrameWindow )
	 EV_COMMAND( 301, CMGameStart ),
	 EV_COMMAND( 302, CMGameSkillLevel ),
	 EV_COMMAND( 303, CMGameResign ),
	 EV_COMMAND( 304, CMGameExit ),
	 EV_COMMAND( 307, CMHelpAbout ),
	 EV_WM_LBUTTONDOWN,
	 EV_WM_MOUSEMOVE,
	 EV_WM_LBUTTONUP,
END_RESPONSE_TABLE;

DEFINE_RESPONSE_TABLE1( TDialogGoesFirst, TDialog )
	 EV_CHILD_NOTIFY_ALL_CODES( 6, Yes ),
	 EV_CHILD_NOTIFY_ALL_CODES( 7, No ),
END_RESPONSE_TABLE;
