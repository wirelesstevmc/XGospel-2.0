#include <stddef.h>
#include <X11/Intrinsic.h>

#include "gospel.h"

#define UNKNOWN_ADVANTAGE (-1000.)

typedef struct _Analyze Analyze;
extern Analyze *OpenAnalyze(Widget w, const char *Name,
                            int Level, size_t Size, int Move,
                            int AllowSuicide, float WhiteAdvantage);
extern Analyze *AnalyzeBoard(char **Board, const char *Name,
                             int Level, size_t Size, int Move,
                             int AllowSuicide, float WhiteAdvantage);
extern void ToggleBlink(Widget w, XtPointer clientdata, XtPointer calldata);
extern void AnalyzeTime(unsigned long diff);
extern void AnalyzeGoto(Widget w, XEvent *event, String *string, Cardinal *n);
extern void DisplayScore(Boolean scoring, Gamelog *Log, float WhiteAdvantage,
			 Widget BoardWidget, Widget TopWidget);
