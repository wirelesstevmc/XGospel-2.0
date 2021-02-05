#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/Toggle.h>

#include <mymalloc.h>
#include <except.h>
#include <myxlib.h>

#include "messages.h"
#include "utils.h"
#include "xgospel.h"

Widget ServerButton;

static Widget ServerBeep, ServerRaise, ServerErrorBeep, ServerErrorRaise;
static Widget ServerInfo, ServerOverwrite, ServerRoot;
static char *ServerFileName;

static void ChangeServerFilename(Widget w,
                                 XtPointer clientdata, XtPointer calldata)
{
    char **Filename;
    Widget Root;

    Filename = (char **) clientdata;
    Root = ChangeFilename(toplevel, "serverFilename",
                          "Enter name of igs messages file",
                          Filename, "filename", *Filename, NULL);
    MyDependsOn(Root, w);
}

static void SaveServer(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Boolean                   Overwrite;
    static char const * const ErrorType = "Igs messages save error";

    if (ServerInfo) {
        if (ServerOverwrite) XtVaGetValues(ServerOverwrite, XtNstate,
                                           (XtArgVal) &Overwrite, NULL);
        else                 Overwrite = False;

        SaveWrite(ServerFileName, Overwrite,
                  ServerErrorBeep, ServerErrorRaise, ErrorType,
                  SaveTextFun, (XtPointer) ServerInfo);
    } else {
        IfBell(ServerErrorBeep);
        IfRaise(ServerErrorRaise, ServerErrorRaise);
        PopMessage(ErrorType, "Igs messages were ignored so there is "
                   "nothing to be saved");
    }
}

void InitMessages(Widget TopLevel)
{
    Widget ServerCollect, ServerSave, ServerFile;

/*
    XtAppAddActions(XtWidgetToApplicationContext(TopLevel),
                    actionTable, XtNumber(actionTable));
*/

    ServerRoot = MyVaCreateManagedWidget("igsMessages", TopLevel, NULL);
    ServerBeep       = XtNameToWidget(ServerRoot, "*beep");
    ServerRaise      = XtNameToWidget(ServerRoot, "*raise");
    ServerInfo       = XtNameToWidget(ServerRoot, "*info");
    ServerOverwrite  = XtNameToWidget(ServerRoot, "*overwrite");
    ServerErrorBeep  = XtNameToWidget(ServerRoot, "*errorBeep");
    ServerErrorRaise = XtNameToWidget(ServerRoot, "*errorRaise");

    ServerSave       = XtNameToWidget(ServerRoot, "*save");
    if (ServerSave) XtAddCallback(ServerSave, XtNcallback, SaveServer, NULL);
    ServerFile       = XtNameToWidget(ServerRoot, "*file");
    if (ServerFile)
        XtAddCallback(ServerFile, XtNcallback, ChangeServerFilename,
                      (XtPointer) &ServerFileName);

    ServerFileName = StringToFilename(appdata.IgsMessageFilename,
                                      'T', "IGSmessages", 't', "_", 0);
                                      
    XtAddCallback(ServerRoot, XtNdestroyCallback, CallFree,
                  (XtPointer) ServerFileName);
    if (ServerButton) {
        XtAddCallback(ServerButton, XtNcallback,         CallToggleUpDown,
                      (XtPointer) ServerRoot);
        XtAddCallback(ServerRoot,   XtNpopupCallback,    CallToggleOn,
                      (XtPointer) ServerButton);
        XtAddCallback(ServerRoot,   XtNpopdownCallback,  CallToggleOff,
                      (XtPointer) ServerButton);
        CallToggleUpDown(ServerButton, (XtPointer) ServerRoot, NULL);
    }
    XtRealizeWidget(ServerRoot);
    ServerCollect    = XtNameToWidget(ServerRoot, "*collect");
    if (ServerCollect) XtInstallAllAccelerators(ServerCollect, ServerCollect);
    DeleteProtocol(ServerRoot);
}

#ifndef HAVE_NO_STDARG_H
void  ServerMessage(const char *Message, ...)
#else  /* HAVE_NO_STDARG_H */
void  ServerMessage(va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    char      Buffer[2048];
    va_list args;

#ifndef HAVE_NO_STDARG_H
    va_start(args, Message);
#else  /* HAVE_NO_STDARG_H */
    const char *Message;
    va_start(args);
    Message = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    vsprintf(Buffer, Message, args);
    va_end(args);
    AddText(ServerInfo, "%s", Buffer);
    IfBell(ServerBeep);
    IfRaise(ServerRaise, ServerRaise);
}
