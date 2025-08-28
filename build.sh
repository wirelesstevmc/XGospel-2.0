#!/bin/bash

# 32-bit build script for XGospel
CC="gcc"
CFLAGS="-m32 -O2 -DSTDC_HEADERS=1 -DHAVE_STRING_H=1 -DHAVE_STDLIB_H=1 -DHAVE_MEMORY_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DHAVE_SYS_SELECT_H=1 -DRETSIGTYPE=void -DHAVE_STRFTIME=1 -DHAVE_MEMCHR=1 -DHAVE_MEMMOVE=1 -DHAVE_STRERROR=1 -DHAVE_DIFFTIME=1 -DHAVE_UNAME=1 -DHAVE_CUSERID=1 -DHAVE_MEMCHR=1 -DHAVE_STRERROR=1 -DHAVE_H_ERRLIST=1 -DHAVE_ALLOCA_H=1 -DHAVE_ALLOCA=1 -DNO_XAW3D=1 -DHAVE_XPM=1 -DHAVE_GETHOSTNAME=1 -DHAVE_NO_TERM=1 -DHAVE_NO_TERMNET=1 -DHAVE_NO_SOCKS=1 -DFUNCPROTO=15"
INCLUDES="-I. -Imy -Iregex"
LDFLAGS="-m32"
LIBS="-L/usr/lib -lXaw -lXmu -lXt -lXext -lXpm -lX11 -lm -lresolv"

echo "Building 32-bit XGospel..."

# Generate parser files first
echo "Generating parser files..."
cd . && bison -y -d gointer.y
sed 's/YYOVERFLOW/yyoverflow(x1, x2, x3, x4, x5, x8)/g; s/yy/IgsYY/g' y.tab.c > gointer.c
sed 's/yy/IgsYY/g' y.tab.h > gointer.tab.h
rm -f y.tab.c y.tab.h

flex -d goserver.l
sed 's/yy/IgsYY/g' lex.yy.c > goserver.c
rm -f lex.yy.c

echo "Compiling source files..."
$CC $CFLAGS $INCLUDES -c gointer.c
$CC $CFLAGS $INCLUDES -c goserver.c
$CC $CFLAGS $INCLUDES -c xgospel.c
$CC $CFLAGS $INCLUDES -c gospel.c
$CC $CFLAGS $INCLUDES -c resources.c
$CC $CFLAGS $INCLUDES -c GoBoard.c
$CC $CFLAGS $INCLUDES -c connect.c
$CC $CFLAGS $INCLUDES -c observe.c
$CC $CFLAGS $INCLUDES -c analyze.c
$CC $CFLAGS $INCLUDES -c stats.c
$CC $CFLAGS $INCLUDES -c reviews.c
$CC $CFLAGS $INCLUDES -c games.c
$CC $CFLAGS $INCLUDES -c players.c
$CC $CFLAGS $INCLUDES -c broadcast.c
$CC $CFLAGS $INCLUDES -c tell.c
$CC $CFLAGS $INCLUDES -c messages.c
$CC $CFLAGS $INCLUDES -c match.c
$CC $CFLAGS $INCLUDES -c events.c
$CC $CFLAGS $INCLUDES -c utils.c
$CC $CFLAGS $INCLUDES -c SmeBell.c
$CC $CFLAGS $INCLUDES -c modern_parser.c
$CC $CFLAGS $INCLUDES -c modern_connect.c
$CC $CFLAGS $INCLUDES -c modern_integration.c
$CC $CFLAGS $INCLUDES -c modern_xgospel_patch.c

echo "Building support libraries..."
cd my
make clean || true
cd ..
cd regex  
make clean || true
cd ..

echo "Linking..."
$CC $LDFLAGS -o xgospel *.o $LIBS

echo "Build complete!"