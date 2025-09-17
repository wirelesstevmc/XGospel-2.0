#!/bin/bash

# 32-bit build script for XGospel (without xaw3d, with xpm)
CC="gcc"
CFLAGS="-m32 -O2 -DSTDC_HEADERS=1 -DHAVE_STRING_H=1 -DHAVE_STDLIB_H=1 -DHAVE_MEMORY_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DHAVE_SYS_SELECT_H=1 -DRETSIGTYPE=void -DHAVE_STRFTIME=1 -DHAVE_MEMCHR=1 -DHAVE_MEMMOVE=1 -DHAVE_STRERROR=1 -DHAVE_DIFFTIME=1 -DHAVE_UNAME=1 -DHAVE_CUSERID=1 -DHAVE_MEMCHR=1 -DHAVE_STRERROR=1 -DHAVE_H_ERRLIST=1 -DHAVE_ALLOCA_H=1 -DHAVE_ALLOCA=1 -DNO_XAW3D=1 -DHAVE_XPM=1 -DHAVE_GETHOSTNAME=1 -DHAVE_NO_TERM=1 -DHAVE_NO_TERMNET=1 -DHAVE_NO_SOCKS=1 -DFUNCPROTO=15"
INCLUDES="-I. -Imy -Iregex"
LDFLAGS="-m32"
LIBS="-L/usr/lib32 -L/usr/lib/i386-linux-gnu -lXaw -lXmu -lXt -lXext -lXpm -lX11 -lm -lresolv"

echo "Building 32-bit XGospel (without xaw3d, with xpm)..."

# Clean up old files
rm -f *.o xgospel

# Generate parser files if they don't exist
echo "Checking parser files..."
if [ ! -f gointer.c ]; then
    echo "Generating gointer.c..."
    bison -y -d gointer.y
    sed 's/YYOVERFLOW/yyoverflow(x1, x2, x3, x4, x5, x8)/g; s/yy/IgsYY/g' y.tab.c > gointer.c
    sed 's/yy/IgsYY/g' y.tab.h > gointer.tab.h
    rm -f y.tab.c y.tab.h
fi

# Use existing goserver.c (already fixed)
echo "Using existing goserver.c (pre-generated and fixed)"

echo "Compiling source files..."
$CC $CFLAGS $INCLUDES -c gointer.c || exit 1
$CC $CFLAGS $INCLUDES -c goserver.c || exit 1
$CC $CFLAGS $INCLUDES -c xgospel.c || exit 1
$CC $CFLAGS $INCLUDES -c gospel.c || exit 1
$CC $CFLAGS $INCLUDES -c resources.c || exit 1
$CC $CFLAGS $INCLUDES -c GoBoard.c || exit 1
$CC $CFLAGS $INCLUDES -c connect.c || exit 1
$CC $CFLAGS $INCLUDES -c observe.c || exit 1
$CC $CFLAGS $INCLUDES -c analyze.c || exit 1
$CC $CFLAGS $INCLUDES -c stats.c || exit 1
$CC $CFLAGS $INCLUDES -c reviews.c || exit 1
$CC $CFLAGS $INCLUDES -c games.c || exit 1
$CC $CFLAGS $INCLUDES -c players.c || exit 1
$CC $CFLAGS $INCLUDES -c broadcast.c || exit 1
$CC $CFLAGS $INCLUDES -c tell.c || exit 1
$CC $CFLAGS $INCLUDES -c messages.c || exit 1
$CC $CFLAGS $INCLUDES -c match.c || exit 1
# Skip events.c for now due to function pointer type issues
# $CC $CFLAGS $INCLUDES -c events.c || exit 1
$CC $CFLAGS $INCLUDES -c event_stubs.c || exit 1
$CC $CFLAGS $INCLUDES -c utils.c || exit 1
$CC $CFLAGS $INCLUDES -c SmeBell.c || exit 1
$CC $CFLAGS $INCLUDES -c igs_protocol_adapter.c || exit 1

echo "Building support libraries..."
# Build my library
cd my
echo "Building libmy.a..."
for src in except.c mymalloc.c actions.c showwidgets.c myxlib.c myxinternals.c mytext.c TMyprint.c mycontext.c myconverters.c myclass.c YShell.c Canvas.c Tree.c SmeToggle.c SmeLabel.c Popup.c TearofMenu.c lwidgettree.c ywidgettree.c lreslang.c yreslang.c; do
    if [ -f "$src" ]; then
        echo "Compiling $src..."
        gcc $CFLAGS $INCLUDES -c "$src" || exit 1
    fi
done
ar rv libmy.a *.o
cd ..

# Build regex library  
cd regex
echo "Building libregex.a..."
for src in regex.c; do
    if [ -f "$src" ]; then
        echo "Compiling $src..."
        gcc $CFLAGS $INCLUDES -c "$src" || exit 1
    fi
done
ar rv libregex.a *.o
cd ..

echo "Linking..."
$CC $LDFLAGS -o xgospel *.o -Lmy -Lregex -lmy -lregex $LIBS

if [ -f xgospel ]; then
    echo "Build successful! 32-bit binary created: xgospel"
    ls -la xgospel
    file xgospel
    echo ""
    echo "Key configuration:"
    echo "- 32-bit compilation (-m32)"
    echo "- No XAW3D support (-DNO_XAW3D=1)"
    echo "- XPM support enabled (-DHAVE_XPM=1)"
    echo "- Using standard Xaw libraries"
else
    echo "Build failed!"
    exit 1
fi