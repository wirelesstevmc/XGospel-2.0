#include "my/myxlib.h"
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>

/* Test fallback resources with pixmap */
String test_fallback_resources[] = {
    "*test.backgroundPixmap: pixmap(board.xpm)",
    "*testlabel.backgroundPixmap: pixmap(board.xpm)", 
    NULL
};

int main(int argc, char *argv[])
{
    XtAppContext app_context;
    Widget toplevel, label, form;
    
    /* Register converters immediately after XtToolkitInitialize */
    XtToolkitInitialize();
    
    /* Register our converter RIGHT after toolkit init, before anything else */
    printf("DEBUG: Registering converters immediately after XtToolkitInitialize\n");
    fflush(stdout);
    
    static XtConvertArgRec ScreenConvertArg[] = {
        {XtWidgetBaseOffset, (XtPointer) XtOffsetOf(WidgetRec, core.screen), sizeof(Screen *)},
        {XtWidgetBaseOffset, (XtPointer) XtOffsetOf(WidgetRec, core.colormap), sizeof(Colormap)},
        {XtWidgetBaseOffset, (XtPointer) XtOffsetOf(WidgetRec, core.depth), sizeof(Cardinal)}
    };
    
    extern void MyCvtStringToPixmapOld(XrmValuePtr args, Cardinal *num_args,
                                       XrmValuePtr fromVal, XrmValuePtr toVal);
    
    /* Register immediately with XtAddConverter */
    XtAddConverter(XtRString, XtRPixmap, MyCvtStringToPixmapOld,
                   ScreenConvertArg, 3);
    
    printf("DEBUG: Direct converter registration completed\n");
    fflush(stdout);
    
    app_context = XtCreateApplicationContext();
    
    /* Initialize with fallback resources */
    toplevel = XtVaAppInitialize(&app_context, "TestPixmap", NULL, 0,
                                &argc, argv, test_fallback_resources, NULL);
    
    /* Create widgets that should use the fallback resources */
    form = XtVaCreateManagedWidget("form", formWidgetClass, toplevel, NULL);
    
    printf("Creating test widget...\n");
    fflush(stdout);
    label = XtVaCreateManagedWidget("test", labelWidgetClass, form, NULL);
    
    printf("Creating testlabel widget...\n");
    fflush(stdout);
    label = XtVaCreateManagedWidget("testlabel", labelWidgetClass, form, NULL);
    
    printf("Widgets created, realizing...\n");
    fflush(stdout);
    XtRealizeWidget(toplevel);
    
    printf("Test completed\n");
    fflush(stdout);
    
    return 0;
}