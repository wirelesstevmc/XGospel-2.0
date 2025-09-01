#include "my/myxlib.h" 
#include "GoBoard.h"
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>

/* Test fallback resources that should use our converter */
String test_fallback_resources[] = {
    "*test.backgroundPixmap: pixmap(board.xpm)",
    "*board.backgroundPixmap: pixmap(board.xpm)",
    "*myboard.backgroundPixmap: pixmap(board.xmp)",  /* This should fail */
    "*iconPixmap: pixmap(builtin(XgospelIcon))",
    NULL
};

int main(int argc, char *argv[])
{
    XtAppContext app_context;
    Widget toplevel, form, board;
    Arg args[10];
    Cardinal num_args;
    
    /* Use our MyAppInitialize which handles converter registration timing */
    num_args = 0;
    toplevel = MyAppInitialize(&app_context, "TestPixmapIntegration",
                               NULL, 0, &argc, argv, test_fallback_resources,
                               args, num_args, NULL, 0, NULL);
    
    printf("MyAppInitialize completed - creating widgets...\n");
    fflush(stdout);
    
    /* Create form container */
    form = XtVaCreateManagedWidget("form", formWidgetClass, toplevel, NULL);
    
    /* Create test widget that should use fallback pixmap */
    printf("Creating test widget (should use fallback pixmap)...\n");
    fflush(stdout);
    XtVaCreateManagedWidget("test", labelWidgetClass, form, NULL);
    
    /* Create board widget that should use board pixmap */
    printf("Creating board widget (should use board pixmap)...\n");
    fflush(stdout);
    board = XtVaCreateManagedWidget("board", boardWidgetClass, form, NULL);
    
    /* Create widget with explicit pixmap that should call our converter */
    printf("Creating widget with explicit pixmap...\n");
    fflush(stdout);
    XtVaCreateManagedWidget("explicit", labelWidgetClass, form,
                           XtNbackgroundPixmap, "pixmap(pagoda.xpm)",
                           NULL);
    
    printf("Realizing widgets...\n");
    fflush(stdout);
    XtRealizeWidget(toplevel);
    
    printf("Integration test completed successfully!\n");
    fflush(stdout);
    
    return 0;
}