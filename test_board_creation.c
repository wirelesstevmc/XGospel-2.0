#include "GoBoard.h"
#include "my/myxlib.h"
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>

int main(int argc, char *argv[])
{
    XtAppContext app_context;
    Widget toplevel, board, form;
    
    /* Initialize application like xgospel does */
    toplevel = XtVaAppInitialize(&app_context, "TestBoard", NULL, 0,
                                &argc, argv, NULL, NULL);
    
    /* Register our converter first */
    extern void GetConverters(XtAppContext app_context);
    GetConverters(app_context);
    
    /* Create a form to hold the board */
    form = XtVaCreateManagedWidget("form", formWidgetClass, toplevel, NULL);
    
    /* Create board widget - this should trigger pixmap conversion from fallback resources */
    printf("Creating board widget without explicit pixmap...\n");
    fflush(stdout);
    board = XtVaCreateManagedWidget("board", boardWidgetClass, form, NULL);
    
    printf("Board widget created successfully\n");
    fflush(stdout);
    
    XtRealizeWidget(toplevel);
    printf("Widgets realized\n");
    fflush(stdout);
    
    return 0;
}