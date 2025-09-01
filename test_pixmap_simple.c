#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <stdio.h>

int main() {
    Display *dpy;
    Screen *screen;
    Pixmap pixmap;
    int rc;
    
    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        printf("Cannot open display\n");
        return 1;
    }
    
    screen = DefaultScreenOfDisplay(dpy);
    
    rc = XpmReadFileToPixmap(dpy, RootWindowOfScreen(screen), 
                           "/home/cahill/board.xpm", &pixmap, NULL, NULL);
    
    printf("XpmReadFileToPixmap result: %d\n", rc);
    if (rc == XpmSuccess) {
        printf("Successfully loaded pixmap\n");
        XFreePixmap(dpy, pixmap);
    } else {
        printf("Failed to load pixmap\n");
    }
    
    XCloseDisplay(dpy);
    return 0;
}