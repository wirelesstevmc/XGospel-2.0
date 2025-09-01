#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    XtAppContext app_con;
    Widget toplevel;
    XrmValue from, to;
    Boolean result;
    
    toplevel = XtAppInitialize(&app_con, "DebugConverters", NULL, 0,
                               &argc, argv, NULL, NULL, 0);
    
    from.addr = "pixmap(/home/cahill/Claude_Projects/fresh-start/board.xpm)";
    from.size = strlen(from.addr) + 1;
    to.addr = NULL;
    to.size = 0;
    
    printf("Testing default X11 converter with: %s\n", (char*)from.addr);
    
    result = XtConvertAndStore(toplevel, XtRString, &from, XtRPixmap, &to);
    
    if (result) {
        printf("Default converter succeeded!\n");
    } else {
        printf("Default converter failed!\n");
    }
    
    return 0;
}