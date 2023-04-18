// Match Sticks Engine Header File
// Version 1.0
// Copyright (c) 1989, 1993 by Rachel Bowyer


const int Max_No_Piles = 4;
const int Max_No_Matches = 7;
const int Max_Skill_Level = 4;

typedef int TPosition[Max_No_Piles];
inline void Assign (TPosition&, const TPosition&); //Assigns one position
						      //to another.
const TPosition Start_Position = {7, 5, 3 ,1};
const TPosition End_Position = {0, 0, 0, 0};


//Error Handler.
typedef void (*TPError_Routine)(int, const char[]);

class TErrorHandler
{
  static TPError_Routine PError_Routine;
protected:
  void Error(int err_no, const char err_msg[]) const
  { (*PError_Routine)(err_no, err_msg); };
public:
  TErrorHandler() {};
  TErrorHandler(TPError_Routine Param) { PError_Routine = Param; };
};


struct TMove: public TErrorHandler
{
    int Pile, Matches;
    TMove() {} ;
    void Assign (int, int);
    TMove(int a, int b) { Assign(a, b); } ;
};

_CLASSDEF(TGame);
class TGame: public TErrorHandler
{
    int Skill_Level;
    TPosition Position;
protected:
    void Set_Position(const TPosition&);
public:
    TGame(int Skill = 1);
    ~TGame() {};
    virtual void Reset()  { Set_Position(Start_Position); };
    virtual void Resign() { Set_Position(End_Position); };
    void Play(TMove);
    void Get_Position(TPosition&) const;
    int Get_Skill_Level() const { return(Skill_Level); };
    void Set_Skill_Level(int Skill);
    TMove Get_Best_Move() const;
    inline BOOL Is_Legal(TMove) const;
	 BOOL Has_Won() const { return FALSE; };
    BOOL Has_Lost() const;
};


