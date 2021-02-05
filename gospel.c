#include <string.h>
#include <mymalloc.h>
#include <stddef.h>
#include "gospel.h"
#include "games.h"

#define INT_TO_XTPOINTER(x) ((XtPointer) (long) (x))

#define Strcmp(rc, str1, len1, str2, len2)      \
do {                                            \
    size_t len;                                 \
                                                \
    if (len1 < len2) len = len1;                \
    else             len = len2;                \
    rc = memcmp(str1, str2, len);               \
    if (rc == 0) rc = len1-len2;                \
} while(0)

typedef struct _Comment   Comment;
typedef struct _NodeEntry NodeEntry;
typedef struct _PropEntry PropEntry;

struct _NodeEntry {
    NodeEntry *Next, *Previous;
    /* Move:   Contains the move connecting the states, NULL if it was not a
               move. Is assumed to contain at most one move.
       Add :   Stones that appear (replace)
       Delete: Stones that disappear (get replaced) */
    StoneList *Move, *Add, *Delete; 
    Node      *Entry;
};

struct _Comment {
    Comment    *Next, *Previous;
    CommentFun *Fun;
    XtPointer   Comment;
};

struct _Node {
    NodePos           Pos;      /* Must be first for current NodeNumber def */
    NodeEntry         Entries;  /* Circular double linked list with sentinel.
                                   All point to a variation, except sentinel
                                   sentinel which points to parent ! */
    Comment           Comments;
};

struct _PropEntry {
    PropEntry  *Left, *Right;
    char       *Name, *SgfName;
    size_t      NameLength, SgfNameLength;
    CommentFun  Fun;
};

typedef struct _MultiProp {
    int    NrEntries;
    int   *Lengths;
    char **Entries;
} MultiProp;

static PropEntry *PropEntries = NULL;

Exception GamePropertyNotFound  = { "Gameproperty" };
Exception GamePropertyNotEnough = { "Gameproperty" };
Exception GamePropertyExists = { "Gameproperty already exists:" };
Exception ErrNode            = { "Call to Go node Errorfunction" };
Exception OutOfBounds        = { "Considering a stone outside the board" };
Exception Occupied           = { "Position already occupied" }; 
Exception InvalidColor       = { "Color should be either black or white" };
Exception Suicide            = { "Stone attempts suicide" };
Exception UpFromTop          = { "Attempt to go up from top of gamelog" };
Exception DownFromBottom     = { "Attempt to go down from bottom of gamelog" };
Exception NotAGoMove         = { "Invalid move description" };
Exception WriteSgfException  = { "Could not write to sgf file",
                                     NULL, ErrnoExceptionAction };
Exception WritePsException   = { "Could not write to postscriptfile",
                                     NULL, ErrnoExceptionAction };

static void FreePtr(XtPointer data)
{
    myfree((void *) data);
}

static const char CommentName[] = "Comment";

static int StringGoComment(CommentFun *Fun, XtPointer com, int entry,
                           size_t *pos, char **string, size_t length)
{
    char  *comment;
    size_t comLength;

    if      (entry == -1) return 1;
    else if (entry ==  0) {
        comment   = (char *) CommentName+*pos;
        comLength = sizeof(CommentName)-1-*pos;
    } else {
        comment   = (char *) com + *pos;
        comLength = strlen(comment);
    }

    if (*string)
        if (length < comLength) {
            memcpy(*string, comment, length);
            *pos += length;
            return 1;
        } else {
            memcpy(*string, comment, comLength);
            *pos += comLength;
        }
    else {
        if (length) *string = comment;
        else        *string = mystrndup(comment, comLength);
        *pos += comLength;
    }
    return 0;
}

static int StringProperty(CommentFun *Fun, XtPointer com, int entry,
                          size_t *pos, char **string, size_t length)
{
    char       *comment;
    size_t      comLength;
    PropEntry  *Entry;
    MultiProp  *Prop;

    Prop = (MultiProp *) com;
    if      (entry == -1) return Prop->NrEntries;
    else if (entry ==  0) {
        Entry = (PropEntry *) ((char *) Fun - offsetof(PropEntry, Fun));
        comment   = (char *) Entry->SgfName + *pos;
        comLength = Entry->SgfNameLength - *pos;
    } else {
        entry--;
        comment   = Prop->Entries[entry] + *pos;
        comLength = Prop->Lengths[entry] - *pos;
    }

    if (*string)
        if (length < comLength) {
            memcpy(*string, comment, length);
            *pos += length;
            return 1;
        } else {
            memcpy(*string, comment, comLength);
            *pos += comLength;
        }
    else {
        if (length) *string = comment;
        else        *string = mystrndup(comment, comLength);
        *pos += comLength;
    }
    return 0;
}

static void ErrorFree(XtPointer comment)
{
    Raise(ErrNode);
}

static CommentFun GoComment = {
    FreePtr,
    StringGoComment
};

static CommentFun GameProperty = {
    FreePtr,
    StringProperty
};

static const char Letters[]= "ABCDEFGHJKLMNOPQRSTUVWXYZ"; /* No I */
static const char Digits[] = "0123456789";

static CommentFun ErrorFun = {
    ErrorFree,
    NULL
};

void NewGameProperty(const char *Name, const char *SgfName)
{
    PropEntry **Here, *Ptr;
    size_t      NameLength, SgfNameLength;
    int         rc;
    char        Buffer[2048];

    NameLength    = strlen(Name);
    SgfNameLength = strlen(SgfName);
    Here = &PropEntries;
    while ((Ptr = *Here) != NULL) {
        Strcmp(rc, Name, NameLength, Ptr->Name, Ptr->NameLength);
        if      (rc < 0) Here = &Ptr->Left;
        else if (rc > 0) Here = &Ptr->Right;
        else { 
            Strcmp(rc, SgfName, SgfNameLength,
                   Ptr->SgfName, Ptr->SgfNameLength);
            if (rc == 0) return;
            rc = (sizeof(Buffer)-100)/3;
            sprintf(Buffer, "trying to add property %.*s with sgf name %.*s "
                    "while it already exists with sgf name %.*s",
                    rc, Name, rc, SgfName, rc, Ptr->SgfName);
            Raise1(GamePropertyExists, ExceptionCopy(Buffer));
        }
    }
    Ptr = mynew(PropEntry);
    WITH_HANDLING {
        Ptr->Left = Ptr->Right = NULL;
        Ptr->Name = Ptr->SgfName = NULL;
        Ptr->Name          = mystrndup(Name,    NameLength);
        Ptr->SgfName       = mystrndup(SgfName, SgfNameLength);

        Ptr->NameLength    = NameLength;
        Ptr->SgfNameLength = SgfNameLength;
        Ptr->Fun           = GameProperty;
        
        *Here = Ptr;
    } ON_EXCEPTION {
        myfree(Ptr->SgfName);
        myfree(Ptr->Name);
    } END_HANDLING;
}

static const char *PropTable[] = {
    "NodeName",         "N",         /* "Nodename",         */
    "NodeValue",        "V",	     /* "nodeValue",        */
    "Name",             "GN",	     /* "GameName",         */
    "Date",             "DT",        /* "DaTe",             */
    "Place",            "PC",	     /* "PlaCe",            */
    "Comment",          "C",	     /* "Comment",          */
    "Copyright",        "CR",	     /* "CopyRight",        */
    "EnteredBy",        "US",	     /* "USer",             */
    "Result",           "RE",	     /* "REsult",           */
    "Source",           "SO",	     /* "SOurce",           */
    "Tournament",       "EV",	     /* "EVent",            */
    "Round",            "RO",	     /* "ROund",            */
    "PositionMarks",    "M",	     /* "Marks",            */
    "PositionLetters",  "L",	     /* "Letters",          */
    "PositionSelected", "SL",	     /* "SeLected",         */
    "BlackName",        "PB",	     /* "PlayerBlack",      */
    "WhiteName",        "PW",	     /* "PlayerWhite",      */
    "BlackStrength",    "BR",	     /* "BlackRank",        */
    "WhiteStrength",    "WR",	     /* "WhiteRank",        */
    "Handicap",         "HA",	     /* "HAndicap",         */
    "Komi",             "KM",	     /* "KoMi",             */
    "BlackTerritory",   "TB",        /* "TerritoryBlack",   */
    "WhiteTerritory",   "TW",        /* "TerritoryWhite",   */
    "Region",           "RG",	     /* "ReGion",           */
    "SecureStones",     "SC",	     /* "SeCurestones",     */
    "TimeLimit",        "TM",	     /* "TiMe",             */
    "TimeLeftBlack",    "BL",        /* "BlacktimeLeft",    */
    "TimeLeftWhite",    "WL",        /* "WhitetimeLeft",    */
    "Figure",           "FG",	     /* "FiGure",           */
    "FirstPlayer",      "PL",	     /* "PLayerfirst",      */
    "BlackSpecies",     "BS",	     /* "BlackSpecies",     */
    "WhiteSpecies",     "WS",	     /* "WhiteSpecies",     */
    "Evaluation",       "EL",	     /* "EvaLuation",       */
    "ExpectedMove"      "EX"	     /* "EXpectedmove"      */
    "CheckMark",        "CH",	     /* "CHeckmark",        */
    "GoodForBlack",     "GB",	     /* "GoodforBlack",     */
    "GoodForWhite",     "GW",	     /* "GoodforWhite",     */
    "GoodMove",         "TE",	     /* "TEsuji",           */
    "BadMove",          "BM",        /* "BadMove",          */
};

void InitGospel(void)
{
    int i;
    const char **Here, *Ptr1, *Ptr2;

    PropEntries = NULL;
    Here = PropTable;
    for (i= sizeof(PropTable)/sizeof(*PropTable)/2; i>0; i--) {
        Ptr1 = *Here++;
        Ptr2 = *Here++;
        NewGameProperty(Ptr1, Ptr2);
    }
}

static void ClearEntries(PropEntry *Entry)
{
    if (Entry->Left)  ClearEntries(Entry->Left);
    if (Entry->Right) ClearEntries(Entry->Right);
    myfree(Entry);
}

void CleanGospel(void)
{
    if (PropEntries) {
        ClearEntries(PropEntries);
        PropEntries = NULL;
    }
}

void GoXYFromMove(int *x, int *y, const char *Move)
{
    char *ptr;

    ptr = strchr(Letters, Move[0]);
    /* This test may be too aggressive */
    if (ptr && '1'<= Move[1] && Move[1] <= '9' &&
        (Move[2] == 0 || (Move[3] == 0 && '0'<= Move[2] && Move[2] <= '9'))) {
        *x = ptr-Letters;
        *y = Move[1]-'0';
        if (Move[2]) *y = *y*10+Move[2]-'0';
        (*y)--;
    } else {
	Raise1(NotAGoMove, ExceptionCopy(Move));
    }
}

void GoMoveFromXY(char *Move, int x, int y)
{
    /* This test may be too aggressive */
    if (0 <= x && x <= 25 && 0 <= y && y <= 98) {
        y++;
        Move[0] = Letters[x];
        if (y<10) {
            Move[1] = Digits[y];
            Move[2] = 0;
        } else {
            Move[1] = Digits[y/10];
            Move[2] = Digits[y%10];
            Move[3] = 0;
        }
    } else Raise(NotAGoMove);
}

Gamelog *AllocGamelog(size_t sizeX, size_t sizeY, int AllowSuicide)
{
    Gamelog *game;
    char   **board;
    Node    *node;

    board = AllocBoard(sizeX, sizeY);

    WITH_HANDLING {
        game = mynew(Gamelog);
        WITH_HANDLING {
            node = (Node *) mymalloc(sizeof(Node));
    
            node->Entries.Previous  = node->Entries.Next = &node->Entries;
            node->Entries.Move      = NULL;
            node->Entries.Add       = node->Entries.Delete = NULL;
            node->Entries.Entry     = NULL;

            node->Pos               = 0;
            node->Comments.Previous = node->Comments.Next = &node->Comments;
            node->Comments.Fun      = &ErrorFun;
            node->Comments.Comment  = (XtPointer) mtSETUP;

            game->AllowSuicide = AllowSuicide;
            game->SizeX        = sizeX;
            game->SizeY        = sizeY;
            game->Board        = board;
            game->Nodes        = 1;
            game->Initial = game->Current = node;
        } ON_EXCEPTION {
            myfree(game);
        } END_HANDLING;
    } ON_EXCEPTION {
        myfree(board);
    } END_HANDLING;

    return game;
}

typedef struct {
    int    x, y;
} Pos;

#define USED     0x80  /* Should work if chars are at least 8 bit */

#define TestPos()                         \
do {                                      \
    Test = board[y][x];                   \
    if ((Test & USED) == 0) {             \
        if (Test == Empty) {              \
            VeryLast = Last;              \
            Raise(NoTake);                \
        } else if (Test == Color) {       \
            Last++;                       \
            Last->x = x;                  \
            Last->y = y;                  \
            board[y][x] = Test | USED;    \
        }                                 \
    }                                     \
} while(0)

static Pos *VeryLast;

/* Essentially a floodfill algorithm */
static StoneList *Captured(char **board, StoneList *captured,
                           int x, int y, int maxX, int maxY)
{
    static Exception NoTake = { "No take. You should never see this" };

    StoneList * volatile Cap;
    StoneList *newstone, *oldlist;
    Pos       *Flow, *Last, *Here;
    char       Color, Test;
    
    Here = Last = Flow = mynews(Pos, maxX * maxY);
    Here->x     = x;
    Here->y     = y;
    Color       = board[y][x];
    board[y][x] = Color | USED;
    Cap = oldlist     = captured;

    WITH_HANDLING {
        while (Here <= Last) {
            x = Here->x;
            y = Here->y;
            Here++;

            y++;                /* Go up */
            if (y < maxY) TestPos();
            y -= 2;             /* Go down */ 
            if (0 <= y) TestPos();
            y++;                /* And back to old pos */

            x++;                /* Go right */
            if (x < maxX) TestPos();
            x -= 2;             /* Go left */ 
            if (0 <= x) TestPos();
            x++;                /* And back to old pos */
        }
        for (Here = Flow; Here <= Last; Here++) {
            board[Here->y][Here->x] &= ~USED;
            newstone = mynew(StoneList);
            newstone->x     = Here->x;
            newstone->y     = Here->y;
            newstone->Color = Color;
            newstone->Next  = Cap;
            Cap             = newstone;
        }
    } ON_EXCEPTION {
        if (ExceptionP(NoTake)) {
            Last = VeryLast;
            for (Here = Flow; Here <= Last; Here++) {
                board[Here->y][Here->x] &= ~USED;
            }
            ClearException();
        } else {
            while (Cap != oldlist) {
                newstone = Cap->Next;
                myfree(Cap);
                Cap = newstone;
            }
        }
    } END_HANDLING;
    myfree(Flow);
    
    return Cap;
}
#undef TestPos

#define TestPos()                         \
do {                                      \
    Test = board[y][x];                   \
    if (Test == Color) {                  \
        Last++;                           \
        Last->x = x;                      \
        Last->y = y;                      \
        board[y][x] = Test | USED;        \
    }                                     \
} while(0)

/* Essentially a floodfill algorithm */
static StoneList *GroupFromStone(char **board, StoneList *extend,
                                 int x, int y, int maxX, int maxY)
{
    StoneList *newstone;
    Pos       *Flow, *Last, *Here;
    char       Color, Test;
    
    Here = Last = Flow = mynews(Pos, maxX * maxY);
    Here->x     = x;
    Here->y     = y;
    Color       = board[y][x];
    board[y][x] = Color | USED;

    while (Here <= Last) {
        x = Here->x;
        y = Here->y;
        Here++;

        y++;                    /* Go up */
        if (y < maxY) TestPos();
        y -= 2;                 /* Go down */ 
        if (0 <= y) TestPos();
        y++;                    /* And back to old pos */

        x++;                    /* Go right */
        if (x < maxX) TestPos();
        x -= 2;                 /* Go left */ 
        if (0 <= x) TestPos();
        x++;                    /* And back to old pos */
    }
    for (Here = Flow; Here <= Last; Here++) {
        board[Here->y][Here->x] &= ~USED;
        newstone = mynew(StoneList);
        newstone->x     = Here->x;
        newstone->y     = Here->y;
        newstone->Color = Color;
        newstone->Next  = extend;
        extend = newstone;
    }
    myfree(Flow);
    
    return extend;
}
#undef TestPos

void FreeStones(StoneList *stones)
{
    StoneList *Here, *Next;

    for (Here = stones; Here; Here = Next) {
        Next = Here->Next;
        myfree(Here);
    }
}

static Node *NewNode(Gamelog *game,
                     StoneList *move, StoneList *add, StoneList *delete)
{
    NodeEntry *newEntry;
    Node      *newNode, *oldNode;

    oldNode  = game->Current;
    newNode  = mynew(Node);
    WITH_HANDLING {
        newNode->Entries.Next     = newNode->Entries.Previous =
            &newNode->Entries;
        newNode->Entries.Entry    = oldNode;
        newNode->Entries.Move     = move;
        newNode->Entries.Add      = delete;
        newNode->Entries.Delete   = add;

        newNode->Pos              = oldNode->Pos+1;
        newNode->Comments.Next    = newNode->Comments.Previous =
            &newNode->Comments;
        newNode->Comments.Comment = (XtPointer) mtSETUP;
        newNode->Comments.Fun     = &ErrorFun;

        newEntry = mynew(NodeEntry);
        newEntry->Next     =  oldNode->Entries.Next;
        newEntry->Previous = &oldNode->Entries;
        newEntry->Next->Previous = newEntry->Previous->Next = newEntry;
        newEntry->Move     = move;
        newEntry->Add      = add;
        newEntry->Delete   = delete;
        newEntry->Entry    = newNode;

        game->Current = newNode;
        game->Nodes++;
    } ON_EXCEPTION {
        myfree(newNode);
    } END_HANDLING;
    return newNode;
}

int DoMove(Gamelog *game, int x, int y, BWPiece Who)
/* Returns the number of captured black stones, negative if the captured
 * stones are white.
 */
{
    StoneList * volatile delete;
    StoneList *move, *temp;
    char     **board, OppColor, *Move;
    int        maxX, maxY;
    int        captures = 0;

    if (x<0 || y<0 || game->SizeX <= x || game->SizeY <= y) {
        char Buffer[80];

        sprintf(Buffer, "(%d, %d) is outside a (%d,%d) board",
                x, y, game->SizeX, game->SizeY);
        Raise1(OutOfBounds, Buffer);
    }
    OppColor    = OpponentColor(Who);
    if (OppColor == Empty) Raise(InvalidColor);

    board   = game->Board;
    maxX    = game->SizeX;
    maxY    = game->SizeY;

    Move = &board[y][x];
    if (*Move != Empty) Raise(Occupied);
    *Move = Who;
    WITH_HANDLING {
        delete      = NULL;
        WITH_HANDLING {
            y++;                /* Go up */
            if (y < maxY && board[y][x] == OppColor)
                delete = Captured(board, delete, x, y, maxX, maxY);
            y -= 2;             /* Go down */ 
            if (0 <= y   && board[y][x] == OppColor)
                delete = Captured(board, delete, x, y, maxX, maxY);
            y++;                /* And back to old pos */

            x++;                /* Go right */
            if (x < maxX && board[y][x] == OppColor)
                delete = Captured(board, delete, x, y, maxX, maxY); 
            x -= 2;             /* Go left */ 
            if (0 <= x   && board[y][x] == OppColor)
                delete = Captured(board, delete, x, y, maxX, maxY);
            x++;

            /* if (!delete && (temp = Captured(board, NULL, x, y, maxX, maxY))) { */

            if (!delete) {
                delete = Captured(board, NULL, x, y, maxX, maxY);
                if (delete && !game->AllowSuicide) Raise(Suicide);
            }

            move = mynew(StoneList);
            WITH_HANDLING {
                move->x     = x;
                move->y     = y;
                move->Color = Who;
                move->Next  = NULL;
                NewNode(game, move, NULL, delete)->Comments.Comment =
                    INT_TO_XTPOINTER(Who == White ? mtWHITEMOVE : mtBLACKMOVE);

                for (temp = delete; temp; temp = temp->Next) {
		    captures += (board[temp->y][temp->x] == Black ? 1 : -1);
                    board[temp->y][temp->x] = Empty;
		}
            } ON_EXCEPTION {
                myfree(move);
            } END_HANDLING;
        } ON_EXCEPTION {
            FreeStones(delete);
        } END_HANDLING;
    } ON_EXCEPTION {
        *Move = Empty;
    } END_HANDLING;
    return captures;
}

void DoPass(Gamelog *game, BWPiece Who)
{
    int Type;

    if      (Who == White) Type = mtWHITEPASS;
    else if (Who == Black) Type = mtBLACKPASS;
    else Raise(InvalidColor);

    NewNode(game, NULL, NULL, NULL)->Comments.Comment = INT_TO_XTPOINTER(Type);
}

static void DeleteEntry(Gamelog *game, Node *entry)
{
    NodeEntry *entries,  *nHere, *nNext;
    Comment   *com, *cHere, *cNext;
    void      (*fun)(XtPointer comment);

    entries = &entry->Entries;
    for (nHere = entries->Next; nHere != entries; nHere = nNext) {
        nNext = nHere->Next;
        DeleteEntry(game, nHere->Entry);
        myfree(nHere->Move);
        FreeStones(nHere->Add);
        FreeStones(nHere->Delete);
        myfree(nHere);
    }
    
    com = &entry->Comments;
    for (cHere = com->Next; cHere != com; cHere = cNext) {
        cNext = cHere->Next;
        if (cHere->Fun && (fun = cHere->Fun->Free) != 0)
            (*fun)(cHere->Comment);
        myfree(cHere);
    }
    myfree(entry);
    game->Nodes--;
}

int DeleteNode(Gamelog *game)
/* Returns the number of uncaptured black stones, negative if the uncaptured
 * stones are white.
 */
{
    char      **board;
    Node       *entry;
    NodeEntry  *node;
    StoneList  *add, *delete, *stone;
    int        uncaptured = 0;
   
    entry = game->Current;
    if (!entry->Entries.Entry) Raise(UpFromTop);

    board  = game->Board;
    delete = entry->Entries.Delete;
    add    = entry->Entries.Add;
    /* The order of these loops is important for suicide */
    for (stone = delete; stone; stone = stone->Next) {
        board[stone->y][stone->x] = Empty;
	/* Do not update uncaptured. This should be for undoing the
         * "handicap" move only.
         */
    }
    for (stone = add;    stone; stone = stone->Next) {
        board[stone->y][stone->x] = stone->Color;
	uncaptured += (stone->Color == Black ? 1 : -1);
    }
    stone  = entry->Entries.Move;
    if (stone) board[stone->y][stone->x] = Empty;

    game->Current = entry->Entries.Entry;
    node = game->Current->Entries.Next;
    while (node->Entry != entry) node = node->Next;
    node->Next->Previous = node->Previous;
    node->Previous->Next = node->Next;
    myfree(node);
    DeleteEntry(game, entry);
    return uncaptured;
}

XtPointer AddComment(Gamelog *game, CommentFun *Fun, XtPointer comment)
{
    Comment *New, *Base;

    Base = &game->Current->Comments;
    New = mynew(Comment);
    New->Comment  = comment;
    New->Fun      = Fun;
    New->Next     = Base;
    New->Previous = Base->Previous;
    New->Next->Previous = New->Previous->Next = New;
    return (XtPointer) New;
}

XtPointer AddTextComment(Gamelog *game, const char *Text, int Length)
{
    char      *Copy;
    XtPointer  Result;

    if (Length < 0) Length = strlen(Text);
    Copy = mystrndup(Text, (size_t) Length);
    WITH_HANDLING {
        Result = AddComment(game, &GoComment, (XtPointer) Copy);
    } ON_EXCEPTION {
        myfree(Copy);
    } END_HANDLING;
    return Result;
}

static XtPointer AddProperties(Comment *Base, const char *Name, int NrEntries,
                               const char **Text, int *Length)
{
    PropEntry *Entry;
    char      *Copy, *Ptr;
    Comment   *Result;
    int        i, rc, NameLength, TotalLength;
    size_t    *Len;
    MultiProp *Prop;

    NameLength = strlen(Name);
    Entry = PropEntries;
    while (Entry) {
        Strcmp(rc, Name, NameLength, Entry->Name, Entry->NameLength);
        if (rc < 0) Entry = Entry->Left;
        else if (rc > 0) Entry = Entry->Right;
        else {
            Len = mynews(size_t, NrEntries);
            WITH_UNWIND {
                if (Length) for (i=0; i<NrEntries; i++)
                    if (Length[i] < 0) Len[i] = strlen(Text[i]);
                    else               Len[i] = Length[i];
                else for (i=0; i<NrEntries; i++) Len[i] = strlen(Text[i]); 

                rc = (NrEntries*sizeof(int)+sizeof(char *)-1)/sizeof(char *)*sizeof(char *);
                TotalLength = sizeof(MultiProp)+rc+
                    NrEntries*(1+sizeof(char *));
                for (i=0; i<NrEntries; i++) TotalLength += Len[i];
                Copy = mynews(char, TotalLength);

                Prop = (MultiProp *) Copy;
                Prop->NrEntries = NrEntries;
                Prop->Lengths = (int *) &Prop[1];
                Prop->Entries = (char **) (rc + (char *) Prop->Lengths);
                Ptr = (char *) &Prop->Entries[NrEntries];

                memcpy(Prop->Lengths, Len, NrEntries*sizeof(int));
                for (i=0; i<NrEntries; i++) {
                    Prop->Entries[i] = Ptr;
                    memcpy(Ptr, Text[i], Len[i]);
                    Ptr += Len[i];
                    *Ptr++ = 0;
                }

                WITH_HANDLING {
                    Result = mynew(Comment);
                    Result->Comment  = (XtPointer) Copy;
                    Result->Fun      = &Entry->Fun;
                    Result->Next     = Base;
                    Result->Previous = Base->Previous;
                    Result->Next->Previous = Result->Previous->Next = Result;
                } ON_EXCEPTION {
                    myfree(Copy);
                } END_HANDLING;
            } ON_UNWIND {
                myfree(Len);
            } END_UNWIND;
            return (XtPointer) Result;
        }
    }
    Raise2(GamePropertyNotFound, ExceptionCopy(Name), "does not exist");
    return NULL;
}

XtPointer AddLocalProperty(Gamelog *game, const char *Name,
                           const char *Text, int Length)
{
    return AddProperties(&game->Current->Comments, Name, 1, &Text, &Length);
}

XtPointer AddLocalProperties(Gamelog *game, const char *Name, int NrEntries,
                             const char **Text, int *Length)
{
    return AddProperties(&game->Current->Comments, Name,
                         NrEntries, Text, Length);
}

XtPointer AddGlobalProperty(Gamelog *game, const char *Name,
                            const char *Text, int Length)
{
    return AddProperties(&game->Initial->Comments, Name, 1, &Text, &Length);
}

void DeleteGlobalProperty(Gamelog *game, const char *Name)
{
    PropEntry  *Entry;
    Comment    *Here, *Base, *Ptr;
    int         rc, NameLength;
    void      (*fun)(XtPointer comment);

    NameLength = strlen(Name);
    Entry = PropEntries;
    while (Entry) {
        Strcmp(rc, Name, NameLength, Entry->Name, Entry->NameLength);
        if (rc < 0) Entry = Entry->Left;
        else if (rc > 0) Entry = Entry->Right;
        else {
            Base = &game->Initial->Comments;
            Here = Base->Next;
            while (Here != Base)
                if (Here->Fun == &Entry->Fun) {
                    Ptr = Here;
                    Here = Here->Next;
                    Here->Previous = Ptr->Previous;
                    Here->Previous->Next = Here;
                    if (Ptr->Fun && (fun = Ptr->Fun->Free) != 0)
                        (*fun)(Ptr->Comment);
                    myfree(Ptr);
                } else Here = Here->Next;
            return;
        }
    }
    Raise2(GamePropertyNotFound, ExceptionCopy(Name), "does not exist");
}

static int GetProperty(Comment *Base, const char *Name, size_t NameLength,
                       int Nr, int entry, size_t *pos,
                       char **string, size_t length)
{
    PropEntry   *Entry;
    Comment     *Here;
    int          rc, Result;
    CommentFun  *Fun;

    Entry = PropEntries;
    while (Entry) {
        Strcmp(rc, Name, NameLength, Entry->Name, Entry->NameLength);
        if (rc < 0) Entry = Entry->Left;
        else if (rc > 0) Entry = Entry->Right;
        else {
            Fun = &Entry->Fun;
            if (Nr < 0) {
                Result = 0;
                for (Here = Base->Next; Here != Base; Here = Here->Next)
                    if (Here->Fun == Fun) Result++;
                return Result;
            }
            for (Here = Base->Next; Here != Base; Here = Here->Next)
                if (Here->Fun == Fun && --Nr < 0)
                    return Fun->StringRep(Fun, Here->Comment,
                                          entry, pos, string, length);
            Raise2(GamePropertyNotEnough, ExceptionCopy(Name),
                   "does not exist in that number");
        }
    }
    Raise2(GamePropertyNotFound, ExceptionCopy(Name), "does not exist");
    return -1;
}

int GetLocalProperty(const Gamelog *game, const char *Name, int Nr,
                     int entry, size_t *pos, char **string, size_t length)
{
    return GetProperty(&game->Current->Comments, Name, strlen(Name), Nr,
                       entry, pos, string, length);
}

int GetGlobalProperty(const Gamelog *game, const char *Name, int Nr,
                      int entry, size_t *pos, char **string, size_t length)
{
    return GetProperty(&game->Initial->Comments, Name, strlen(Name), Nr,
                       entry, pos, string, length);
}

XtPointer *Findcomment(Gamelog *game, CommentFun *Fun)
{
    Comment *Base, *Here;
    Base = &game->Current->Comments;
    for (Here = Base->Next; Here != Base; Here = Here->Next)
        if (Here->Fun == Fun) return &Here->Comment;
    return NULL;
}

void FreeGamelog(Gamelog *game)
{
    Node *initial;

    if (game) {
        initial = game->Initial;
        myfree(initial->Entries.Move);
        FreeStones(initial->Entries.Add);
        FreeStones(initial->Entries.Delete);
        DeleteEntry(game, initial);
        FreeBoard(game->Board, game->SizeY);
        myfree(game);
    }
}

void UpGamelog(Gamelog *game)
{
    NodeEntry *entries;
    StoneList *Stone;
    char     **board;

    entries = &game->Current->Entries;
    if (!entries->Entry) Raise(UpFromTop);
    
    board = game->Board;
    /* The order of these loops is important for suicide */
    for (Stone = entries->Delete; Stone; Stone = Stone->Next)
        board[Stone->y][Stone->x] = Empty;
    for (Stone = entries->Add;    Stone; Stone = Stone->Next)
        board[Stone->y][Stone->x] = Stone->Color;
    Stone = entries->Move;
    if (Stone) board[Stone->y][Stone->x] = Empty;

    game->Current = entries->Entry;
}

/* Use first branch */
void DownGamelog(Gamelog *game)
{
    NodeEntry *here, *down;
    StoneList *Stone;
    char     **board;

    here = &game->Current->Entries;
    down = here->Next;
    if (here == down) {
	/* ??? Raise(DownFromBottom); */
	printf("ERROR Raise(DownFromBottom);\n"); fflush(stdout);
	ShowGamelogInfo(game);

	game->Current = down->Entry;
	return;
    }
    board = game->Board;
    /* The order of these loops is important for suicide */
    Stone = down->Move;
    if (Stone) board[Stone->y][Stone->x] = Stone->Color;
    for (Stone = down->Delete; Stone; Stone = Stone->Next)
        board[Stone->y][Stone->x] = Empty;
    for (Stone = down->Add;    Stone; Stone = Stone->Next)
        board[Stone->y][Stone->x] = Stone->Color;

    game->Current = down->Entry;
}

void PositionToNode(Gamelog *game, char **board, int Color)
{
    char     **OldBoard, OldColor, NewColor;
    int        x, y, xSize, ySize;
    StoneList *add, *delete, *extra;
    Node      *node;
    
    OldBoard = game->Board;
    xSize    = game->SizeX;
    ySize    = game->SizeY;
    add = delete = NULL;
    WITH_HANDLING {
        for (y=0; y<ySize; y++)
            for (x=0; x<xSize; x++) {
                OldColor = OldBoard[y][x];
                NewColor =    board[y][x];
                if (OldColor != NewColor) {
                    if (OldColor != Empty) {
                        extra = mynew(StoneList);
                        extra->Next  = delete;
                        extra->Color = OldColor;
                        extra->x     = x;
                        extra->y     = y;
                        delete = extra;
                    }
                    if (NewColor != Empty) {
                        extra = mynew(StoneList);
                        extra->Next  = add;
                        extra->Color = NewColor;
                        extra->x     = x;
                        extra->y     = y;
                        add = extra;
                    }
                }
            }
        node = NewNode(game, NULL, add, delete);
        if (Color != Empty)
            node->Comments.Comment =
                INT_TO_XTPOINTER(Color == Black ? mtBLACKUNDO : mtWHITEUNDO);
        for (extra = delete; extra; extra = extra->Next) {
            OldBoard[extra->y][extra->x] = Empty;
        }
        for (extra = add;    extra; extra = extra->Next) {
            OldBoard[extra->y][extra->x] = extra->Color;
        }
    } ON_EXCEPTION {
        FreeStones(add);
        FreeStones(delete);
    } END_HANDLING;
}

/* Still leaves duplicate sets though */
void SetStones(Gamelog *game, StoneList *stones)
{
    char     **OldBoard, OldColor, NewColor;
    int        x, y, xSize, ySize;
    StoneList *add, *delete, *extra, *stone;
    
    OldBoard = game->Board;
    xSize    = game->SizeX;
    ySize    = game->SizeY;
    add = delete = NULL;
    WITH_HANDLING {
        for (stone = stones; stone; stone = stone->Next) {
            x = stone->x;
            y = stone->y;
            OldColor = OldBoard[y][x];
            NewColor = stone->Color;
            if (OldColor != NewColor) {
                if (OldColor != Empty) {
                    extra = mynew(StoneList);
                    extra->Next  = delete;
                    extra->Color = OldColor;
                    extra->x     = x;
                    extra->y     = y;
                    delete = extra;
                }
                if (NewColor != Empty) {
                    extra = mynew(StoneList);
                    extra->Next  = add;
                    extra->Color = NewColor;
                    extra->x     = x;
                    extra->y     = y;
                    add = extra;
                }
            }
        }
        NewNode(game, NULL, add, delete);
        for (extra = delete; extra; extra = extra->Next) {
            OldBoard[extra->y][extra->x] = Empty;
        }
        for (extra = add;    extra; extra = extra->Next) {
            OldBoard[extra->y][extra->x] = extra->Color;
        }
    } ON_EXCEPTION {
        FreeStones(add);
        FreeStones(delete);
    } END_HANDLING;
}

int RemoveGroupFromStone(Gamelog *game, int x, int y)
/* Returns the number of removed black stones, negative if the removed
 * stones are white.
 */
{
    char      **Board;
    int         xSize, ySize;
    StoneList  *Stones, *Stone;
    int         removed = 0;

    Board = game->Board;
    xSize = game->SizeX;
    ySize = game->SizeY;
    Stones = GroupFromStone(Board, NULL, x, y, xSize, ySize);
    WITH_HANDLING {
        NewNode(game, NULL, NULL, Stones);
        for (Stone = Stones; Stone; Stone = Stone->Next) {
	    removed += (Board[Stone->y][Stone->x] == Black ? 1 : -1);
            Board[Stone->y][Stone->x] = Empty;
	}
    } ON_EXCEPTION {
        FreeStones(Stones);
        ClearException();
    } END_HANDLING;
    return removed;
}

void SetGamelog(Gamelog *game, char **board)
{
    int    y, maxY;
    size_t maxX;
    char **oldboard;

    maxX     = game->SizeX;
    maxY     = game->SizeY;
    oldboard = game->Board;

    for (y=0; y<maxY; y++) memcpy(oldboard[y], board[y], maxX);
}

void PrintBoard(FILE *To, Gamelog *game)
{
    char **board;
    int    maxX, x, y;
    
    board = game->Board;
    maxX = game->SizeX;

    for (y = game->SizeY-1; y>=0; y--) {
        fprintf(To, "\n%2d ", y);
        for (x=0; x<maxX; x++) {
            fputc(board[y][x] & USED ? 'u' : ' ', To);
            switch(board[y][x] & ~USED) {
              case White:
                fputc('O', To);
                break;
              case Black:
                fputc('X', To);
                break;
              case Empty:
                fputc(' ', To);
                break;
              default: /* This must be an error */
                fputc('?', To);
                break;
            }
        }
    }
    fprintf(To, "\n   %.*s\n", 2*maxX,
            " A B C D E F G H J K L M N O P Q R S T U V W X Y Z");
}

#define Putc(ch, fp)                                    \
do {                                                    \
    if (EOF == putc(ch, fp)) Raise(EXCEPTION);          \
} while(0)

#define Puts(str, fp)                                   \
do {                                                    \
    if (EOF == fputs(str, fp)) Raise(EXCEPTION);        \
} while(0)

#define Printf(x)                                       \
do {                                                    \
    if (fprintf x < 0) Raise(EXCEPTION);                \
} while(0)

#define Write(str, len, fp)                             \
do {                                                    \
    if (len != fwrite(str, sizeof(char), len, fp))      \
        Raise(EXCEPTION);				\
} while(0)

#define EXCEPTION       WriteSgfException
static void WriteComments(FILE *fp, Comment *Comments)
{
    Comment *Com;
    char    *Ptr, *Here;
    int      i, j, Nr, More;
    size_t   pos, OldPos, Length;
    int    (*Fun)(CommentFun *Fun, XtPointer com, int Entry, size_t *pPos,
                  char **string, size_t length);
    int      StringComments;

    for (StringComments = 2; StringComments > 0 ; StringComments--) {
        for (Com = Comments->Next; Com != Comments; Com = Com->Next) {
            Fun = Com->Fun->StringRep;
            if (Fun && (Fun == StringGoComment) != (StringComments == 2)) {
                if (StringComments) {
                    pos = 0;
                    Ptr = NULL;
                    (*Fun)(Com->Fun, Com->Comment, 0, &pos, &Ptr, 1);
                    Write(Ptr, pos, fp);
                }
                Nr = (*Fun)(Com->Fun, Com->Comment, -1, NULL, NULL, 0);
                for (j=1; j<=Nr; j++) {
                    if (StringComments) {
                        Putc('[', fp);
                        if (StringComments == 1) StringComments = 0;
                    } else Putc('\n', fp);
                    pos = 0;
                    do {
                        OldPos = pos;
                        Ptr    = NULL;
                        More = (*Fun)(Com->Fun, Com->Comment, j, &pos, &Ptr, 1);
                        Here = Ptr;
                        for (i=pos - OldPos; i>0; i--, Here++)
                            switch (*Here) {
                              case '[':
                              case ']':
                              case '(':
                              case ')':
                              case '\\':
                                Length = Here - Ptr;
                                Write(Ptr, Length, fp);
                                Putc('\\', fp);
                                Ptr = Here;
                            }
                        Length = Here - Ptr;
                        Write(Ptr, Length, fp);
                    } while(More);
                    if (StringComments) Putc(']',  fp);
                }
                if (StringComments) Putc('\n', fp);
            }
        }
        if (!StringComments) {
            Putc(']', fp);
            Putc('\n', fp);
        }
    }
}

static const char SgfPos[] = "abcdefghijklmnopqrstuvwxyz";

#define WriteStone(fp, x, y)    \
do {                            \
    Putc('[', fp);              \
    Putc(SgfPos[x], fp);        \
    Putc(SgfPos[y], fp);        \
    Putc(']', fp);              \
} while(0)

static void WriteSgfEntry(FILE *fp, char **board, int diffY, NodeEntry *From)
{
    Node      *Base, *Current, * volatile Cur;
    NodeEntry *Entry;
    StoneList *Stone;
    int        Found;

    Entry = From;
    Base  = Entry->Entry->Entries.Entry;
    WITH_UNWIND {
        do {
            Cur = Current = Entry->Entry;
            Putc(';',  fp);
            /* Putc('\n', fp); */

            switch((MoveType) Current->Comments.Comment) {
              case mtWHITEMOVE:
                Puts("W", fp);
                WriteStone(fp, Entry->Move->x, diffY-Entry->Move->y);
                Putc('\n', fp);
                break;
              case mtBLACKMOVE:
                Puts("B", fp);
                WriteStone(fp, Entry->Move->x, diffY-Entry->Move->y);
                Putc('\n', fp);
                break;
              case mtWHITEPASS:
                Puts("W", fp);
                if (diffY < 19) WriteStone(fp, 19, 19);
                else            WriteStone(fp, diffY+1, diffY+1);
                Putc('\n', fp);
                break;
              case mtBLACKPASS:
                Puts("B", fp);
                if (diffY < 19) WriteStone(fp, 19, 19);
                else            WriteStone(fp, diffY+1, diffY+1);
                Putc('\n', fp);
                break;
              case mtWHITEUNDO:
              case mtBLACKUNDO:
              case mtSETUP:
                if (Entry->Delete) {
                    Puts("AE", fp); /* AddEmpty */
                    for (Stone = Entry->Delete; Stone; Stone = Stone->Next)
                        WriteStone(fp, Stone->x, diffY-Stone->y);
                    Putc('\n', fp);
                }

                Found = 0;
                for (Stone = Entry->Add; Stone; Stone = Stone->Next)
                    if (Stone->Color == White) {
                        if (!Found) {
                            Puts("AW", fp); /* AddWhite */
                            Found = 1;
                        }
                        WriteStone(fp, Stone->x, diffY-Stone->y);
                    }
                if (Found && EOF == putc('\n', fp)) Raise(WriteSgfException);

                Found = 0;
                for (Stone = Entry->Add; Stone; Stone = Stone->Next)
                    if (Stone->Color == Black) {
                        if (!Found) {
                            Puts("AB", fp); /* AddBlack */
                            Found = 1;
                        }
                        WriteStone(fp, Stone->x, diffY-Stone->y);
                    }
                if (Found && EOF == putc('\n', fp)) Raise(WriteSgfException);
                break;
              default:
                Raise1(AssertException, "Impossible movetype during sgf save");
            }

            /* The order of these loops is important for suicide */
            Stone = Entry->Move;
            if (Stone) board[Stone->y][Stone->x] = Stone->Color;
            for (Stone = Entry->Delete; Stone; Stone = Stone->Next)
                board[Stone->y][Stone->x] = Empty;
            for (Stone = Entry->Add; Stone; Stone = Stone->Next)
                board[Stone->y][Stone->x] = Stone->Color;

            WriteComments(fp, &Current->Comments);

            Entry = Current->Entries.Next;
            if (Entry->Next != &Current->Entries) {
                do {
                    Putc('(', fp);
                    WriteSgfEntry(fp, board, diffY, Entry);
                    Putc(')', fp);
                    Entry = Entry->Next;
                } while (Entry != &Current->Entries);
                break;
            }
        } while (Entry != &Current->Entries);
    } ON_UNWIND {
        Current = Cur;
        do {
            /* The order of these loops is important for suicide */
            for (Stone = Current->Entries.Delete; Stone; Stone = Stone->Next)
                board[Stone->y][Stone->x] = Empty;
            for (Stone = Current->Entries.Add;    Stone; Stone = Stone->Next)
                board[Stone->y][Stone->x] = Stone->Color;
            Stone = Current->Entries.Move;
            if (Stone) board[Stone->y][Stone->x] = Empty;
            Current = Current->Entries.Entry;
        } while (Current != Base);
    } END_UNWIND;
}

void WriteSgf(FILE *fp, Gamelog *game)
{
    int x, y, SizeX, SizeY;
    char **Board, *B;
    StoneList * volatile WhiteStones, * volatile BlackStones, *Stone;
    Node      *Current;
    NodeEntry *Entry;

    Putc('(',  fp);
    Putc('\n', fp);
    WhiteStones = BlackStones = NULL;
    SizeX = game->SizeX;
    SizeY = game->SizeY;
    /* Change exceptionName. Check on size (sgf<=19, certainly <= 25 -Ton */
    if (SizeX != SizeY)
        Raise1(AssertException, "Sgf format not defined on non square boards");
    Board = game->Board;
    
    Printf((fp, ";\n"
                "GM[%d]\n" /* GaMe */
	                   /* "VieW[]\n" */
                "SZ[%d]\n",/* SiZe */
                 SGFGO, SizeX));
    WriteComments(fp, &game->Initial->Comments);

    SizeX--;
    SizeY--;
    WITH_UNWIND { 
        for (y=SizeY; y>=0; y--, Board++)
            for (x=SizeX, B= *Board+SizeX; x>=0; x--, B--)
                switch(*B) {
                  case White:
                    Stone = mynew(StoneList);
                    Stone->Next = WhiteStones;
                    Stone->x = x;
                    Stone->y = y;
                    WhiteStones = Stone;
                    break;
                  case Black:
                    Stone = mynew(StoneList);
                    Stone->Next = BlackStones;
                    Stone->x = x;
                    Stone->y = y;
                    BlackStones = Stone;
                    break;
                  case Empty:
                    break;
                  default:
                    Raise1(AssertException, "impossible nodevalue");
                }
        if (BlackStones || WhiteStones) {
            Putc(';',  fp);
            Putc('\n', fp);
        }
        if (BlackStones) {
            Puts("AB", fp); /* AddBlack */
            for (Stone = BlackStones; Stone; Stone = Stone->Next)
                WriteStone(fp, Stone->x, Stone->y);
            Putc('\n', fp);
        }
        if (WhiteStones) {
            Puts("AW", fp); /* AddWhite */
            for (Stone = WhiteStones; Stone; Stone = Stone->Next)
                WriteStone(fp, Stone->x, Stone->y);
            Putc('\n', fp);
        }
    } ON_UNWIND { 
        FreeStones(WhiteStones);
        FreeStones(BlackStones);
    } END_UNWIND; 

    Current = game->Current;
    Entry   = Current->Entries.Next;
    Board   = game->Board;
    if (game->Current != game->Initial) WriteComments(fp, &Current->Comments);
    if (Entry->Next != &Current->Entries) {
        do {
            Putc('(', fp);
            WriteSgfEntry(fp, Board, SizeY, Entry);
            Putc(')', fp);
            Entry = Entry->Next;
        } while (Entry != &Current->Entries);
    } else if (Entry != &Current->Entries)
        WriteSgfEntry(fp, Board, SizeY, Entry);

    Putc(')',  fp);
    Putc('\n', fp);
}

XtPointer WriteSgfFun(FILE *fp, XtPointer Closure)
{
    WriteSgf(fp, (Gamelog *) Closure);
    return Closure;
}
#undef EXCEPTION

#define EXCEPTION WritePsException
static void WritePsComments(FILE *fp, Comment *Comments, int MoveNr)
{
    Comment *Com;
    char    *Ptr, *Here, *EPtr;
    int      i, j, Nr, More, NotDone;
    size_t   pos, OldPos, EPos, Length;
    int    (*Fun)(CommentFun *Fun, XtPointer com, int Entry, size_t *pPos,
                  char **string, size_t length);

    NotDone = 1;
    for (Com = Comments->Next; Com != Comments; Com = Com->Next) {
        Fun = Com->Fun->StringRep;
        if (Fun) {
            pos = 0;
            Ptr = NULL;
            (*Fun)(Com->Fun, Com->Comment, 0, &pos, &Ptr, 1);
            EPos = pos;
            EPtr = Ptr;
            if (Fun != StringProperty && NotDone) {
                NotDone = 0;
                Printf((fp, "(%d) %s\n", MoveNr, MoveNr % 2 ? "BC" : "WC"));
            }
            Nr = (*Fun)(Com->Fun, Com->Comment, -1, NULL, NULL, 0);
            for (j=1; j<=Nr; j++) {
                pos = 0;
                Putc('(', fp);
                do {
                    OldPos = pos;
                    Ptr    = NULL;
                    More = (*Fun)(Com->Fun, Com->Comment, j, &pos, &Ptr, 1);
                    Here = Ptr;
                    for (i=pos - OldPos; i>0; i--, Here++)
                        switch (*Here) {
                          case '(':
                          case ')':
                          case '\\':
                            Length = Here - Ptr;
                            Write(Ptr, Length, fp);
                            Putc('\\', fp);
                            Ptr = Here;
                        }
                    Length = Here - Ptr;
                    Write(Ptr, Length, fp);
                } while(More);
                Putc(')', fp);
                Putc(' ', fp);
            }
            if (Fun == StringProperty)
                Printf((fp, "%d %.*s\n", Nr, (int) EPos, EPtr));
            else {
                Write(EPtr, EPos, fp);
                Putc('\n', fp);
            }
        }
    }
    Puts("EC\n", fp);
}

static void WritePsEntry(FILE *fp, char **board, NodeEntry *From,
			 int SaveNr, int MoveNr)
{
    Node      *Base, *Current, * volatile Cur;
    NodeEntry *Entry, *Next;
    StoneList *Stone;

    Entry = From;
    Base  = Entry->Entry->Entries.Entry;
    WITH_UNWIND {
        do {
            Cur = Current = Entry->Entry;

            switch((MoveType) Current->Comments.Comment) {
              case mtWHITEMOVE:
 		MoveNr = (MoveNr+2) & ~1;
		Printf((fp, "%2d %2d (%d) WSM\n",
			Entry->Move->x, Entry->Move->y, MoveNr));
                break;
              case mtBLACKMOVE:
 		MoveNr = ((MoveNr+1) & ~1)+1;
		Printf((fp, "%2d %2d (%d) BSM\n",
			Entry->Move->x, Entry->Move->y, MoveNr));
                break;
              case mtWHITEPASS:
 		MoveNr = (MoveNr+2) & ~1;
		Printf((fp, "(%d) WhitePass\n", MoveNr));
                break;
              case mtBLACKPASS:
 		MoveNr = ((MoveNr+1) & ~1)+1;
		Printf((fp, "(%d) BlackPass\n", MoveNr));
                break;
              case mtWHITEUNDO:
		MoveNr--;
		Printf((fp, "(%d) WhiteUndo\n", MoveNr));
                break;
              case mtBLACKUNDO:
		MoveNr--;
		Printf((fp, "(%d) BlackUndo\n", MoveNr));
                break;
              case mtSETUP:
                Puts("Setup\n", fp);
		MoveNr = 0;
                break;
              default:
                Raise1(AssertException, "Impossible movetype during sgf save");
            }

            /* The order of these loops is important for suicide */
            Stone = Entry->Move;
            if (Stone) board[Stone->y][Stone->x] = Stone->Color;
            for (Stone = Entry->Delete; Stone; Stone = Stone->Next) {
                Printf((fp, "%2d %2d ES\n", Stone->x, Stone->y));
                board[Stone->y][Stone->x] = Empty;
            }
            for (Stone = Entry->Add; Stone; Stone = Stone->Next) {
                Printf((fp, "%2d %2d %s\n", Stone->x, Stone->y,
                        Stone->Color == White ? "WS" : "BS"));
                board[Stone->y][Stone->x] = Stone->Color;
            }

            WritePsComments(fp, &Current->Comments, MoveNr);

            Entry = Current->Entries.Next;
            if (Entry->Next != &Current->Entries) {
		Printf((fp, "%2d SavePosition\n", SaveNr++));
		WritePsEntry(fp, board, Entry, SaveNr, MoveNr);
		Entry = Entry->Next;
		do {
		    Next = Entry->Next;
		    Printf((fp, "%2d LoadPosition\n", SaveNr-1));
		    if (Next == &Current->Entries)
			Printf((fp, "%2d DropPosition\n", --SaveNr));
		    WritePsEntry(fp, board, Entry, SaveNr, MoveNr);
		    Entry = Next;
		} while (Entry != &Current->Entries);
                break;
            }
        } while (Entry != &Current->Entries);
    } ON_UNWIND {
        Current = Cur;
        do {
            /* The order of these loops is important for suicide */
            for (Stone = Current->Entries.Delete; Stone; Stone = Stone->Next)
                board[Stone->y][Stone->x] = Empty;
            for (Stone = Current->Entries.Add;    Stone; Stone = Stone->Next)
                board[Stone->y][Stone->x] = Stone->Color;
            Current = Current->Entries.Entry;
            Stone = Current->Entries.Move;
            if (Stone) board[Stone->y][Stone->x] = Empty;
        } while (Current != Base);
    } END_UNWIND;
}

void WritePostscript(FILE *fp, Gamelog *game)
{
    int        x, y, SizeX, SizeY, SaveNr;
    char     **Board, *B;
    Node      *Current;
    NodeEntry *Entry, *Next;
/*
    FILE      *fpi;
    char       Buffer[1000];

    fpi = fopen("diagram.ps", "r");
    if (!fpi) Raise1(ErrnoFatalException, "could not open diagram.ps");
    WITH_UNWIND {
        while (fgets(Buffer, sizeof(Buffer), fpi)) Puts(Buffer, fp);
        if (!feof(fpi))
            Raise1(ErrnoFatalException, "failed to read diagram.ps");
    } ON_UNWIND {
        fclose(fpi);
    } END_UNWIND;
*/

    SizeX = game->SizeX;
    SizeY = game->SizeY;
    Board = game->Board;

    Puts("%% ---------- begin of game record ---------\n", fp);

    Printf((fp, "%d %d StartGame\n", SizeX, SizeY));
    WritePsComments(fp, &game->Initial->Comments, 0);
    Puts("Header\n", fp);

    SizeX--;
    SizeY--;
    for (y=SizeY; y>=0; y--, Board++)
        for (x=SizeX, B= *Board+SizeX; x>=0; x--, B--)
            switch(*B) {
              case White:
                Printf((fp, "%d %d WS\n", x, y));
                break;
              case Black:
                Printf((fp, "%d %d BS\n", x, y));
                break;
              case Empty:
                break;
              default:
                Raise1(AssertException, "impossible nodevalue");
            }

    Current = game->Current;
    Entry   = Current->Entries.Next;
    Board   = game->Board;
    SaveNr  = 0;
    if (game->Current != game->Initial)
        WritePsComments(fp, &Current->Comments, 0);
    if (Entry->Next != &Current->Entries) {
        Printf((fp, "%2d SavePosition\n", SaveNr++));
        WritePsEntry(fp, Board, Entry, SaveNr, 0);
        Entry = Entry->Next;
        do {
            Next = Entry->Next;
            Printf((fp, "%2d LoadPosition\n", SaveNr-1));
            if (Next == &Current->Entries)
		Printf((fp, "%2d DropPosition\n", --SaveNr));
	    WritePsEntry(fp, Board, Entry, SaveNr, 0);
	    Entry = Next;
        } while (Entry != &Current->Entries);
    } else if (Entry != &Current->Entries)
        WritePsEntry(fp, Board, Entry, SaveNr, 0);

    Puts("EndGame\n", fp);
    Puts("%% ----------- end of game record ----------\n", fp);
}

XtPointer WritePsFun(FILE *fp, XtPointer Closure)
{
    WritePostscript(fp, (Gamelog *) Closure);
    return Closure;
}
#undef EXCEPTION
