#include "myxlib.h"
#include "mymalloc.h"
#include "except.h"
#include "YShell.h"

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>

#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Grip.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Panner.h>
#include <X11/Xaw/Repeater.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include "SmeLabel.h"
#include <X11/Xaw/SmeLine.h>
#include "SmeToggle.h"
#include <X11/Xaw/StripChart.h>
#include <X11/Xaw/Toggle.h>

#include <X11/Xaw/Box.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Porthole.h>
# include "Tree.h"
#include <X11/Xaw/Viewport.h>
#ifdef XAW3D
# include <X11/Xaw3d/SmeBSB.h>
/* # include <X11/Xaw3d/Layout.h> */
#else  /* XAW3D */
# include <X11/Xaw/SmeBSB.h>
#endif /* XAW3D */
#if XlibSpecificationRelease <= 5
# include <X11/Xaw/Clock.h>
# include <X11/Xaw/Logo.h>
# include <X11/Xaw/Mailbox.h>
#else  /* XlibSpecificationRelease */
# include <X11/Xaw/MultiSink.h>
# include <X11/Xaw/MultiSrc.h>
#endif /* XlibSpecificationRelease */

#include "Canvas.h"
#include "TearofMenu.h"

#include <stdarg.h>

WidgetClass LookupClass(const NameClassList Base, const char *Name)
{
    NameClass *Here;

    for (Here = Base; Here; Here = Here->Next)
        if (0 == strcmp(Here->Name, Name)) return Here->Class;
    return NULL;
}

void FreeClassList(NameClassList Base)
{
    NameClass *Here, *Next;

    for (Here = Base; Here; Here = Next) {
        Next = Here->Next;
        myfree(Here);
    }
}

void AddClassList(NameClassList *Base, const char *Name, WidgetClass class)
{
    NameClass *NewNameClass;
    size_t     length;

    length = strlen(Name);
    NewNameClass = (NameClass *) mymalloc(sizeof(NameClass)+length);
    memcpy(NewNameClass->Name, Name, length+1);
    NewNameClass->Class = class;
    
    NewNameClass->Next = *Base;
    *Base              = NewNameClass;
}

NameClassList ExtraClassList(const char *Name, ...)
{
     NameClassList Base;
     WidgetClass   Class;
     va_list       Args;
     
     Base = InitClassList();
     WITH_HANDLING {
         WITH_UNWIND {
             for (va_start(Args, Name);
                  Name;
                  Name = va_arg(Args, const char *)) {
                 Class = va_arg(Args, WidgetClass);
                 AddClassList(&Base, Name, Class);
             }
         } ON_UNWIND {
             va_end(Args);
         } END_UNWIND;
     } ON_EXCEPTION {
         FreeClassList(Base);
     } END_HANDLING;
     return Base;
}

void NameClassArg(Arg *arg, const char *Name, ...)
{
     NameClassList Base;
     WidgetClass   Class;
     va_list       Args;
     
     Base = InitClassList();
     WITH_HANDLING {
         WITH_UNWIND {
             for (va_start(Args, Name);
                  Name;
                  Name = va_arg(Args, const char *)) {
                 Class = va_arg(Args, WidgetClass);
                 AddClassList(&Base, Name, Class);
             }
         } ON_UNWIND {
             va_end(Args);
         } END_UNWIND;
     } ON_EXCEPTION {
         FreeClassList(Base);
     } END_HANDLING;
     XtSetArg(*arg, (String) XtNnameClassList, (XtArgVal) Base);
}

/* See also AllWidgets.c in the Xt distribution */
NameClassList InitClassList(void)
{
    NameClassList Base;

    Base = NULL;
    AddClassList(&Base, "AsciiText",   asciiTextWidgetClass);
    AddClassList(&Base, "AsciiSink",   asciiSinkObjectClass);    
    AddClassList(&Base, "AsciiSource", asciiSrcObjectClass);
#ifdef ASCII_DISK    
    AddClassList(&Base, "AsciiDisk",   asciiDiskWidgetClass);
#endif
#ifdef ASCII_STRING
    AddClassList(&Base, "AsciiString", asciiStringWidgetClass);
#endif
    AddClassList(&Base, "Canvas",      canvasWidgetClass);
    AddClassList(&Base, "Core",        coreWidgetClass);
    AddClassList(&Base, "Command",     commandWidgetClass);
    AddClassList(&Base, "Grip",        gripWidgetClass);
    AddClassList(&Base, "Label",       labelWidgetClass);
    AddClassList(&Base, "List",        listWidgetClass);
#if XlibSpecificationRelease <= 5
#if 0 /* ??? not available on Solaris 5.6 */
    AddClassList(&Base, "Clock",       clockWidgetClass);
    AddClassList(&Base, "Logo",        logoWidgetClass);
    AddClassList(&Base, "Mailbox",     mailboxWidgetClass);
#endif
#else
    AddClassList(&Base, "MultiSink",   multiSinkObjectClass);    
    AddClassList(&Base, "MultiSource", multiSrcObjectClass);    
#endif /* XlibSpecificationRelease */
    AddClassList(&Base, "MenuButton",  menuButtonWidgetClass);
    AddClassList(&Base, "Object",      objectClass);            
    AddClassList(&Base, "Panner",      pannerWidgetClass);
    AddClassList(&Base, "RectObject",  rectObjClass);           
    AddClassList(&Base, "Repeater",    repeaterWidgetClass);
    AddClassList(&Base, "Scrollbar",   scrollbarWidgetClass);
    AddClassList(&Base, "Sme",         smeObjectClass);         
    AddClassList(&Base, "SmeBSB",      smeBSBObjectClass);      
    AddClassList(&Base, "SmeLabel",    smeLabelObjectClass);     
    AddClassList(&Base, "SmeLine",     smeLineObjectClass);     
    AddClassList(&Base, "SmeToggle",   smeToggleObjectClass);     
    AddClassList(&Base, "StripChart",  stripChartWidgetClass);
    AddClassList(&Base, "Simple",      simpleWidgetClass);
    AddClassList(&Base, "SimpleMenu",  simpleMenuWidgetClass);
    AddClassList(&Base, "TearofMenu",  tearofMenuWidgetClass);
    AddClassList(&Base, "Text",        textWidgetClass);
    AddClassList(&Base, "TextSink",    textSinkObjectClass);    
    AddClassList(&Base, "TextSource",  textSrcObjectClass);     
    AddClassList(&Base, "Toggle",      toggleWidgetClass);

    AddClassList(&Base, "Box",         boxWidgetClass);
    AddClassList(&Base, "Composite",   compositeWidgetClass);
    AddClassList(&Base, "Constraint",  constraintWidgetClass);
    AddClassList(&Base, "Dialog",      dialogWidgetClass);
    AddClassList(&Base, "Form",        formWidgetClass);
#ifdef XAW3D
/* Seems this one is not always compiled in
    AddClassList(&Base, "Layout",      layoutWidgetClass);
*/
#endif /* XAW3D */
    AddClassList(&Base, "Paned",       panedWidgetClass);
    AddClassList(&Base, "Porthole",    portholeWidgetClass);
    AddClassList(&Base, "Tree",        treeWidgetClass);
    AddClassList(&Base, "Viewport",    viewportWidgetClass);

    AddClassList(&Base, "ApplicationShell", applicationShellWidgetClass);
    AddClassList(&Base, "OverrideShell",    overrideShellWidgetClass);
    AddClassList(&Base, "Shell",            shellWidgetClass);
    AddClassList(&Base, "TransientShell",   transientShellWidgetClass);
    AddClassList(&Base, "TopLevelShell",    topLevelShellWidgetClass);
    AddClassList(&Base, "VendorShell",      vendorShellWidgetClass);
    AddClassList(&Base, "WmShell",          wmShellWidgetClass);

    return Base;
}
