// Match Sticks Engine
// Version 1.0
// Copyright (c) 1989, 1993 by Rachel Bowyer


#include <stdlib.h>
#include <iostream.h>
#include <time.h>
#include <windows.h>

//enum BOOL { False, True };
#include "matcheng.h"
#define False FALSE
#define True TRUE


const int No_Win_Pos = 18;
const TPosition Winning_Positions[No_Win_Pos] ={
       {1,0,0,0}, {1,1,1,0}, {2,2,0,0}, {3,3,0,0}, {3,2,1,0}, {4,4,0,0},
       {5,5,0,0}, {2,2,1,1}, {3,3,1,1}, {4,4,1,1}, {5,5,1,1}, {5,4,1,0},
       {6,4,2,0}, {6,5,3,0}, {6,4,3,1}, {7,5,2,0}, {6,5,2,1}, {7,4,2,1}
      };
const int Depth_of_Search[Max_Skill_Level] = {4, 10, 13, 17};


//Error Handler + Routine Defined.
void Error_Routine(int err_no, const char err_msg[])
{
     cout << "Run Time Error " << err_no << ": " << err_msg << "\n";
     exit(3);                                         
}

TPError_Routine TErrorHandler::PError_Routine = Error_Routine;

//Function for TPosition
inline void Assign(TPosition& Pos1, const TPosition& Pos2)
{
    Pos1[0] = Pos2[0]; Pos1[1] = Pos2[1];
    Pos1[2] = Pos2[2]; Pos1[3] = Pos2[3];
}

//Methods for TMove
void TMove::Assign(int p, int m)
{
    Pile = p; Matches = m;

#ifdef debug
    if (Pile < 0 || Pile > Max_No_Piles - 1)  
       Error(1, "Wrong Number of Piles");
    if (Matches < 0 || Matches > Max_No_Matches ) 
       Error(2, "Wrong Number of Matches");
#endif
}


//Methods for TGame
TGame::TGame(int Skill)
{
    //Seed Random Number Generator with time in sec since 1970.
    srand((int) time(NULL));  

    Skill_Level = Skill;
    Assign(Position, Start_Position);

#ifdef debug
    if (Skill<1 || Skill >Max_Skill_Level)
    Error(3, "Wrong Skill Level");
#endif
}

void TGame::Set_Position(const TPosition& Pos)
{
     Assign(Position, Pos);

#ifdef debug
    int i;
    for (i=0; i<= Max_No_Piles - 1; i++) 
      if (Pos[i]<0 || Pos[i] > Start_Position[i]) 
      Error(4, "Wrong position");
#endif
}

inline BOOL TGame::Is_Legal(TMove Move) const
{
  return BOOL(Position[Move.Pile] >= Move.Matches && Move.Matches >= 1);
}
    

void TGame::Play(TMove Move)
{
#ifdef debug
    if ( !Is_Legal(Move) ) Error(5, "Illegal Move");
#endif

    Position[Move.Pile]-= Move.Matches;
}


void TGame::Set_Skill_Level(int Skill)
{
    Skill_Level= Skill;

#ifdef debug
    if (Skill<1 || Skill >Max_Skill_Level)
    Error(3, "Wrong Skill Level");  
#endif
}

void TGame::Get_Position(TPosition& Pos) const
{
    Assign(Pos, Position);
}

BOOL TGame::Has_Lost() const
{
     int Res;
     Res = !Position[0] && !Position[1];
     Res = Res && !Position[2] && !Position[3];
     return BOOL(Res);
}


int cmp(const void* Item1, const void* Item2)
{
    int res = * (int*) Item2 - * (int*) Item1;
return  res;
}

BOOL cmpfp(const TPosition& Pos1, const TPosition& Pos2)
{
	 int Equal = (int) True;
    int i;
    for (i = 0; i<= Max_No_Piles - 1; ++i)
      Equal = Equal && (Pos1[i] == Pos2[i]);

return (BOOL) Equal;
}

inline int Min(int a, int b) { return (a > b) ? b : a; }

TMove TGame::Get_Best_Move() const
{
    int i, j, k, r;
    TPosition Trial_Pos;
    BOOL Found;
    int Found_Pile, Found_No;
    TMove Move(0, 0);

#ifdef debug
  if ( Has_Lost() )
  {
    Error(6, "Game is finished");
    return (Move);
  }
#endif

    //We try every move that we can do.
    Found = False;
    for (i = 0; i <= Max_No_Piles - 1; ++i)
    for (j = 1; j <= Position[i]; ++j)
    {
        Assign(Trial_Pos, Position);
	Trial_Pos[i]-= j;

	//Arrange position in numerical order. 
	qsort(Trial_Pos, Max_No_Piles, sizeof(Trial_Pos[0]), cmp);

	//Search table of winning positions for this position.
	//Note we are handicaped by the skill level.
        for (k = 0; k <= Depth_of_Search[Skill_Level - 1]; ++k)       
        if ( (int) cmpfp(Trial_Pos, Winning_Positions[k]) )
        {
             Found = True; Found_Pile = i; Found_No = j;
	}

    }

    //But what if there is no winning move ?  We have to bluff and
    //and pretend to play a good move.
    if (!int(Found))
	 {
         //Choose any pile with matches left in it.
	 while (Position[ i = rand()%(Max_No_Piles) ] == 0) {};
    
	 //Take a random no of matches from this pile, but we are
         //allowed to take more on a lower skill level.
         r = Min(Max_Skill_Level - Skill_Level +1 , Position[i]);
         j = rand()%r + 1;  //j=0..r


         Found = True; Found_Pile = i; Found_No =j; 
    }

    //We now have our move.
    Move.Assign(Found_Pile, Found_No);

return Move;
}
