#ifndef _gospel_h
# define _gospel_h

# include <stdio.h>
# include <stddef.h>
# include <except.h>
# include <X11/Intrinsic.h>
# include "GoBoard.h"

# define SGFEXTENSION     "sgf"
# define PSEXTENSION      "ps"
# define SGFGO            1

typedef enum {
    mtSETUP, mtBLACKMOVE, mtWHITEMOVE, mtBLACKPASS, mtWHITEPASS,
    mtBLACKUNDO, mtWHITEUNDO
} MoveType;
    
typedef struct stonelist {
    struct stonelist *Next;
    int               x, y;
    char              Color;
} StoneList; 

typedef struct _CommentFun CommentFun;

struct _CommentFun {
    void (*Free)(XtPointer comment);
    int  (*StringRep)(CommentFun *Fun, XtPointer com, int Entry, size_t *pos,
                      char **string, size_t length);
};

typedef struct _Node Node;
typedef int          NodePos;

typedef struct {
    size_t SizeX, SizeY, Nodes;
    char **Board;
    Node  *Initial, *Current;
    int    AllowSuicide;
} Gamelog;

extern Exception ErrNode;
extern Exception OutOfBounds;
extern Exception Occupied;
extern Exception InvalidColor;
extern Exception Suicide;
extern Exception UpFromTop;
extern Exception DownFromBottom;
extern Exception NotAGoMove;

# define NodeNumber(game)        (*(int *) (game)->Current)
# define NumberNodes(game)       ((game)->Nodes)
# define GamelogToBoard(game)    ((game)->Board)
# define FindComment(game, fun)  (*Findcomment(game, fun))
# define GetStone(game, x, y)    (*(char *) &(game)->Board[y][x])

extern void       InitGospel(void);
extern void       CleanGospel(void);
extern void       NewGameProperty(const char *Name, const char *SgfName);
extern XtPointer  AddLocalProperty(Gamelog *game, const char *Name,
                                   const char *Text, int Length);
extern XtPointer  AddLocalProperties(Gamelog *game, const char *Name,
                                     int NrEntries,
                                     const char **Text, int *Length);
extern XtPointer  AddGlobalProperty(Gamelog *game, const char *Name,
                                    const char *Text, int Length);
extern void       DeleteGlobalProperty(Gamelog *game, const char *Name);
extern int        GetLocalProperty(const Gamelog *game, const char *Name,
                                   int Nr, int entry, size_t *pos, 
                                   char **string, size_t length);
extern int        GetGlobalProperty(const Gamelog *game, const char *Name,
                                    int Nr, int entry, size_t *pos, 
                                    char **string, size_t length);
extern Gamelog   *AllocGamelog(size_t sizeX, size_t sizeY, int AllowSuicide);
extern void       FreeGamelog(Gamelog *game);
extern void       FreeStones(StoneList *stones);
extern XtPointer  AddComment(Gamelog *game, CommentFun *fun,
                             XtPointer comment);
extern XtPointer  AddTextComment(Gamelog *game, const char *Text, int Length);
extern XtPointer *Findcomment(Gamelog *game, CommentFun *fun);
extern void       SetGamelog(Gamelog *game, char **board);
extern void       UpGamelog(Gamelog *game);
extern void       DownGamelog(Gamelog *game);
extern int        DeleteNode(Gamelog *game);
extern int        DoMove(Gamelog *game, int x, int y, BWPiece Who);
extern void       DoPass(Gamelog *game, BWPiece Who);
extern void       PositionToNode(Gamelog *game, char **board, int Color);
extern void       SetStones(Gamelog *game, StoneList *stones);
extern int        RemoveGroupFromStone(Gamelog *game, int x, int y);
extern void       PrintBoard(FILE *To, Gamelog *game);
extern void       GoMoveFromXY(char *Move, int x, int y);
extern void       GoXYFromMove(int *x, int *y, const char *Move);
extern void       WriteSgf(FILE *fp, Gamelog *game);
extern XtPointer  WriteSgfFun(FILE *fp, XtPointer Closure);
extern void       WritePostscript(FILE *fp, Gamelog *game);
extern XtPointer  WritePsFun(FILE *fp, XtPointer Closure);
#endif /* _gospel_h */
