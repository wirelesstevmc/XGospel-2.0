#include "myxlib.h"
#include "mymalloc.h"
#include "except.h"

#include <X11/StringDefs.h>
#include <X11/Xaw/Scrollbar.h>

/* need the private definitions to work around Xaw scroll bug */
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/Xaw/AsciiTextP.h>

extern unsigned long FMT8BIT;   /* Seems to be missing on CONVEX */

#define NOFOLLOW      1
#define TEXTINBATCH 128

typedef struct _Batch {
    struct _Batch *Next;
    char  *Text;
    size_t Length;
    int    Mode;
    char   MoreText[1];
} Batch;

typedef struct _BatchWidget {
    struct _BatchWidget *Previous, *Next;
    Widget               w;
    XtIntervalId         Id;
    Boolean              Destroy;
    Batch               *Text;
    int                  New;
} BatchWidget;

static BatchWidget BatchWidgets;
static MyContext   Context;
static unsigned long BatchTime = 200;

/*****************************************************************************/
/* Save text in a widget to file                                             */
/*****************************************************************************/
Exception WriteTextException = { "Could not write text widget",
                                  0, ErrnoExceptionAction };

long MySaveText(FILE *fp, Widget w)
{
    XawTextPosition From, End;
    XawTextBlock    block;
    long            Written;
    int             Wrote;
    Widget          Source;

    Written = 0;
    Source = XawTextGetSource(w);
    From   = 0;
    End    = XawTextSourceScan(Source, 0, XawstAll, XawsdRight, 1, True);
    while (From < End) {
        From = XawTextSourceRead(Source, From, &block, End-From);
        Wrote = fwrite(block.ptr, sizeof(char), (size_t) block.length, fp);
        if (Wrote != block.length)
            Raise2(WriteTextException, XtName(w), "to file");
        Written += Wrote;
    }
    return Written;
}

/*****************************************************************************/
/* Add text at the end of text widget                                        */
/*****************************************************************************/

/* Is this sick way the only way to do it ??? -Ton */
#ifndef   HAVE_NO_STDARG_H
void AddText(Widget w, const char *Format, ...)
#else  /* HAVE_NO_STDARG_H */
void AddText(w, va_alist)
Widget w;
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list         args;
    char            Text[2048];
    Widget          Source;
    XawTextPosition Pos1, Pos2;
    XawTextBlock    block;
    XawTextEditType type;

#ifndef   HAVE_NO_STDARG_H
    if (Format == NULL) return;
    va_start(args, Format);
#else  /* HAVE_NO_STDARG_H */
    Widget w;
    const char *Format;

    va_start(args);
    Format = va_arg(args, const char *);
    if (Format == NULL) {
        va_end(args);
        return;
    }
#endif /* HAVE_NO_STDARG_H */
    vsprintf(Text, Format, args);
    va_end(args);
   
    XtVaGetValues(w,
                  XtNeditType,       (XtArgVal) &type,
                  XtNtextSource,     (XtArgVal) &Source,
                  XtNinsertPosition, (XtArgVal) &Pos2,
                  NULL);
    XtVaSetValues(w, XtNeditType, (XtArgVal) XawtextEdit, NULL);

    Pos1 = XawTextSourceScan(Source, 0, XawstAll, XawsdRight, 1, True);

    block.firstPos = 0;
    block.length   = strlen(Text);
    block.ptr      = Text;
    block.format   = FMT8BIT;
    if (XawEditDone == XawTextReplace(w, Pos1, Pos1, &block) && 
        Pos2 >= Pos1) Pos2 = Pos1+block.length;

    XtVaSetValues(w,
                  XtNeditType,       (XtArgVal) type,
                  XtNinsertPosition, (XtArgVal) Pos2,
                  NULL);
}

int AppendText(Widget w, const char *Text, size_t Length)
{
    Widget          Source;
    XawTextPosition Pos, Pos2;
    XawTextBlock    block;
    XawTextEditType type;

    XtVaGetValues(w,
                  XtNeditType,   (XtArgVal) &type,
                  XtNtextSource, (XtArgVal) &Source,
                  XtNinsertPosition, (XtArgVal) &Pos2,
                  NULL);
    XtVaSetValues(w, XtNeditType, (XtArgVal) XawtextEdit, NULL);

    Pos = XawTextSourceScan(Source, 0, XawstAll, XawsdRight, 1, True);
    block.firstPos = 0;
    block.length   = Length;
    block.ptr      = (char *) Text;
    block.format   = FMT8BIT;
    XawTextReplace(w, Pos, Pos, &block);

    XtVaSetValues(w, XtNeditType, (XtArgVal) type, NULL);
    return Pos2 == Pos;
}

/* Massage w until it expanded to it's natural size. Incredibly sick flaw in
   the Xaw text widget that this is needed. (Also notice the sick way it works.
*/
void RelaxText(Widget w)
{
    Dimension Height, Width, OldHeight, OldWidth;
    XawTextPosition Pos1, Pos2;
    XawTextBlock    block;
    XawTextEditType type;
    Widget          Source;

    XtVaGetValues(w,
                  XtNeditType,       (XtArgVal) &type,
                  XtNtextSource,     (XtArgVal) &Source,
                  XtNinsertPosition, (XtArgVal) &Pos2,
                  NULL);
    XtVaSetValues(w, XtNeditType, (XtArgVal) XawtextEdit, NULL);
    Pos1 = XawTextSourceScan(Source, 0, XawstAll, XawsdRight, 1, True);
    block.firstPos = 0;
    block.length   = 0;
    block.ptr      = (char *) "";
    block.format   = FMT8BIT;

    /* For some reason the first XawTextReplace does nothing.
       Force execution at least twice */
    Height = Width = 0;
    do {
        XawTextReplace(w, Pos1, Pos1, &block);
        OldHeight = Height;
        OldWidth  = Width;
        XtVaGetValues(w,
                      XtNheight, (XtArgVal) &Height,
                      XtNwidth,  (XtArgVal) &Width, NULL);
    } while (Height != OldHeight || Width != OldWidth);

    XtVaSetValues(w,
                  XtNeditType,       (XtArgVal) type,
                  XtNinsertPosition, (XtArgVal) Pos2,
                  NULL);
}

static void FreeBatchWidget(BatchWidget *w);
static void CallDestroyBatchWidget(Widget w,
                                   XtPointer clientdata, XtPointer calldata)
{
    BatchWidget *bw;

    bw = (BatchWidget *) clientdata;
    bw->Destroy = False;
    FreeBatchWidget(bw);
}

static void FreeBatchWidget(BatchWidget *bw)
{
    Batch *Here, *Next;

    bw->Previous->Next = bw->Next;
    bw->Next->Previous = bw->Previous;
    for (Here = bw->Text; Here; Here = Next) {
        Next = Here->Next;
        if ((Here->Mode & TEXTINBATCH) == 0) myfree(Here->Text);
        myfree(Here);
    }
    MyDeleteContext(Context, bw->w, 0);
    if (bw->Id) XtRemoveTimeOut(bw->Id);
    if (bw->Destroy != False)
        XtRemoveCallback(bw->w, XtNdestroyCallback,
                         CallDestroyBatchWidget, (XtPointer) bw);
    myfree(bw);
}

void ForceText(Widget w)
{
    int          Follow;
    size_t       Len;
    Batch       *Here;
    BatchWidget *bw;
    char        *Out, *End;
    XtPointer    Last;
    int          n;

    if (MyFindContext(Context, w, 0, &Last)) return;

    bw = (BatchWidget *) Last;
    if (bw->Id) {
        XtRemoveTimeOut(bw->Id);
        bw->Id = 0;
    }

    Len=0;
    for (Here = bw->Text; Here; Here = Here->Next) Len += Here->Length;
    Out = mynews(char, Len+1);
    End = Out+Len;
    *End = 0;
    Follow = 1;
    for (Here = bw->Text; Here; Here = Here->Next) {
        if (Here->Mode & NOFOLLOW) Follow = 0;
        End -= Here->Length;
        memcpy(End, Here->Text, Here->Length);
    }
    FreeBatchWidget(bw);
    if (AppendText(w, Out, Len) && Follow)
        XawTextSetInsertionPoint(w, XawTextSourceScan(XawTextGetSource(w),
                                 0, XawstAll, XawsdRight, 1, True));
    myfree(Out);
}

static void BatchTest(XtPointer closure, XtIntervalId *id)
{
    Widget       w, vbar;
    int          Follow;
    size_t       Len;
    Batch       *Here;
    BatchWidget *bw;
    char        *Out, *End;
    float       top, shown;
    XawTextPosition pos, oldPos;

    bw = (BatchWidget *) closure;
    w  = bw->w;
    if (bw->New) {
        bw->Id = XtAppAddTimeOut(XtWidgetToApplicationContext(w), BatchTime,
                                 BatchTest, closure);
        bw->New = 0;
    } else {
        bw->Id = 0;
        Len=0;
        for (Here = bw->Text; Here; Here = Here->Next) Len += Here->Length;
        Out = mynews(char, Len+1);
        End = Out+Len;
        *End = 0;
        Follow = 1;
        for (Here = bw->Text; Here; Here = Here->Next) {
            if (Here->Mode & NOFOLLOW) Follow = 0;
            End -= Here->Length;
            memcpy(End, Here->Text, Here->Length);
        }
        FreeBatchWidget(bw);
        if (AppendText(w, Out, Len) && Follow) {
	    pos = XawTextSourceScan(XawTextGetSource(w),
				    0, XawstAll, XawsdRight, 1, True);
            XawTextSetInsertionPoint(w, pos);

	    /* Work around a nasty bug in Xaw, which doesn't show the
	       insertion point when a long line was wrapped. So the end
	       of the line may be hidden.
	     */
	    vbar = ((TextRec*)w)->text.vbar;
	    if (vbar && XtIsRealized(vbar)) {
		for (;;) {
		    XtVaGetValues(vbar,
				  XtNtopOfThumb,     (XtArgVal) &top,
				  XtNshown,          (XtArgVal) &shown,
				  NULL);
		    /* If the scroll bar is at the bottom, all ok: */
		    if (top + shown >= 0.999999) break;

		    XtVaGetValues(w,
				  XtNdisplayPosition, (XtArgVal) &oldPos,
				  NULL);
		    /* Find the next line after the first line currently
		       displayed at the top of the text widget: 
		     */
		    pos = XawTextSourceScan(XawTextGetSource(w), oldPos,
					    XawstEOL, XawsdRight, 1, True);
		    if (pos <= oldPos) break;
		    /* Force scrolling to this next line: */
		    XtVaSetValues(w,
				  XtNdisplayPosition, (XtArgVal) pos,
				  NULL);
		}
	    }
	}
        myfree(Out);
    }
}

void BatchAppendText(Widget w, const char *Text, size_t Length, int Mode)
{
    BatchWidget *bw;
    Batch       *batch;
    XtPointer    Last;

    if (MyFindContext(Context, w, 0, &Last)) {
        bw = mynew(BatchWidget);
        bw->w = w;
        bw->New = 0;
        bw->Text = NULL;
        bw->Id = XtAppAddTimeOut(XtWidgetToApplicationContext(w),
                                 BatchTime, BatchTest, (XtPointer) bw);
        XtAddCallback(w, XtNdestroyCallback,
                      CallDestroyBatchWidget, (XtPointer) bw);
        bw->Destroy = True;
        WITH_HANDLING {
            MySaveContext(Context, w, 0, (XtPointer) bw);
            bw->Next     =  BatchWidgets.Next;
            bw->Previous = &BatchWidgets;
            bw->Next->Previous = bw->Previous->Next = bw;
        } ON_EXCEPTION {
            XtRemoveTimeOut(bw->Id);
            myfree(bw);
        } END_HANDLING;
    } else {
        bw = (BatchWidget *) Last;
        bw->New = 1;
    }

    batch = (Batch *) mymalloc(sizeof(Batch)+Length);
    batch->Text   = batch->MoreText;
    batch->Mode   = TEXTINBATCH | Mode;
    memcpy(batch->MoreText, Text, Length);
    batch->MoreText[Length] = 0;
    batch->Length = Length;
    batch->Next = bw->Text;
    bw->Text = batch;
}

#ifndef   HAVE_NO_STDARG_H
size_t BatchAddText(Widget w, const char *Format, ...)
#else  /* HAVE_NO_STDARG_H */
size_t BatchAddText(w, va_alist)
Widget w;
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list         args;
    char            Text[2048];
    size_t          length;

#ifndef   HAVE_NO_STDARG_H
    if (Format == NULL) return 0;
    va_start(args, Format);
#else  /* HAVE_NO_STDARG_H */
    Widget w;
    const char *Format;

    va_start(args);
    Format = va_arg(args, const char *);
    if (Format == NULL) {
        va_end(args);
        return;
    }
#endif /* HAVE_NO_STDARG_H */
    vsprintf(Text, Format, args);
    va_end(args);

    length = strlen(Text);
    if (w) BatchAppendText(w, Text, length, 0);
    return length;
}

void InitTextBatch(void)
{
    Context = MyAllocContext();
    BatchWidgets.Next = BatchWidgets.Previous = &BatchWidgets;
}

/* Should force collected text to widget -Ton */
void CleanTextBatch(void)
{
    while (BatchWidgets.Next != &BatchWidgets)
        FreeBatchWidget(BatchWidgets.Next);
    MyFreeContext(Context);
}

char *TextWidgetCursorLine(Widget w)
{
    XawTextPosition Pos, Pos1, Pos2;
    XawTextBlock    block;
    Widget          Src;
    char           *From, *Ptr;
    int             Length, ToRead;

    XtVaGetValues(w, 
                  XtNtextSource,     (XtArgVal) &Src,
                  XtNinsertPosition, (XtArgVal) &Pos,
                  NULL);
    Pos1 = XawTextSourceScan(Src, Pos, XawstEOL, XawsdLeft, 1, False);
    Pos2 = XawTextSourceScan(Src, Pos, XawstEOL, XawsdRight, 1, False);
    Length = ToRead = Pos2-Pos1;
    From = Ptr = mynews(char, Length+1);
    for (ToRead = Length; ToRead; ToRead -= block.length) {
        Pos1 = XawTextSourceRead(Src, Pos1, &block, ToRead);
        memcpy(Ptr, block.ptr, (size_t) block.length);
        Ptr    += block.length;
    }
    From[Length] = 0;
    return From;
}
