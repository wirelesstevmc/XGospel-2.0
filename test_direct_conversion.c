#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <stdio.h>
#include <string.h>

extern void GetConverters(XtAppContext app_con);

int main(int argc, char *argv[]) {
    XtAppContext app_con;
    Widget toplevel;
    Display *dpy;
    XrmValue from, to;
    Boolean result;
    
    /* Initialize X toolkit */
    XtToolkitInitialize();
    app_con = XtCreateApplicationContext();
    
    /* Register our custom converters */
    printf("Registering custom converters...\n");
    GetConverters(app_con);
    
    /* Initialize application */
    dpy = XtOpenDisplay(app_con, NULL, "TestPixmap", "TestPixmap", 
                       NULL, 0, &argc, argv);
    if (!dpy) {
        printf("ERROR: Cannot open display\n");
        return 1;
    }
    
    toplevel = XtVaAppCreateShell("TestPixmap", "TestPixmap",
                                  applicationShellWidgetClass, dpy,
                                  NULL);
    
    /* Test direct conversion */
    from.addr = "pixmap(/home/cahill/Claude_Projects/fresh-start/board.xpm)";
    from.size = strlen(from.addr) + 1;
    to.addr = NULL;
    to.size = 0;
    
    printf("Testing direct conversion of: %s\n", (char*)from.addr);
    
    result = XtConvertAndStore(toplevel, XtRString, &from, XtRPixmap, &to);
    
    if (result) {
        printf("SUCCESS: Conversion succeeded!\n");
    } else {
        printf("FAILED: Conversion failed!\n");
    }
    
    return 0;
}