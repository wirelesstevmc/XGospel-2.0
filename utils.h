#ifndef UTILS_H
# define UTILS_H

# ifdef	__GNUC__
/* Figure out how to declare functions that (1) depend only on their
   parameters and have no side effects, or (2) don't return.  */
#  if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) /* Old GCC way. */
#   define       __PRINTF2
#  else                                                     /* New GCC way. */
#   define       __PRINTF2     __attribute__ ((format (printf, 2, 3)))
#  endif
# else	/* Not GCC.  */
#  define        __PRINTF2
# endif	/* GCC.  */

# include <stdio.h>
# include <regex.h>
# include <X11/Intrinsic.h>

typedef struct _RegexYesNo RegexYesNo;
struct _RegexYesNo {
    regex_t *YesPatterns,  *NoPatterns;
    int      NrYesPatterns, NrNoPatterns;
};

#ifndef _MYXLIB_H
  struct _StringList;
#endif

/* Functions to create/change/destroy pass/kill patterns */
extern void        EmptyRegexYesNo(RegexYesNo *Pattern);
extern void        FreeRegexYesNo(RegexYesNo *Convert);
extern const char *CompileRegexYesNo(struct _StringList *YesBase,
                                     struct _StringList *NoBase,
                                     RegexYesNo *Compiled);
extern const char *TestRegexYesNo(const char *Text, RegexYesNo *Pattern,
                                  int *Yes, int *No);
extern void EditStringList(Widget w, const char *Title, const char *Error,
                           struct _StringList **Pass,struct _StringList **Kill,
                           RegexYesNo *Pattern, int which);

extern void      InitUtils(Widget TopLevel);
extern FILE     *OpenWrite(const char *FileName, Boolean Overwrite);
extern XtPointer SaveWrite(const char *FileName, Boolean Overwrite,
                           Widget Bell, Widget Raize, const char *ErrorType,
                           XtPointer (*WriteFun)(FILE *fp, XtPointer Closure),
                           XtPointer Closure);
extern XtPointer SaveTextFun(FILE *fp, XtPointer Closure);
extern void      ChangeSgfFilename(Widget w,
                                   XtPointer clientdata, XtPointer calldata);
extern void      ChangePsFilename(Widget w,
                                  XtPointer clientdata, XtPointer calldata);
extern void      ChangeKibitzFilename(Widget w, XtPointer clientdata,
                                      XtPointer calldata);
extern void      IfBell(Widget w);
extern void      IfRaise(Widget w, Widget root);
extern void      SetWidgetTitles(Widget w, char *(*fun)(const char *Pattern,
                                                        XtPointer Closure),
                                 XtPointer closure);
extern void      SetWidgetProperty(Widget w,
				   const char *Name, const char *Class,
				   char *(*fun)(const char *Pattern,
						XtPointer Closure),
				   XtPointer Closure);
extern int ConvertSize(const char *Name, size_t *Target,
                       XtPointer Closure, int Free);
extern int ConvertPositive(const char *Name, int *Target,
                           XtPointer Closure, int Free);
extern int ConvertNatural(const char *Name, int *Target,
                          XtPointer Closure, int Free);
extern int ConvertBoolean(const char *Name,Boolean *Target,
                          XtPointer Closure, int Free);
# ifndef   HAVE_NO_STDARG_H
extern char     *StringToFilename(const char *Filename, ...);
extern Widget    AskString(Widget toplevel, void (*Callback)(XtPointer),
                           XtPointer Closure, const char *title, ...);
extern Widget    ChangeFilename(Widget toplevel, const char *Type,
                                const char *Title, char **Filename, ...);
extern Widget    PopMessage(const char *Title,
                            const char *Format, ...) __PRINTF2;
# else  /* HAVE_NO_STDARG_H */
extern char     *StringToFilename();
extern Widget    AskString();
extern Widget    ChangeFilename();
extern Widget    PopMessage();
# endif /* HAVE_NO_STDARG_H */
#endif /* UTILS_H */
