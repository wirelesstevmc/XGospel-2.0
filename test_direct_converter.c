#include "my/myxlib.h"
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>

int main(int argc, char *argv[])
{
    XtAppContext app_context;
    Widget toplevel;
    XrmValue from, to;
    XrmValue args[3];
    Cardinal num_args = 3;
    Screen *screen;
    Colormap colormap;
    Cardinal depth;
    
    toplevel = XtVaAppInitialize(&app_context, "TestDirectConverter", NULL, 0,
                                &argc, argv, NULL, NULL);
    
    screen = XtScreen(toplevel);
    colormap = DefaultColormapOfScreen(screen);
    depth = DefaultDepthOfScreen(screen);
    
    /* Set up arguments */
    args[0].addr = (XtPointer)&screen;
    args[0].size = sizeof(Screen *);
    args[1].addr = (XtPointer)&colormap;
    args[1].size = sizeof(Colormap);
    args[2].addr = (XtPointer)&depth;
    args[2].size = sizeof(Cardinal);
    
    /* Set up conversion values */
    from.addr = "pixmap(board.xpm)";
    from.size = strlen(from.addr) + 1;
    to.addr = NULL;
    to.size = 0;
    
    printf("DEBUG: Calling MyCvtStringToPixmapOld directly...\n");
    fflush(stdout);
    
    extern void MyCvtStringToPixmapOld(XrmValuePtr args, Cardinal *num_args,
                                       XrmValuePtr fromVal, XrmValuePtr toVal);
    
    /* Call our converter directly */
    MyCvtStringToPixmapOld(args, &num_args, &from, &to);
    
    printf("DEBUG: Direct converter call completed\n");
    fflush(stdout);
    
    return 0;
}