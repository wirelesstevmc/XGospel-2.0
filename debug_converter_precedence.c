#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <stdio.h>

/* Test what converters are registered and their precedence */

static Boolean TestConverter1(Display *disp,
                             XrmValuePtr args, Cardinal *num_args,
                             XrmValuePtr fromVal, XrmValuePtr toVal,
                             XtPointer *ConvertData)
{
    printf("TestConverter1 called with: '%s'\n", (char*)fromVal->addr);
    return False; /* Always fail to see if next converter is called */
}

static Boolean TestConverter2(Display *disp,
                             XrmValuePtr args, Cardinal *num_args,
                             XrmValuePtr fromVal, XrmValuePtr toVal,
                             XtPointer *ConvertData)
{
    printf("TestConverter2 called with: '%s'\n", (char*)fromVal->addr);
    return False; /* Always fail to see if next converter is called */
}

int main(int argc, char **argv)
{
    Widget toplevel;
    XtAppContext app_context;
    XrmValue from, to;
    Boolean result;
#define offset(field) (XtPointer) XtOffset(WidgetRec *, core.field)
    static XtConvertArgRec ScreenConvertArg[] = {
        { XtWidgetBaseOffset, offset(screen),   sizeof(Screen *) },
        { XtWidgetBaseOffset, offset(colormap), sizeof(Colormap) },
        { XtWidgetBaseOffset, offset(depth),    sizeof(Cardinal) }
    };
#undef offset
    
    toplevel = XtAppInitialize(&app_context, "TestApp", NULL, 0, 
                               &argc, argv, NULL, NULL, 0);
    
    printf("=== Testing converter precedence ===\n");
    
    /* Register two test converters to see order */
    printf("Registering TestConverter1 with XtSetTypeConverter...\n");
    XtSetTypeConverter(XtRString, XtRPixmap, TestConverter1,
                       ScreenConvertArg, XtNumber(ScreenConvertArg),
                       XtCacheNone, NULL);
    
    printf("Registering TestConverter2 with XtAppSetTypeConverter...\n");                   
    XtAppSetTypeConverter(app_context, XtRString, XtRPixmap, TestConverter2,
                          ScreenConvertArg, XtNumber(ScreenConvertArg),
                          XtCacheNone, NULL);
    
    /* Test conversion */
    from.size = strlen("pixmap(/test.xpm)") + 1;
    from.addr = "pixmap(/test.xpm)";
    to.size = sizeof(Pixmap);
    to.addr = NULL;
    
    printf("\nTesting conversion of 'pixmap(/test.xpm)':\n");
    result = XtConvertAndStore(toplevel, XtRString, &from, XtRPixmap, &to);
    printf("Conversion result: %s\n", result ? "True" : "False");
    
    return 0;
}