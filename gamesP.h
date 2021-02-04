#ifndef GAMESP_H
# define GAMESP_H

# include <time.h>
# include <stddef.h>
# include <regex.h>

# include "players.h"

#define AppContext(w)   XtWidgetToApplicationContext(w)

/* The exact value of these is important ! */
# define NOTONSERVER -1
# define REVIEWGAME  -2
# define SGFGAME     -3

# define BUSY      2 /* The first three exclude each other */
# define ADJOURNED 3 /* The bit assignments are important  */
# define OVER      1
# define SCORING   4 /* This one is a tagged on bit */
# define UNSCORING (~SCORING) /* To extract non scoring part */

#define NOBYO     -1

#define CHANGEINT(ptr, field, value, extra)     \
do {                                            \
    if ((ptr)->field != (value)) {              \
        (ptr)->field = (value);                 \
        (ptr)->Found = CHANGED;                 \
        extra;                                  \
    }                                           \
} while(0)

#define CHANGESTRING(ptr, field, value, extra)  \
do {                                            \
    if (((ptr)->field && (!value || strcmp((ptr)->field, (value)))) || \
        (!(ptr)->field && value)) {             \
        char *sillystr;                         \
                                                \
        sillystr = mystrdup(value);             \
        myfree((ptr)->field);                   \
        (ptr)->field = sillystr;                \
        (ptr)->Found = CHANGED;                 \
        extra;                                  \
    }                                           \
} while(0)

#define PROPHANDICAP(value)                                     \
    if (Log) {                                                  \
        DeleteGlobalProperty(Log, "Handicap");                  \
        if (value) {                                            \
            sprintf(Buffer, "%d", (value));                     \
            AddGlobalProperty(Log, "Handicap", Buffer, -1);     \
        }                                                       \
    }

#define PROPSTRING(value, name)                                 \
    if (Log) {                                                  \
        DeleteGlobalProperty(Log, (name));                      \
        AddGlobalProperty(Log, (name), (value), -1);            \
    }

struct _Game {
    struct _Game    *Next, *Previous;
    Gamelog         *Log;
    int              ServerId; /* -1: not on server, -2: review */
    int              WantObserved, Observed, NrObservers;
    int              WhiteByo,       BlackByo;
    int              WhiteCaptures,  BlackCaptures;
    int              oldWhiteCaptures,  oldBlackCaptures; /* before scoring */
    Player          *Black, *White, *Strongest, *Weakest;
    Player          *Black2, *White2, *Teacher;
    long             WhiteTime,      BlackTime;
    int              ByoPeriod;
    struct _Observe *Observers;
    int              Move, Handicap;
    size_t           XSize, YSize;
    char            *Komi, *Title;
    size_t           TitleLength;
    int              Pos;
    int              Finished, Mode, Rules, WidgetPlan;
    State            Found;
    struct tm        UniversalTime;  /* When did we see the game first */
#ifndef NO_CLIENT_TIME
    time_t           MoveTime;       /* time when it became my turn to move */
#endif
    Widget           Widget;         /* Widget in the gamelist    */
    BWPiece          ToMove;         /* Who   is to move          */
    BWPiece	     Color; 	     /* WHITE if you play white,  */
                		     /* BLACK if you play black,  */
                		     /* EMPTY if you don't play   */
};

typedef struct {
    regex_t     Pattern;
    const char *Name;
    int         NameLength;
} ToWidget;

typedef struct {
    ToWidget *Convert;
    int       NrConvert;
} ConvertToWidget;

typedef struct _WidgetPlan *WidgetPlan;

#define HASH_SIZE 32768
typedef struct {
    char *content;
    char *description;
} HashEntry;

extern HashEntry playerHash[HASH_SIZE];

#define CREATE    1
#define DELETE    2

extern int  PlanInsert(Widget w);
extern WidgetPlan
AllocWidgetPlan(void *Base, size_t Next, size_t Pos, size_t widgetPlan,
                size_t widget, int *Rate, Widget Collect, XtPointer Closure,
                int  (*Compare)(const void *entry1, const void *entry2),
                const char * (*Properties)(Cardinal *i, Arg *arg, void *entry,
                                           XtPointer Closure),
                void (*WidgetCreate) (void *Entry, XtPointer Closure),
                void (*WidgetDestroy)(void *Entry, XtPointer Closure),
                void (*done)(int NrEntries, XtPointer Closure));
extern void FreeWidgetPlan(WidgetPlan Plan);
extern void WidgetPlanRefresh(WidgetPlan Plan);
extern void WidgetPlanRefreshNow(WidgetPlan Plan);
extern void WidgetPlanResort(WidgetPlan Plan);
extern void WidgetPlanReposition(void *entry, WidgetPlan Plan);

extern Game ReviewBase;
extern CommentFun NextMoveFun, PrevMoveFun;
extern void FreeGame(Game *game);
extern void DumpGame(const Game *game);
extern void ShowObserve(Game *game);
extern void TestDeleteReview(struct _Game *review);

extern void FreeToWidget(ConvertToWidget *Convert);
extern const char *CompileToWidget(struct _StringPairList *Base,
                                   ConvertToWidget *Convert);
extern const char *CompileToHash(struct _StringPairList *Base,
				 HashEntry *hashTable,
			  const char *Prefix, ConvertToWidget *Convert);
extern const char *RegexGetPlayerType(const char *Description);
extern const char *RegexGetGameType(const char *Description);
extern const char *RegexGetType(const char *Prefix, ConvertToWidget *Convert,
                                const char *Description);
#endif /* GAMESP_H */
