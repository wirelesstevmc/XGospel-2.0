#include "my/myxlib.h"
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>

/* Simple test resources */
String test_resources[] = {
    "*test.background: red",
    "*test.foreground: yellow", 
    "*test.label: RESOURCE LOADED!",
    NULL
};

int main(int argc, char *argv[])
{
    XtAppContext app_context;
    Widget toplevel, label;
    Arg args[1];
    
    args[0].name = NULL; /* Empty args */
    
    toplevel = MyAppInitialize(&app_context, "TestResources",
                               NULL, 0, &argc, argv, test_resources,
                               args, 0, NULL, 0, NULL);
    
    printf("Creating test widget...\n");
    fflush(stdout);
    
    /* Create widget that should get resources applied */
    label = XtVaCreateManagedWidget("test", labelWidgetClass, toplevel, NULL);
    
    /* Check what values the widget actually has */
    String label_text;
    Pixel bg, fg;
    XtVaGetValues(label,
                  XtNlabel, &label_text,
                  XtNbackground, &bg,
                  XtNforeground, &fg,
                  NULL);
    
    printf("Widget values: label='%s', bg=%lu, fg=%lu\n", 
           label_text ? label_text : "NULL", bg, fg);
    fflush(stdout);
    
    XtRealizeWidget(toplevel);
    printf("Test completed\n");
    fflush(stdout);
    
    return 0;
}