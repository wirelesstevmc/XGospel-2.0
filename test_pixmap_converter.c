#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <stdio.h>
#include <string.h>

/* Include our custom converter functions */
extern void GetConverters(XtAppContext app_con);

int main(int argc, char *argv[]) {
    XtAppContext app_con;
    Widget toplevel, button;
    Display *dpy;
    
    /* Initialize X toolkit */
    XtToolkitInitialize();
    app_con = XtCreateApplicationContext();
    
    /* Register our custom converters BEFORE processing resources */
    printf("Registering custom converters...\n");
    GetConverters(app_con);
    
    /* Now initialize application */
    dpy = XtOpenDisplay(app_con, NULL, "TestPixmap", "TestPixmap", 
                       NULL, 0, &argc, argv);
    if (!dpy) {
        printf("ERROR: Cannot open display\n");
        return 1;
    }
    
    toplevel = XtVaAppCreateShell("TestPixmap", "TestPixmap",
                                  applicationShellWidgetClass, dpy,
                                  NULL);
    
    /* Create a button with pixmap background resource */
    button = XtVaCreateManagedWidget("test", commandWidgetClass, toplevel,
                                     XtNbackgroundPixmap, "pixmap(/home/cahill/Claude_Projects/fresh-start/board.xpm)",
                                     XtNlabel, "Test Pixmap",
                                     NULL);
    
    XtRealizeWidget(toplevel);
    
    printf("Widget created with pixmap background. Check for conversion warnings.\n");
    printf("If no warnings appear, the converter is working!\n");
    
    return 0;
}