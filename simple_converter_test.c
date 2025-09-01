#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <string.h>

extern void GetConverters(XtAppContext app_con);

int main(int argc, char *argv[]) {
    XtAppContext app_con;
    Widget toplevel;
    XrmValue from, to;
    Boolean result;
    
    toplevel = XtAppInitialize(&app_con, "TestApp", NULL, 0, &argc, argv, NULL, NULL, 0);
    
    printf("Calling GetConverters...\n");
    GetConverters(app_con);
    printf("GetConverters completed\n");
    
    from.addr = "pixmap(/home/cahill/Claude_Projects/fresh-start/board.xpm)";
    from.size = strlen(from.addr) + 1;
    to.addr = NULL;
    to.size = 0;
    
    printf("Testing XtConvertAndStore with: %s\n", (char*)from.addr);
    
    result = XtConvertAndStore(toplevel, XtRString, &from, XtRPixmap, &to);
    
    if (result) {
        printf("Conversion succeeded!\n");
    } else {
        printf("Conversion failed!\n");
    }
    
    return 0;
}