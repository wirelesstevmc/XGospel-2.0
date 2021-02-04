AC_DEFUN(AC_PROG_ETAGS, [AC_CHECK_PROG(ETAGS, etags, etags, :)])dnl

AC_DEFUN(AC_PROG_MAKEDEPEND, [AC_CHECK_PROG(MAKEDEPEND, makedepend, makedepend, :)])dnl

dnl many people have an older makedepend. Check if this is the case.
dnl AC_PROG_OPTION_MAKEDEPEND(OPTION, ACTION-IF-FOUND [,ACTION-IF-NOT-FOUND])
AC_DEFUN(AC_PROG_OPTION_MAKEDEPEND,
[dnl Ultrix and Pyramid sh refuse to redirect output of eval, so use subshell.
(eval "$ac_cv_prog_MAKEDEPEND $1 -f \"\"") > conftest.makedepend 2>&1
if egrep "ignoring option" conftest.makedepend >/dev/null 2>&1; then
  ifelse([$3], , :, [rm -rf conftest*
  $3])
ifelse([$2], , , [else
  rm -rf conftest*
  $2
])dnl
fi
rm -f conftest*])

dnl Check whether makedepend accepts the -p option. If not, it is an older
dnl version of makedepend and the user should update his X distribution
AC_DEFUN(AC_PROG_MAKEDEPEND_OPTION_P,
[ AC_MSG_CHECKING(whether makedepend accepts the -p option)
  AC_CACHE_VAL(ac_cv_prog_MAKEDEPEND_option_p,
     AC_PROG_OPTION_MAKEDEPEND(-p,
                               eval "ac_cv_prog_MAKEDEPEND_option_p=yes",
                               eval "ac_cv_prog_MAKEDEPEND_option_p=no"))dnl
  if eval "test \"`echo '$ac_cv_prog_MAKEDEPEND_option_p'`\" = yes"; then
    AC_MSG_RESULT(yes)
    MAKEDEPENDP="$MAKEDEPEND"
  else
    AC_MSG_RESULT(no)
    MAKEDEPENDP=":"
  fi
  AC_SUBST(MAKEDEPENDP)])

dnl Check for gcc version if we are using gcc
AC_DEFUN(AC_PROG_GCC,
  [AC_REQUIRE([AC_PROG_CC])
  old_ac_cv_prog_gcc_version="$ac_cv_prog_gcc_version"
  if test "$ac_cv_prog_gcc" = "yes"; then
    AC_MSG_CHECKING(compiler version)
    ac_cv_prog_gcc_version=`${CC-cc} -v 2>&1 | grep "^gcc"`
    AC_MSG_RESULT($ac_cv_prog_gcc_version)
  fi])

dnl Kludge to get certain defines into makedepend
dnl Seems to be surprisingly portable
AC_DEFUN(AC_PROG_MAKEDEPEND_DEFINES,
  [AC_MSG_CHECKING(what extra options we should give makedepend)
  if test "$old_ac_cv_prog_gcc_version" != "$ac_cv_prog_gcc_version"; then
    unset ac_cv_prog_MAKEDEPEND_dependflags
  fi
  AC_CACHE_VAL(ac_cv_prog_MAKEDEPEND_dependflags,
    [AC_REQUIRE([AC_PROG_GCC])
    cat >conftest.$ac_ext <<EOF
int foo=0;
EOF
    IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="$IFS,"
    for i in `${CC-cc} -v -c conftest.$ac_ext 2>&1`
    do
      case "$i" in 
dnl    (((
dnl     -I*) DEPENDFLAGS="$DEPENDFLAGS $i";;
        -D*) DEPENDFLAGS="$DEPENDFLAGS $i";;
        -A*) test $ac_cv_prog_gcc = yes && \
	     DEPENDFLAGS="$DEPENDFLAGS "`echo "$i" | sed 's/^-A\(.*\)(\(.*\))/-D\1=\2/'`;;
      esac
    done
    IFS="$ac_save_ifs"

    if test "$ac_cv_prog_gcc" = yes; then
      ac_temp_do_includes=no
      IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="
"
      for i in `${CC-cc} -v -c conftest.$ac_ext 2>&1 1>&5 | sed 's/ //g'`
      do
        case "$i" in
dnl       (((
          \#include*:) ac_temp_do_includes=yes;;
          End*of*) ac_temp_do_includes=no;;
          *) test "$ac_temp_do_includes" = yes && \
             DEPENDFLAGS="$DEPENDFLAGS -I$i";;
        esac
      done
      IFS="$ac_save_ifs"
    fi
    rm conftest.$ac_ext
    ac_cv_prog_MAKEDEPEND_dependflags="$DEPENDFLAGS"])
  AC_MSG_RESULT($ac_cv_prog_MAKEDEPEND_dependflags)
  DEPENDFLAGS="$ac_cv_prog_MAKEDEPEND_dependflags"
  AC_SUBST(DEPENDFLAGS)])

dnl Save the user supplied value of DEPEND
AC_DEFUN(AC_SAVE_DEPEND,
[ if test "${DEPEND+set}" = set; then
    MAKEDEPEND="$DEPEND"
  fi])

dnl Save the user supplied value of CFLAGS
AC_DEFUN(AC_SAVE_CFLAGS,
[ AC_BEFORE([$0], [AC_PROG_CC_CFLAGS])dnl
  if test "${CFLAGS+set}" = set; then
    OLD_CFLAGS="$CFLAGS"
  else
    unset OLD_CFLAGS
  fi])

dnl override the gnu default of compiling everything with -g if no initial
dnl CFLAGS is given. xgospel is too big for that kind of jokes.
AC_DEFUN(AC_PROG_CC_CFLAGS,
[ if test "${OLD_CFLAGS+set}" != set; then
    if test $ac_cv_prog_gcc = yes; then
      OPTIMIZE="-O6 -fomit-frame-pointer"
      WARN=     # "-Wall"
    else
      OPTIMIZE="-O"
      WARN=
    fi
    CFLAGS="$OPTIMIZE $WARN"
  fi])

dnl Checking for goldfish_does_blub is a sanity check. If your library has it,
dnl you either have a VERY strange library, or cc isn't a (workin) C compiler
dnl After that compile a VERY trivial program. If that fails, something strange
dnl is happening
AC_DEFUN(AC_PROG_CC_SANITY,
[ AC_MSG_CHECKING(whether your compiler really does something)
  AC_REQUIRE([AC_PROG_CC])
  if test "${ac_cc+set}" != set; then
    set dummy $CC; ac_cc="`echo [$]2 | 
    changequote(, )dnl
      sed -e 's/[^a-zA-Z0-9_]/_/g' -e 's/^[0-9]/_/'`"
    changequote([, ])dnl
  fi
  AC_CACHE_VAL(ac_cv_prog_cc_${ac_cc}_sanity,
    [AC_TRY_LINK([extern int goldfish_does_blub();
int a;],[a=goldfish_does_blub();], eval ac_cv_prog_cc_${ac_cc}_sanity=no,
      AC_TRY_LINK([],[],eval ac_cv_prog_cc_${ac_cc}_sanity=yes,
    eval ac_cv_prog_cc_${ac_cc}_sanity=no))])dnl
  if eval "test \"`echo '$ac_cv_prog_cc_'${ac_cc}_sanity`\" = yes"; then
    AC_MSG_RESULT(yes)
    ifelse([$1], , :, [$1])
  else
    AC_MSG_RESULT(no)
    ifelse([$2], , :, [$2])
  fi])

dnl Check wether compiler supports prototypes
dnl AC_PROG_CC_PROTOTYPES([ACTION-IF-TRUE [,ACTION-IF-FALSE]])
AC_DEFUN(AC_PROG_CC_PROTOTYPES,
[ AC_MSG_CHECKING(whether your compiler understands prototypes)
  AC_REQUIRE([AC_PROG_CC])
  if test "${ac_cc+set}" != set; then
    set dummy $CC; ac_cc="`echo [$]2 | 
    changequote(, )dnl
      sed -e 's/[^a-zA-Z0-9_]/_/g' -e 's/^[0-9]/_/'`"
    changequote([, ])dnl
  fi
  AC_CACHE_VAL(ac_cv_prog_cc_${ac_cc}_prototypes,
    [AC_TRY_COMPILE([extern void perror(const char *);/*],[}*/int main(void){],
eval ac_cv_prog_cc_${ac_cc}_prototypes=yes,
eval ac_cv_prog_cc_${ac_cc}_prototypes=no)])dnl
  if eval "test \"`echo '$ac_cv_prog_cc_'${ac_cc}_prototypes`\" = yes"; then
    AC_MSG_RESULT(yes)
    ifelse([$1], , :, [$1])
  else
    AC_MSG_RESULT(no)
    ifelse([$2], , :, [$2])
  fi])

dnl check for a feature used of some old bison templates
dnl You probably should use the YYOVERFLOW test macro though
AC_DEFUN(AC_PROG_CPP_MACROTEST,
  [AC_MSG_CHECKING(whether cpp accepts tests in macro expansion)
  AC_CACHE_VAL(ac_cv_prog_cpp_macrotest,
  AC_TRY_COMPILE(,[#define fun(a0, a3)
#ifdef BLUB /* Paranoia strikes again */
# undef BLUB
#endif
fun(a1,
#ifdef BLUB
    a2, a3,
#endif
    a4)
int a;],eval "ac_cv_prog_cpp_macrotest=yes", eval "ac_cv_prog_cpp_macrotest=no"))dnl
  if eval "test \"`echo '$ac_cv_prog_cpp_macrotest'`\" = yes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_TEST_IN_MACRO)dnl
  else
    AC_MSG_RESULT(no)
    AC_DEFINE(HAVE_NO_TEST_IN_MACRO)dnl
  fi])

dnl If the compiler survived the HEADER_STDC test, it's silly to check for
dnl the relevant include files again, so cache them.
AC_DEFUN(AC_POST_HEADER_STDC,
  [AC_REQUIRE([AC_HEADER_STDC])
  if test "$ac_cv_header_stdc" = yes; then
    ac_cv_header_stdlib_h=yes
    ac_cv_header_stdarg_h=yes
    ac_cv_header_string_h=yes
    ac_cv_header_ctype_h=yes
    ac_cv_header_float_h=yes
  fi])

dnl Like CHECK_HEADERS, but also set the NO symbols on failure.
dnl This helps if the package should also be compilable not using configure
dnl AC_CHECK_NO_HEADERS(HEADER-FILE... [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN(AC_CHECK_NO_HEADERS,
[for ac_hdr in $1
do
changequote(, )dnl
  ac_tr_hdr=`echo $ac_hdr | tr '[a-z]./\055' '[A-Z]___'`
  ac_yes_tr_hdr=HAVE_$ac_tr_hdr
  ac_no_tr_hdr=HAVE_NO_$ac_tr_hdr
changequote([, ])dnl
AC_CHECK_HEADER($ac_hdr,
  AC_DEFINE_UNQUOTED($ac_yes_tr_hdr) $2,
  AC_DEFINE_UNQUOTED($ac_no_tr_hdr) $3)dnl
done])

dnl The same thing for functions
AC_DEFUN(AC_CHECK_NO_FUNCS,
[for ac_func in $1
do
changequote(, )dnl
  ac_tr_func=`echo $ac_func | tr '[a-z]' '[A-Z]'`
  ac_yes_tr_func=HAVE_$ac_tr_func
  ac_no_tr_func=HAVE_NO_$ac_tr_func
changequote([, ])dnl
AC_CHECK_FUNC($ac_func,
  AC_DEFINE_UNQUOTED($ac_yes_tr_func) $2,
  AC_DEFINE_UNQUOTED($ac_no_tr_func) $3)dnl
done])

dnl strftime is special since it needs an extra library on SCO
AC_DEFUN(AC_FUNC_NO_STRFTIME,
[# strftime is in -lintl on SCO UNIX.
AC_CHECK_LIB(intl, strftime, LIBS="$LIBS -lintl")
AC_CHECK_NO_FUNCS(strftime)])

dnl Check given header part for prototype
dnl AC_CHECK_PROTO(NAME, HEADER-PART, [ACTION-IF-FOUND [,ACTION-IF-NOT-FOUND]])
AC_DEFUN(AC_CHECK_PROTO,
  [ AC_MSG_CHECKING(for $1 prototype)
    AC_CACHE_VAL(ac_cv_proto_$1,
    [AC_REQUIRE([AC_PROG_CPP])dnl
     changequote(, )dnl
     ac_proto_temp="([^a-zA-Z0-9]|^)$1([^a-zA-Z0-9]|$)"
     changequote([, ])dnl
     AC_EGREP_CPP($ac_proto_temp, [$2], 
                  eval "ac_cv_proto_$1=yes", eval "ac_cv_proto_$1=no")])dnl
  if eval "test \"`echo '$ac_cv_proto_'$1`\" = yes"; then
    AC_MSG_RESULT(yes)
    ifelse([$3], , :, [$3])
  else
    AC_MSG_RESULT(no)
    ifelse([$4], , :, [$4])
  fi
])

dnl The corresponding NO version
AC_DEFUN(AC_CHECK_NO_PROTO,
  [changequote(, )dnl
    ac_tr_proto=`echo $1 | tr '[a-z]' '[A-Z]'`
    ac_yes_tr_proto=HAVE_$ac_tr_proto
    ac_no_tr_proto=HAVE_NO_$ac_tr_proto
  changequote([, ])dnl
  AC_CHECK_PROTO($1, [$2],
    AC_DEFINE_UNQUOTED($ac_yes_tr_proto) [$3],
    AC_DEFINE_UNQUOTED($ac_no_tr_proto) [$4])])

dnl Easy macro for the common but messy case we are interested in a prototype
dnl from <string.h>
AC_DEFUN(AC_CHECK_NO_STRING_PROTO,
  [ AC_CHECK_NO_PROTO($1, [#if STDC_HEADERS || HAVE_STRING_H
# include <string.h>
/* An ANSI string.h and pre-ANSI memory.h might conflict.  */
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif /* not STDC_HEADERS and HAVE_MEMORY_H */
#else /* not STDC_HEADERS and not HAVE_STRING_H */
# include <strings.h>
/* memory.h and strings.h conflict on some systems.  */
#endif /* not STDC_HEADERS and not HAVE_STRING_H */], [$2], [$3])])

AC_DEFUN(AC_NEED_RESOLV,
[AC_MSG_CHECKING(whether we might need resolv)
 AC_CACHE_VAL(ac_cv_check_resolv,
  AC_CHECK_LIB(resolv, main, ac_cv_check_resolv=yes, ac_cv_check_resolv=no))dnl
 if eval "test \"`echo '$ac_cv_check_resolv'`\" = yes"; then
   AC_MSG_RESULT(yes)
   RESOLVLIB="-lresolv"
 else
   AC_MSG_RESULT(no)
 fi
 AC_SUBST(RESOLVLIB)])dnl

ifelse([
dnl the following is nonsense. It checks more of the way the file /etc/hosts
dnl is used then whether -lresolv is needed.
dnl Does anybody know a working way to determine if a system needs -lresolv
dnl to do DNS name resolution
dnl Does unix have a (portable) way to get hold of the DNS domain name ?
dnl (not all systems have getdomainname(2))
AC_DEFUN(AC_NEED_RESOLV,
[AC_MSG_CHECKING(whether we might need resolv)
 AC_CACHE_VAL(ac_cv_check_resolv,
 [AC_TRY_RUN([#ifndef HAVE_GETHOSTNAME
# include <sys/utsname.h>
#endif /* HAVE_GETHOSTNAME */

#include <stdio.h>

#if STDC_HEADERS || HAVE_STRING_H
# include <string.h>
/* An ANSI string.h and pre-ANSI memory.h might conflict.  */
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif /* not STDC_HEADERS and HAVE_MEMORY_H */
# define index strchr
#else /* not STDC_HEADERS and not HAVE_STRING_H */
# include <strings.h>
/* memory.h and strings.h conflict on some systems.  */
#endif /* not STDC_HEADERS and not HAVE_STRING_H */

#include <netdb.h>

size_t myGetHostname(char *buf, size_t maxlen)
{
    size_t len;

#ifndef HAVE_GETHOSTNAME
    struct utsname name;

    uname(&name);
    len = strlen(name.nodename);
    if (len >= maxlen) len = maxlen - 1;
    strncpy(buf, name.nodename, len);
    buf[len] = '\0';
#else /* HAVE_GETHOSTNAME */
    buf[0] = '\0';
    (void) gethostname(buf, maxlen);
    buf[maxlen - 1] = '\0';
    len = strlen(buf);
#endif /* HAVE_GETHOSTNAME */
    return len;
}

int main()
{
    char            Buffer[80];
    const char    **hosts, *host;
    struct hostent *hp;
    size_t          Length;

    myGetHostname(Buffer, sizeof(Buffer));
    hp = gethostbyname(Buffer);
    if (hp) {
        Length = strlen(hp->h_name);
        if (!strchr(hp->h_name, '.')) {
            for (hosts = (const char **) hp->h_aliases;
                 (host = *hosts) != NULL; hosts++)
                if (strncmp(host, hp->h_name, Length) == 0 &&
                    host[Length] == '.') {
                    Length = strlen(host);
                    goto found;
                }
        }
        host = hp->h_name;
      found:
        Length++;
        if (Length > sizeof(Buffer)) Length = sizeof(Buffer);
        memcpy(Buffer, host, Length);
    }
    exit(index(Buffer, '.') != 0);
}],
  eval "ac_cv_check_resolv=yes",
  eval "ac_cv_check_resolv=no",
  eval "ac_cv_check_resolv=yes")dnl
 ])dnl
 if eval "test \"`echo '$ac_cv_check_resolv'`\" = yes"; then
   AC_MSG_RESULT(yes)
   AC_CHECK_LIB(resolv, main, RESOLVLIB="-lresolv",)
 else
   AC_MSG_RESULT(no)
 fi
 AC_SUBST(RESOLVLIB)])dnl
])

dnl Check for h_errlist and h_nerr. If not available, we'll have to supply our 
dnl own. Rather easy since the contents seem to be very portable (in fact, they
dnl are so portable that one wonders if this test is useful)
AC_DEFUN(AC_H_ERRLIST,
[AC_MSG_CHECKING([for h_errlist and h_nerr])
AC_CACHE_VAL(ac_cv_var_h_errlist,
[AC_TRY_LINK([#include <netdb.h>
extern char *h_errlist[];
extern int   h_nerr;
int a;], [a=h_nerr+h_errlist[0][0];],
  ac_cv_var_h_errlist=yes, ac_cv_var_h_errlist=no)])dnl
AC_MSG_RESULT($ac_cv_var_h_errlist)
if test "$ac_cv_var_h_errlist" = yes; then
  AC_DEFINE(HAVE_H_ERRLIST)dnl
else
  AC_DEFINE(HAVE_NO_H_ERRLIST)dnl
fi])

dnl Check for X windows should also extract the FUNCPROTO variable from imake.
dnl Just wing it instead.
AC_DEFUN(AC_FUNCPROTO,
[ if test "${FUNCPROTO+set}" != set; then
    if test "${ac_cv_header_stdarg_h+set}" != set; then
      AC_CHECK_NO_HEADERS(stdarg.h)
    fi
    if test "$ac_cv_header_stdarg_h" = "yes"; then
      FUNCPROTO="-DFUNCPROTO=15"
    else
      FUNCPROTO="-DFUNCPROTO=13"
    fi
  fi
  AC_SUBST(FUNCPROTO)])

dnl Check for Xaw or Xaw3d
AC_DEFUN(AC_CHECK_LIB_XAW,
  [AC_ARG_WITH(xaw3d,
  [  --with-xaw3d            Use the 3d version of the athena widgets],
  [XAW3d="$withval"])
  if test "$XAW3d" = "yes" ; then
     AC_DEFINE(XAW3D)dnl
     XAW_LIB="-lXaw3d"
     XAW_NAME="Xaw 3d"
  else
     AC_DEFINE(NO_XAW3D)dnl
     XAW_LIB="-lXaw"
     XAW_NAME="Xaw"
  fi
  AC_SUBST(XAW_LIB)dnl
  
  dnl look for Xaw
    AC_MSG_CHECKING(for $XAW_NAME)
    AC_CACHE_VAL(ac_cv_lib_xaw,
    [USE_LIBS="$XAW_LIB -lXmu -lXt -lXext $X_PRE_LIBS -lX11"
     AC_TRY_LINK([#include <X11/Intrinsic.h>
#include <X11/Xaw/Toggle.h>],[(void) XawToggleUnsetCurrent(NULL);],
                 eval "ac_cv_lib_xaw=yes", eval "ac_cv_lib_xaw=no")dnl
     USE_LIBS=""])dnl
     if eval "test \"`echo '$ac_cv_lib_xaw'`\" = yes"; then
        AC_MSG_RESULT(yes)
     else
        AC_MSG_RESULT(no)
        AC_MSG_ERROR([I could not find the $XAW_NAME library, which the package NEEDS.
If you can find it nowhere on your system, try to ftp xaw (or better yet,
xaw 3d) from ftp.x.org or one of its mirrors])
   fi])
  
dnl look for Xpm
AC_DEFUN(AC_CHECK_LIB_XPM,
[ AC_ARG_WITH(xpm,
  [  --with-xpm              Use the \`xpm' package (color pixmaps)],
  [ ac_cv_lib_xpm="$withval"],
  [ AC_MSG_CHECKING(for Xpm)
    AC_CACHE_VAL(ac_cv_lib_xpm,
    [USE_LIBS="-lXpm $X_PRE_LIBS -lX11"
    AC_TRY_LINK([#include <X11/xpm.h>],[(void) XpmAttributesSize();],
                ac_cv_lib_xpm=yes, ac_cv_lib_xpm=no)dnl
    USE_LIBS=""])dnl
    AC_MSG_RESULT($ac_cv_lib_xpm)])
  if test "$ac_cv_lib_xpm" = yes; then
    AC_DEFINE(HAVE_XPM)dnl
    X_PRE_LIBS="-lXpm $X_PRE_LIBS"
  else
    AC_DEFINE(HAVE_NO_XPM)dnl
  fi])

dnl look for term
AC_DEFUN(AC_CHECK_LIB_TERM,
[ AC_ARG_WITH(term,
  [  --with-term             Use the \`term' package directly from the source],
  [ ac_cv_lib_term="$withval"],
  [ AC_MSG_CHECKING(for term)
    AC_CACHE_VAL(ac_cv_lib_term,
      [USE_LIBS="$TERM_DIR""client.o"
      AC_TRY_LINK([#include <client.h>],[send_command(sock, C_DUMB, 1, 0);],
                  ac_cv_lib_term=yes, ac_cv_lib_term=no)dnl
      USE_LIBS=""])dnl
    AC_MSG_RESULT($ac_cv_lib_term)])
  if test "$ac_cv_lib_term" = yes; then
     AC_DEFINE(HAVE_TERM)dnl
     TERM_LIBS="$TERM_LIBS $TERM_DIR""client.o"
  else
     AC_DEFINE(HAVE_NO_TERM)dnl
  fi
  AC_SUBST(TERM_LIBS)dnl
  AC_SUBST(TERM_DIR)])

dnl look for termnet
AC_DEFUN(AC_CHECK_LIB_TERMNET,
[ AC_ARG_WITH(termnet,
  [  --with-termnet          Use the \`term' package in installed form],
  [ ac_cv_lib_termnet="$withval"],
  [ AC_MSG_CHECKING(for termnet)
    AC_CACHE_VAL(ac_cv_lib_termnet,
      [USE_LIBS="-ltermnet"
      AC_TRY_LINK([#include <termnet.h>],[(void) term_strerror(0);],
                  ac_cv_lib_termnet=yes, ac_cv_lib_termnet=no)dnl
      USE_LIBS=""])dnl
    AC_MSG_RESULT($ac_cv_lib_termnet)])
  if test "$ac_cv_lib_termnet" = yes; then
   AC_DEFINE(HAVE_TERMNET)dnl
   TERM_LIBS="-ltermnet"
  else
    AC_DEFINE(HAVE_NO_TERMNET)dnl
  fi
  AC_SUBST(TERM_LIBS)])

dnl look for socks (from ftp.nec.com, /pub/security/socks.cstc)
AC_DEFUN(AC_CHECK_LIB_SOCKS,
[ AC_ARG_WITH(socks,
  [  --with-socks            Use the \`socks' package (use firewall proxy)],
  [ ac_cv_lib_socks="$withval"],
  [ AC_MSG_CHECKING(for socks)
    AC_CACHE_VAL(ac_cv_lib_socks,
      [USE_LIBS="-lsocks"
      AC_TRY_LINK([],[(void) Rconnect(0, 0, 0);],
                  ac_cv_lib_socks=yes, ac_cv_lib_socks=no)dnl
      USE_LIBS=""])dnl
    AC_MSG_RESULT($ac_cv_lib_socks)])
  if test "$ac_cv_lib_socks" = yes; then
    AC_DEFINE(HAVE_SOCKS)dnl
    X_EXTRA_LIBS="$X_EXTRA_LIBS -lsocks"
  else
    AC_DEFINE(HAVE_NO_SOCKS)dnl
  fi])

dnl Check if make can handle implicit library rules
AC_DEFUN(AC_PROG_MAKE_LIBRARY,
[AC_PROG_MAKE_VARS(ARFLAGS RM)
eval MAKE_ARFLAGS="\"\$ac_cv_prog_make_${ac_make}_var_ARFLAGS\""
# Or maybe we should set -rv. Is there any system were this matters ?
if test "$MAKE_ARFLAGS" = ""; then MAKE_ARFLAGS="rv"; fi
AC_SUBST(MAKE_ARFLAGS)dnl
eval MAKE_RM="\"\$ac_cv_prog_make_${ac_make}_var_RM\""
if test "$MAKE_RM" = "";      then MAKE_RM="rm -f"; fi
if test "$MAKE_RM" = "rm";    then MAKE_RM="rm -f"; fi
AC_SUBST(MAKE_RM)dnl

AC_REQUIRE([AC_PROG_RANLIB])
AC_REQUIRE([AC_PROG_CC])

AC_MSG_CHECKING(whether make can handle implicit library rules)
ac_sensitive=no
AC_CACHE_VAL(ac_cv_prog_make_${ac_make}_library,
[cat > conftestmake <<EOF
FLIB   = conftest.a
RM     = $MAKE_RM
ARFLAGS= $MAKE_ARFLAGS
CC     = $CC
CFLAGS = $CFLAGS
RANLIB = $RANLIB

all: \$(FLIB)

\$(FLIB): \$(FLIB)(conftest.o)
	\$(RANLIB) \$(FLIB)
	touch \$(FLIB)

# In case make does not have these rules built in
.c.o:
	\$(CC) -c \$(CPPFLAGS) \$(CFLAGS) \$<

.c.a:
	\$(CC) -c \$(CPPFLAGS) \$(CFLAGS) \$<
	\$(AR) \$(ARFLAGS) \$(FLIB) \$%
	\$(RM) $%
	@echo "ac_done_compile=yes"
EOF
cat > conftest.$ac_ext <<\EOF
#include "confdefs.h"

int main()
{
    return 0;
}
EOF
# GNU make sometimes prints "make[1]: Entering...", which would confuse us.
ac_done_compile=no
# Ultrix and Pyramid sh refuse to redirect output of eval, so use subshell.
eval `${MAKE-make} -f conftestmake 2>/dev/null | grep done_compile`
if test "$ac_done_compile" = yes; then
  eval ac_cv_prog_make_${ac_make}_library=yes

  ac_done_compile=no
  # Ultrix and Pyramid sh refuse to redirect output of eval, so use subshell.
  eval `${MAKE-make} -f conftestmake 2>/dev/null | grep done_compile`
  if test "$ac_done_compile" = no; then
    ac_sensitive=yes
  fi
else
  eval ac_cv_prog_make_${ac_make}_library=no
fi
rm -f conftestmake conftest.a conftest.$ac_ext])dnl
eval ac_make_temp=\$ac_cv_prog_make_${ac_make}_library
AC_MSG_RESULT($ac_make_temp)

dnl At least one system (AIX ESA could handle implicit library rules, but did
dnl not seem te realise when the contents were changed, and always remade
dnl everything. Makes the rule pretty useless.
AC_MSG_CHECKING(whether make implicit library rules are time sensitive)
AC_CACHE_VAL(ac_cv_prog_make_${ac_make}_sensitive,
[eval ac_cv_prog_make_${ac_make}_sensitive=$ac_sensitive])dnl
eval ac_make_temp=\$ac_cv_prog_make_${ac_make}_sensitive
AC_MSG_RESULT($ac_make_temp)
if test $ac_make_temp = yes; then
  USE_AR=implicit
else
  USE_AR=explicit
fi
AC_SUBST(USE_AR)])

dnl Checking how yyoverflow is defined by yacc/bison
dnl AC_PROG_YACC_OVERFLOW([ACTION-IF-6-ARGS [,ACTION-IF-NOT-6-ARGS [,FILES]]])
AC_DEFUN(AC_PROG_YACC_OVERFLOW,
[AC_MSG_CHECKING(whether \`$YACC' yyoverflow uses 6 arguments)
AC_CACHE_VAL(ac_cv_prog_yacc_overflow_6,
[AC_REQUIRE([AC_PROG_YACC])
cat > conftest.y <<\EOF
%{
#define yyoverflow(x1, x2, x3, x4, x5, x6) ac_flow(x1, x2, x3, x4, x5, x6)
%}
%%
all:
%%
EOF
$YACC conftest.y
eval "$ac_cpp y.tab.c >/dev/null 2>conftest.out"
ac_err=`grep -v '^ *+' conftest.out`
if test -z "$ac_err"; then
  ac_cv_prog_yacc_overflow_6=yes
else
  echo "$ac_err" >&5
  ac_cv_prog_yacc_overflow_6=no
fi
rm -f y.tab.c conftest.y conftest.out
ifelse($3, , :, for p in $3; do touch $p; done)])dnl
AC_MSG_RESULT($ac_cv_prog_yacc_overflow_6)
if test "$ac_cv_prog_yacc_overflow_6" = yes; then
  ifelse([$1], , :, [$1])
else
  ifelse([$2], , :, [$2])
fi])

dnl Get the Xlib version number
AC_DEFUN(AC_X_VERSION,
  [AC_MSG_CHECKING(for X version)
  AC_CACHE_VAL(ac_cv_x_version,
    [AC_REQUIRE([AC_PATH_XTRA])
    ac_cv_x_version=""
    ac_cv_header_x11_xlib_h=no
    if test "$no_x" != "yes"; then
      cat > conftest.$ac_ext <<\EOF
#include "confdefs.h"
#include <X11/Xlib.h>

#ifdef XlibSpecificationRelease
ac_cv_x_version=XlibSpecificationRelease
#endif
EOF
      ac_x_version_temp=`eval "$ac_cpp conftest.$ac_ext 2>conftest.out | grep ac_cv_x_version | sed 's/ //g'"`
      ac_err=`grep -v '^ *+' conftest.out`
      if test -z "$ac_err"; then
        ac_cv_header_x11_xlib_h=yes
        eval "$ac_x_version_temp"
      else
        ac_cv_header_x11_xlib_h=no
        echo "$ac_err" >&5
      fi
      rm -f conftest*
    fi])dnl
  AC_MSG_RESULT($ac_cv_x_version)
  X_VERSION=$ac_cv_x_version
  AC_SUBST(X_VERSION)])

dnl Get the Xt version number
AC_DEFUN(AC_XT_VERSION,
  [AC_MSG_CHECKING(for Xt version)
  AC_CACHE_VAL(ac_cv_xt_version,
    [AC_REQUIRE([AC_PATH_XTRA])
    ac_cv_xt_version=""
    ac_cv_header_x11_intrinsic_h=no
    if test "$no_x" != "yes"; then
      cat > conftest.$ac_ext <<\EOF
#include "confdefs.h"
#include <X11/Intrinsic.h>

#ifdef XtSpecificationRelease
ac_cv_xt_version=XtSpecificationRelease
#endif
EOF
      ac_xt_version_temp=`eval "$ac_cpp conftest.$ac_ext 2>conftest.out | grep ac_cv_xt_version | sed 's/ //g'"`
      ac_err=`grep -v '^ *+' conftest.out`
      if test -z "$ac_err"; then
        ac_cv_header_x11_intrinsic_h=yes
        eval "$ac_xt_version_temp"
      else
        ac_cv_header_x11_intrinsic_h=no
        echo "$ac_err" >&5
      fi
      rm -f conftest*
    fi])dnl
  AC_MSG_RESULT($ac_cv_xt_version)
  XT_VERSION=$ac_cv_xt_version
  AC_SUBST(XT_VERSION)])

dnl Check to what make sets the given variables
AC_DEFUN(AC_PROG_MAKE_VARS,
  [ac_make_vars=""
  for ac_make_var in $1; do
    if eval "test \"`echo '$''{'ac_cv_prog_make_${ac_make}_var_${ac_make_var}'+set}'`\" = set"; then
      AC_MSG_CHECKING(to what make sets \$($ac_make_var))
      eval ac_make_temp="\"(cached) \$ac_cv_prog_make_${ac_make}_var_${ac_make_var}\""
      AC_MSG_RESULT($ac_make_temp)
    else
      ac_make_vars="$ac_make_vars $ac_make_var"
    fi
  done
  if test "$ac_make_vars" != ""; then
    cat > conftestmake <<\EOF
all:
EOF
    for ac_make_var in $ac_make_vars; do
      echo "	@echo \"ac_cv_prog_make_${ac_make}_var_${ac_make_var}='$""(${ac_make_var})'\"" >> conftestmake
    done
    eval `${MAKE-make} -f conftestmake 2>/dev/null | grep "^ac_cv"`
    rm -rf conftestmake
    for ac_make_var in $ac_make_vars; do
      AC_MSG_CHECKING(to what make sets \$($ac_make_var))
      eval ac_make_temp="\"\$ac_cv_prog_make_${ac_make}_var_${ac_make_var}\""
      AC_MSG_RESULT($ac_make_temp)
    done
  fi])

dnl Check for networking libraries in absence of X
AC_DEFUN(AC_PATH_XTRA_NO_X,
  [AC_REQUIRE([AC_ISC_POSIX])
  if test "$ISC" = yes; then
    X_EXTRA_LIBS="$X_EXTRA_LIBS -lnsl_s -linet"
  else
    # Martyn.Johnson@cl.cam.ac.uk says this is needed for Ultrix, if the X
    # libraries were built with DECnet support.  And karl@cs.umb.edu says
    # the Alpha needs dnet_stub (dnet does not exist).
    AC_CHECK_LIB(dnet, dnet_ntoa, [X_EXTRA_LIBS="$X_EXTRA_LIBS -ldnet"])
    if test $ac_cv_lib_dnet = no; then
      AC_CHECK_LIB(dnet_stub, dnet_ntoa,
        [X_EXTRA_LIBS="$X_EXTRA_LIBS -ldnet_stub"])
    fi

    # msh@cis.ufl.edu says -lnsl (and -lsocket) are needed for his 386/AT,
    # to get the SysV transport functions.
    # Not sure which flavor of 386 UNIX this is, but it seems harmless to
    # check for it.
    AC_CHECK_LIB(nsl, t_accept, [X_EXTRA_LIBS="$X_EXTRA_LIBS -lnsl"])

    # lieder@skyler.mavd.honeywell.com says without -lsocket,
    # socket/setsockopt and other routines are undefined under SCO ODT 2.0.
    # But -lsocket is broken on IRIX, according to simon@lia.di.epfl.ch.
    if test "`(uname) 2>/dev/null`" != IRIX; then
      AC_CHECK_LIB(socket, socket, [X_EXTRA_LIBS="$X_EXTRA_LIBS -lsocket"])
    fi
  fi
  AC_SUBST(X_EXTRA_LIBS)])
