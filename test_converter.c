#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <string.h>

extern Boolean MyCvtStringToPixmap(Display *dpy, XrmValuePtr args, Cardinal *num_args,
                                   XrmValuePtr fromVal, XrmValuePtr toVal,
                                   XtPointer *converter_data);

int main(int argc, char *argv[]) {
    XtAppContext app_con;
    Widget toplevel;
    XrmValue from, to;
    XtPointer converter_data;
    Cardinal num_args = 1;
    XrmValue args[1];
    Screen *screen;
    
    toplevel = XtAppInitialize(&app_con, "TestConverter", NULL, 0,
                               &argc, argv, NULL, NULL, 0);
    
    screen = XtScreen(toplevel);
    args[0].addr = (XtPointer) &screen;
    args[0].size = sizeof(Screen *);
    
    from.addr = "pixmap(/home/cahill/Claude_Projects/fresh-start/board.xpm)";
    from.size = strlen(from.addr) + 1;
    to.addr = NULL;
    to.size = 0;
    
    printf("Testing custom converter with: %s\n", (char*)from.addr);
    
    Boolean result = MyCvtStringToPixmap(XtDisplay(toplevel), args, &num_args,
                                         &from, &to, &converter_data);
    
    if (result) {
        printf("Converter succeeded! Pixmap created.\n");
        if (to.addr) {
            printf("Pixmap address: %p\n", *(Pixmap*)to.addr);
        }
    } else {
        printf("Converter failed!\n");
    }
    
    return 0;
}