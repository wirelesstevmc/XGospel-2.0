#include "myxlib.h"
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xmu/Drawing.h>
#include <X11/Xmu/Converters.h>

#include "CheckMark"
#include "XgospelIcon"

#ifdef   HAVE_XPM
# include <X11/xpm.h>
# include "XgospelIcon.xpm"
#endif /* HAVE_XPM */
#include "mymalloc.h"
#include <string.h>
#if !STDC_HEADERS && HAVE_MEMORY_H
# include <memory.h>
#endif /* not STDC_HEADERS and HAVE_MEMORY_H */
#include <ctype.h>
#include <stdlib.h>

#ifdef    HAVE_NO_MEMCHR_PROTO
extern void *memchr(const void *s, int c, size_t n);
#endif /* HAVE_NO_MEMCHR_PROTO */

#define offset(field) (XtPointer) XtOffset(WidgetRec *, core.field)
static XtConvertArgRec ScreenConvertArg[] = {
    { XtWidgetBaseOffset, offset(screen),   sizeof(Screen *) },
    { XtWidgetBaseOffset, offset(colormap), sizeof(Colormap) },
    { XtWidgetBaseOffset, offset(depth),    sizeof(Cardinal) }
};

static XtConvertArgRec ColorConvertArg[] = {
    { XtWidgetBaseOffset, offset(screen),   sizeof(Screen *) },
    { XtWidgetBaseOffset, offset(colormap), sizeof(Colormap) }
};

# ifndef NOSTRINTOWIDGET
static XtConvertArgRec ParentConvertArg[] = {
    { XtWidgetBaseOffset, offset(parent),   sizeof(Widget) },
};
# endif /* NOSTRINGTOWIDGET */
#undef offset

#define	olddone(address, type)          \
if (1) {                                \
    toVal->size = sizeof(type);         \
    toVal->addr = (XPointer) address;   \
    return;                             \
} else return

/* FIXME: We have a memory leak if our value was allocated and we return
   False !! */
#define	done(type, value)                               \
if (1) {                                                \
    if (toVal->addr != NULL) {				\
        if (toVal->size < sizeof(type)) {		\
	    toVal->size = sizeof(type);			\
	    return False;				\
	}						\
        *(type*)(toVal->addr) = (value);		\
    } else {						\
	static type static_val;				\
	static_val = (value);				\
	toVal->addr = (XtPointer)&static_val;		\
    }							\
    toVal->size = sizeof(type);				\
    return True;					\
} else return True

/*****************************************************************************/
/* A String to Pixmap converter                                              */
/* todo: decent file path                                                    */
/*       decent color caching on xpm                                         */
/*****************************************************************************/

static Boolean MyCvtStringToPixel(Display* disp,
                                  XrmValuePtr args, Cardinal *num_args,
                                  XrmValuePtr fromVal, XrmValuePtr toVal,
                                  XtPointer *closure_ret)
{
    Display   *dpy;
    String     str;
    XColor     screenColor;
    XColor     exactColor;
    Screen    *screen;
    Colormap   colormap;
    Status     status;
    String     params[1];

    str = (String)fromVal->addr;
    if (*num_args != XtNumber(ColorConvertArg)) {
        XtAppErrorMsg(XtDisplayToApplicationContext(disp),
                      "wrongParameters", "myCvtStringToPixel", "myProgsError",
                      "String to pixel conversion needs 3 arguments",
                      (String *)NULL, (Cardinal *)NULL);
        return False;
    }

    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);

    dpy = DisplayOfScreen(screen);

    if (XmuCompareISOLatin1(str, XtDefaultBackground) == 0) {
	*closure_ret = False;
        if (ReverseP(dpy)) done(Pixel, BlackPixelOfScreen(screen));
        else               done(Pixel, WhitePixelOfScreen(screen));
    } 
    if (XmuCompareISOLatin1(str, XtDefaultForeground) == 0) {
	*closure_ret = False;
        if (ReverseP(dpy)) done(Pixel, WhitePixelOfScreen(screen));
        else               done(Pixel, BlackPixelOfScreen(screen));
    }

    status = XAllocNamedColor(DisplayOfScreen(screen), colormap,
			      (char*)str, &screenColor, &exactColor);
    if (status == 0) {
	String msg, type;
	params[0] = str;
	/* Server returns a specific error code but Xlib discards it.  Ugh */
	if (XLookupColor(DisplayOfScreen(screen), colormap, (char*)str,
			 &exactColor, &screenColor)) {
	    type = (char *) "noColormap";
	    msg  = (char *) "Cannot allocate colormap entry for \"%s\"";
	}
	else {
	    type = (char *) "badValue";
	    msg  = (char *) "Color name \"%s\" is not defined";
	}

        XtDisplayStringConversionWarning(disp, str, "Pixel");
	*closure_ret = False;
	return False;
    } else {
	*closure_ret = (char*) True;
        done(Pixel, screenColor.pixel);
    }
}

static void MyFreePixel(XtAppContext app, XrmValuePtr toVal, XtPointer closure,
                        XrmValuePtr args, Cardinal *num_args)
{
    Screen  *screen;
    Colormap colormap;

    if (*num_args != XtNumber(ColorConvertArg)) {
        XtAppWarningMsg(app, "wrongParameters", "myFreePixel",
                        "XtToolkitError", "Freeing a pixel requires screen"
                        "and colormap arguments", (String *)NULL,
                        (Cardinal *) NULL);
        return;
    }

    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);

    if (closure) XFreeColors(DisplayOfScreen(screen), colormap,
                             (unsigned long*)toVal->addr, 1, (unsigned long)0);
}

static const char *BitmapFromName(Pixmap *pixmap,
                                  unsigned int *width, unsigned int *height,
                                  unsigned int *xhot, unsigned int *yhot,
                                  Screen *screen, Pixel fg, Pixel bg,
                                  unsigned int depth, const char *Name)
{
    Pixmap         temp;
    Display       *dpy;
    XrmDatabase    db;
    unsigned char *data;
    size_t         Len;
    int            rc;
    unsigned int   w, h, xh, yh;
    String         fn;
    char          *ptr;
    const char    *OpenBrace, *CloseBrace;

    if (strcmp(Name, "None") == 0) {
	*pixmap = None;
	return NULL;
    }

    if (strcmp(Name, "ParentRelative") == 0) {
	*pixmap = ParentRelative;
	return NULL;
    }

    if (Name[0] == '#') {
        temp = (Pixmap) strtol(++Name, &ptr, 0);
	if (ptr != Name && *ptr == 0) {
            if (pixmap) *pixmap = temp;
        } else return "Bitmap: cannot convert constant to pixmap";
	return NULL;
    }

    dpy = DisplayOfScreen(screen);

    OpenBrace = strchr(Name, '(');
    if (OpenBrace) {
        if (memcmp(Name, "builtin", 7) == 0) {
            OpenBrace++;
            CloseBrace = strchr(OpenBrace, ')');
            if (CloseBrace) {
                Len = CloseBrace-OpenBrace;
                data = NULL;
                w = h = 0;
                xh = yh = 0;
                if (Len == 9 && memcmp(OpenBrace, "CheckMark", Len) == 0) {
                    data = (unsigned char *) CheckMark_bits;
                    w  = CheckMark_width;
                    h  = CheckMark_height;
                    xh = CheckMark_x_hot;
                    yh = CheckMark_y_hot;
                } else
                if (Len == 11 && memcmp(OpenBrace, "XgospelIcon", Len) == 0) {
                    data = (unsigned char *) XgospelIcon_bits;
                    w  = XgospelIcon_width;
                    h  = XgospelIcon_height;
                    xh = XgospelIcon_x_hot;
                    yh = XgospelIcon_y_hot;
                }
                if (data) {
                    *pixmap =
                        XCreatePixmapFromBitmapData(dpy,
                                                    RootWindowOfScreen(screen),
                                                    (char *) data,
                                                    w, h, fg, bg, depth);
                    if (width)  *width  = w;
                    if (height) *height = h;
                    if (xhot)   *xhot   = xh;
                    if (yhot)   *yhot   = yh;
                    return NULL;
                } else return "Bitmap: could not find builtin bitmap";
            } else return "Bitmap: could not find closing ')'";
        } else return "Bitmap: unknown function"; 
    }

    temp = XmuLocatePixmapFile(screen, (char *) Name,
                               (unsigned long) fg, (unsigned long) bg, depth,
                               NULL, 0, (int *) width, (int *) height,
                               (int *) xhot, (int *) yhot);
    if (temp != None) {
        *pixmap = temp;
        return NULL;
    }

#ifndef   R4
    db = XrmGetDatabase(dpy);
    XrmSetDatabase(dpy, XtScreenDatabase(screen));
#endif /* R4 */
    fn = XtResolvePathname(dpy, "bitmaps", (char *)Name, "", NULL, NULL, 0, 0);
    if (!fn)
        fn = XtResolvePathname(dpy, "",(char *)Name, ".xbm", NULL, NULL, 0, 0);
#ifndef   R4
    XrmSetDatabase(dpy, db);
#endif /* R4 */
    if (!fn) return "Bitmap: could not resolve name";
    rc = XmuReadBitmapDataFromFile(fn, width, height,
                                   &data, (int *) xhot, (int *) yhot);
    XtFree(fn);
    switch(rc) {
      case BitmapSuccess:
        *pixmap = XCreatePixmapFromBitmapData(dpy, 
                                              RootWindowOfScreen(screen),
                                              (char *) data, *width, *height,
                                              fg, bg, depth);
        XFree((char *)data);
        return NULL;
      case BitmapFileInvalid:
        return "Bitmap: BitmapFileInvalid";
      case BitmapNoMemory:
        return "Bitmap: BitmapNoMemory";
      default:
        return "Bitmap: unknown error";
    }
}

static const char *PixmapFromName(Pixmap *pixmap,
                                  Screen *screen, const char *Name)
{
#ifdef HAVE_XPM
    Pixmap          temp;
    Display        *dpy;
    XrmDatabase     db;
    unsigned char **data;
    size_t          Len;
    int             rc;
    String          fn;
    char           *ptr;
    const char     *OpenBrace, *CloseBrace;

    if (strcmp(Name, "None") == 0) {
	if (pixmap) *pixmap = None;
	return NULL;
    }

    if (strcmp(Name, "ParentRelative") == 0) {
	if (pixmap) *pixmap = ParentRelative;
	return NULL;
    }

    if (strcmp(Name, "XtUnspecifiedPixmap") == 0) {
	if (pixmap) *pixmap = XtUnspecifiedPixmap;
	return NULL;
    }

    if (Name[0] == '#') {
        temp = (Pixmap) strtol(++Name, &ptr, 0);
	if (ptr != Name && *ptr == 0) {
            if (pixmap) *pixmap = temp;
        } else return "Pixmap: cannot convert constant to pixmap";
	return NULL;
    }

    dpy = DisplayOfScreen(screen);

    OpenBrace = strchr(Name, '(');
    if (OpenBrace) {
        if (memcmp(Name, "builtin", 7) == 0) {
            OpenBrace++;
            CloseBrace = strchr(OpenBrace, ')');
            if (CloseBrace) {
                Len = CloseBrace-OpenBrace;
                data = NULL;
                if (Len == 11 && memcmp(OpenBrace, "XgospelIcon", Len) == 0)
                    data = (unsigned char **) XgospelIcon_xpm;
                if (data) {
                    rc = XpmCreatePixmapFromData(dpy,
                                                 RootWindowOfScreen(screen),
                                                 (char **) data, &temp,
                                                 NULL, NULL);
                    goto converted;
                } else return "Pixmap: could not find builtin bitmap";
            } else return "Pixmap: could not find closing ')'";
        } else return "Pixmap: unknown function"; 
    }

#ifndef   R4
    db = XrmGetDatabase(dpy);
    XrmSetDatabase(dpy, XtScreenDatabase(screen));
#endif /* R4 */

    fn = XtFindFile(Name, NULL, 0, 0);
    if (!fn) fn=XtResolvePathname(dpy, "pixmaps", Name, "", NULL, NULL, 0, 0);
    if (!fn) fn=XtResolvePathname(dpy, "", Name, ".xpm", NULL, NULL, 0, 0);
#ifndef   R4
    XrmSetDatabase(dpy, db);
#endif /* R4 */
    if (!fn) return "Pixmap: could not resolve name";
    rc = XpmReadFileToPixmap(dpy, RootWindowOfScreen(screen), fn, &temp,
                             NULL, NULL);
    XtFree(fn);
  converted:
    switch(rc) {
      case XpmSuccess:
        if (pixmap) *pixmap = temp;
        return NULL;
      case XpmOpenFailed:
        return "Pixmap: XpmOpenFailed";
      case XpmFileInvalid:
        return "Pixmap: XpmFileInvalid";
      case XpmNoMemory:
        return "Pixmap: XpmNoMemory";
      default:
        return "Pixmap: unknown error";
    }
#else  /* HAVE_XPM */
    return "Pixmap: was compiled without Xpm support";
#endif /* HAVE_XPM */
}

/* ConvertData is used completely incorrectly --Ton */
/*ARGSUSED*/
static Boolean MyCvtStringToPixmap(Display *disp,
                                   XrmValuePtr args, Cardinal *num_args,
                                   XrmValuePtr fromVal, XrmValuePtr toVal,
                                   XtPointer *ConvertData)
{
    Pixmap         pixmap;
    Pixel          fg, bg;
    char          *name, myname[100], *ptr1, *ptr2, *ptr3, *ptr4;
    const char    *ErrorMessage;
    Screen        *screen;
    Display       *dpy;
    XrmValue       FromString, ToPixel;
    unsigned int   depth;
    XtCacheRef    *refs;
    int            pars, ch;

    name = (char *) fromVal->addr;
    if (*num_args != 3)
        XtAppErrorMsg(XtDisplayToApplicationContext(disp),
                      "wrongParameters","cvtStringToPixmap","myProgsError",
                      "String to pixmap conversion needs 3 arguments",
                      (String *)NULL, (Cardinal *)NULL);

    if (ConvertData) *ConvertData = NULL;

    screen = *(Screen **)       args[0].addr;
    ErrorMessage = PixmapFromName(&pixmap, screen, name);
    if (ErrorMessage == NULL) done(Pixmap, pixmap);

    depth  = *(unsigned int *)  args[2].addr;
    dpy = DisplayOfScreen(screen);
    if (ReverseP(dpy)) {
        bg = BlackPixelOfScreen(screen);
        fg = WhitePixelOfScreen(screen);
    } else {
        bg = WhitePixelOfScreen(screen);
        fg = BlackPixelOfScreen(screen);
    }

    refs = mynews(XtCacheRef, 3);
    refs[0] = refs[1] = refs[2] = NULL;

    ptr1 = strchr(name, '(');
    if (ptr1) {
        strncpy(myname, name, sizeof(myname)-1);
        myname[sizeof(myname)-1] = 0;
        ptr1 = myname+(ptr1-name); 
        *ptr1++ = 0;
        while (isspace(*ptr1)) ptr1++;
        pars = 1;
        for (ptr2 = ptr1; ch = *ptr2, pars; ptr2++)
            if      (ch == '(') pars++;
            else if (ch == ')') if (--pars == 0) break;
            else if (ch == 0  ) break;
        if (ch == 0) ptr2 = NULL;
        else {
            ptr2--;
            while (isspace(*ptr2)) ptr2--;
            ptr2[1] = 0;
        }
  
        ptr2 = strchr(ptr1, ',');
        if (ptr2) {
            ptr3 = ptr2-1;
            while (isspace(*ptr3)) ptr3--;
            ptr3[1] = 0;

            *ptr2++ = 0;
            while (isspace(*ptr2)) ptr2++;
            ptr3 = strchr(ptr2, ',');
            if (ptr3) {
                ptr4 = ptr3-1;
                while (isspace(*ptr4)) ptr4--;
                ptr4[1] = 0;

                *ptr3++ = 0;
                while (isspace(*ptr3)) ptr3++;
                FromString.size = strlen(ptr3)+1;
                FromString.addr = (XPointer) ptr3;
                ToPixel.size = sizeof(Pixel);
                ToPixel.addr = (XPointer) &fg;
                XtCallConverter(dpy, MyCvtStringToPixel, args, 2,
                                &FromString, &ToPixel, refs);
                if (*refs) refs++;
            }
            FromString.size = strlen(ptr2)+1;
            FromString.addr = (XPointer) ptr2;
            ToPixel.size = sizeof(Pixel);
            ToPixel.addr = (XPointer) &bg;
            XtCallConverter(dpy, MyCvtStringToPixel, args, 2,
                            &FromString, &ToPixel, refs);
            if (*refs) refs++;
        }
        name = ptr1;
    }
    if (ConvertData) *ConvertData = (XtPointer) refs;
    else myfree(refs);

    if (XmuCompareISOLatin1(myname, "bitmap") == 0) {
        /* Seems this can break the X connection.... */
        ErrorMessage = BitmapFromName(&pixmap, NULL, NULL, NULL, NULL,
                                      screen, fg, bg, depth, name);
        if (ErrorMessage == NULL) done(Pixmap, pixmap);
    } else if (XmuCompareISOLatin1(myname, "pixmap") == 0) {
        ErrorMessage = PixmapFromName(&pixmap, screen, name);
        if (ErrorMessage == NULL) done(Pixmap, pixmap);
    }

    XtDisplayStringConversionWarning(disp, name, (String) ErrorMessage);
    return False;
}

static void PixmapDestructor(XtAppContext App, XrmValuePtr To,
                             XtPointer ConvertData,
                             XrmValuePtr Args, Cardinal *n)
{
    Display *dpy;

    dpy = DisplayOfScreen(*(Screen **) Args[0].addr);
    XFreePixmap(dpy, * (Pixmap *) To->addr);

    if (ConvertData) {
        XtAppReleaseCacheRefs(App, (XtCacheRef *) ConvertData);
        myfree(ConvertData);
    }
}

/*ARGSUSED*/
static Boolean MyCvtStringToBitmap(Display *disp,
                                   XrmValuePtr args, Cardinal *num_args,
                                   XrmValuePtr fromVal, XrmValuePtr toVal,
                                   XtPointer *ConvertData)
{
    Pixmap         pixmap;
    const char    *ErrorMessage, *name;
    Screen        *screen;

    if (ConvertData) *ConvertData = NULL;
    if (*num_args != 3)
        XtErrorMsg("wrongParameters","cvtStringToBitmap","XtToolkitError",
                   "String to pixmap conversion needs screen argument",
                   (String *) NULL, (Cardinal *) NULL);
    
    name   = (const char *) fromVal->addr;
    screen = *((Screen **) args[0].addr);

    ErrorMessage = BitmapFromName(&pixmap, NULL, NULL, NULL, NULL, screen, 
                                  1, 0, 1, name);
    if (ErrorMessage == NULL) done(Pixmap, pixmap);
    return MyCvtStringToPixmap(disp, args, num_args, fromVal, toVal,
                               ConvertData);
}

/*ARGSUSED*/
static Boolean MyCvtStringToStringList(Display *disp,
                                       XrmValuePtr args, Cardinal *num_args,
                                       XrmValuePtr fromVal, XrmValuePtr toVal,
                                       XtPointer *ConvertData)
{
    String str;
    char  *ptr, **Str, ch, *From;
    int    Nr;
    size_t     *Len;
    StringList *result;

    if (*num_args != 0)
        XtAppErrorMsg(XtDisplayToApplicationContext(disp),
                      "wrongParameters", "myCvtStringToStringList",
                      "myProgsError",
                      "String to Stringlist conversion needs no arguments",
                      (String *)NULL, (Cardinal *)NULL);

    str = (String)fromVal->addr;
    if (!str) done(StringListPtr, NULL);

    result = mynew(StringList);
    Nr = 0;
    for (ptr = str; (ch = *ptr) != 0; ptr++) if (ch == '\n') Nr++;
    if (ptr > str && ptr[-1] != '\n') Nr++;
    result->Nr = Nr;
    result->String = Str = mynews(char *, Nr);
    result->Length = Len = mynews(size_t, Nr);
    From = str;
    while (*From == ' ' || *From == '\t') From++;
    ptr = From;
    while ((ch = *ptr) != 0)
        if (ch == '\n') {
            *Len = ptr-From;
            *Str = mystrndup(From, *Len);
            Len++, Str++;
            ptr++;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            From = ptr;
        } else ptr++;
    if (ptr > str && ptr[-1] != '\n') {
        *Len = ptr-From;
        *Str = mystrndup(From, *Len);
    }
    done(StringListPtr, result);
}

void MyFreeStringList(StringList *target)
{
    char      **Str;
    int         i;

    if (target) {
        for (i = target->Nr, Str = target->String; i>0; i--, Str++)
            myfree(*Str);
        myfree(target);
    }
}

/*ARGSUSED*/
static void MyCvtFreeStringList(XtAppContext App, XrmValuePtr To,
                                XtPointer ConvertData,
                                XrmValuePtr Args, Cardinal *n)
{
    MyFreeStringList(*(StringList **) To->addr);
}

/*ARGSUSED*/
static Boolean
MyCvtStringToStringPairList(Display *disp,
                            XrmValuePtr args, Cardinal *num_args,
                            XrmValuePtr fromVal, XrmValuePtr toVal,
                            XtPointer *ConvertData)
{
    String str;
    char  *ptr, **Str1, **Str2, *Str, *Sep, ch;
    int    Nr;
    size_t         *Len1, *Len2;
    StringPairList *result;

    if (*num_args != 0)
        XtAppErrorMsg(XtDisplayToApplicationContext(disp),
                      "wrongParameters", "myCvtStringToStringList",
                      "myProgsError",
                      "String to Stringlist conversion needs no arguments",
                      (String *)NULL, (Cardinal *)NULL);

    str = (String)fromVal->addr;
    if (!str) done(StringPairListPtr, NULL);

    result = mynew(StringPairList);
    Nr = 0;
    for (ptr = str; (ch = *ptr) != 0; ptr++) if (ch == '\n') Nr++;
    if (ptr > str && ptr[-1] != '\n') Nr++;
    result->Nr = Nr;
    result->Length1 = Len1 = mynews(size_t, Nr);
    result->String1 = Str1 = mynews(char *, Nr);
    result->Length2 = Len2 = mynews(size_t, Nr);
    result->String2 = Str2 = mynews(char *, Nr);
    Str = str;
    while (*Str == ' ' || *Str == '\t') Str++;
    ptr = Str;
    while ((ch = *ptr) != 0)
        if (ch == '\n') {
            *Len1 = ptr-Str;
            Sep = memchr(Str, ',', *Len1);
            if (Sep) {
                *Len1 = Sep-Str;
                *Str1 = mystrndup(Str, *Len1);
                Sep++;
                while (*Sep == ' ' || *Sep == '\t') Sep++;
                *Len2 = ptr-Sep;
                *Str2 = mystrndup(Sep, *Len2);
            } else {
                *Str1 = mystrndup(Str, *Len1);
                *Str2 = NULL;
                *Len2 = 0;
            }
            Len1++, Str1++; Len2++; Str2++;
            ptr++;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            Str = ptr;
        } else ptr++;
    if (ptr > str && ptr[-1] != '\n') {
        *Len1 = ptr-Str;
        Sep = memchr(Str, ',', *Len1);
        if (Sep) {
            *Len1 = Sep-Str;
            *Str1 = mystrndup(Str, *Len1);
            Sep++;
            while (*Sep == ' ' || *Sep == '\t') Sep++;
            *Len2 = ptr-Sep;
            *Str2 = mystrndup(Sep, *Len2);
        } else {
            *Str1 = mystrndup(Str, *Len1);
            *Str2 = NULL;
            *Len2 = 0;
        }
    }
    done(StringPairListPtr, result);
}

void MyFreeStringPairList(StringPairList *target)
{
    char          **Str1, **Str2;
    int             i;

    if (target) {
        for (i = target->Nr, Str1 = target->String1, Str2 = target->String2;
             i>0; i--, Str1++, Str2++) {
            myfree(*Str1);
            myfree(*Str2);
        }
        myfree(target);
    }
}

/*ARGSUSED*/
static void MyCvtFreeStringPairList(XtAppContext App, XrmValuePtr To,
                                    XtPointer ConvertData,
                                    XrmValuePtr Args, Cardinal *n)
{
    MyFreeStringPairList(*(StringPairList **) To->addr);
}

#ifndef   NOSTRINTOWIDGET
/*ARGSUSED*/
Boolean XmuNewCvtStringToWidget(Display *dpy,
                                XrmValuePtr args, Cardinal *num_args,
                                XrmValuePtr fromVal, XrmValuePtr toVal,
                                XtPointer *converter_data)
{
    Widget w, parent;

    if (*num_args != 1)
	XtAppWarningMsg(XtDisplayToApplicationContext(dpy),
			"wrongParameters","myCvtStringToWidget",
                        "myProgsError",
                        "String To Widget conversion needs parent argument",
			(String *)NULL, (Cardinal *)NULL);

    parent = *(Widget *) args[0].addr;
    w      = MyNameToWidget(parent, fromVal->addr);
    if (w) done(Widget, w);
    XtDisplayStringConversionWarning(dpy, (String)fromVal->addr, XtRWidget);
    return False;
}

/* ARGSUSED */
void XmuCvtStringToWidget(XrmValuePtr args, Cardinal *num_args,
                          XrmValuePtr fromVal, XrmValuePtr toVal)
{
    static Widget w, parent;

    if (*num_args != 1)
	XtErrorMsg("wrongParameters", "cvtStringToWidget", "xtToolkitError",
		   "StringToWidget conversion needs parent arg", NULL, 0);

    parent = *(Widget *)args[0].addr;
    parent = MyNameToWidget(parent, fromVal->addr);
    if (parent) {
        w = parent;
        olddone(&w, Widget);
    }
    XtStringConversionWarning(fromVal->addr, XtRWidget);
    toVal->addr = NULL;
    toVal->size = 0;
}
#endif /* NOSTRINGTOWIDGET */

/*****************************************************************************/
/* Make all converters available                                             */
/*****************************************************************************/

void GetConverters(void)
{
    XtSetTypeConverter(XtRString, XtRPixmap, MyCvtStringToPixmap,
                       ScreenConvertArg, XtNumber(ScreenConvertArg),
                       XtCacheByDisplay, PixmapDestructor);
    XtSetTypeConverter(XtRString, XtRPixel, MyCvtStringToPixel,
                       ColorConvertArg, XtNumber(ColorConvertArg),
                       XtCacheByDisplay, MyFreePixel);
    XtSetTypeConverter(XtRString, XtRStringList, MyCvtStringToStringList,
                       NULL, 0, XtCacheNone, MyCvtFreeStringList);
    XtSetTypeConverter(XtRString, XtRStringPairList,
                       MyCvtStringToStringPairList,
                       NULL, 0, XtCacheNone, MyCvtFreeStringPairList);
    XtSetTypeConverter(XtRString, XtRBitmap, MyCvtStringToBitmap,
                       ScreenConvertArg, XtNumber(ScreenConvertArg),
                       XtCacheByDisplay, PixmapDestructor);
#ifndef   NOSTRINTOWIDGET
    XtSetTypeConverter(XtRString, XtRWidget, XmuNewCvtStringToWidget,
                       ParentConvertArg, XtNumber(ParentConvertArg),
                       XtCacheNone, 0);
#endif /* NOSTRINGTOWIDGET */
}
