#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <stdio.h>

int main() {
#ifdef HAVE_XPM
    printf("HAVE_XPM is defined\n");
#else
    printf("HAVE_XPM is NOT defined\n");
#endif
    return 0;
}