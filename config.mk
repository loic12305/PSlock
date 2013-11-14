# PSlock version
VERSION = 1.0

# Customize below to fit your system

# paths
PREFIX = /usr

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# includes and libs
INCS = -I. -I/usr/include -I${X11INC}
#change below to build with libcrypt static
LIBS = -L/usr/lib -lc -lcrypt -L${X11LIB} -lX11 -lXext -lXpm 
#LIBS = -L/usr/lib -lc /usr/lib/libcrypt.a -L${X11LIB} -lX11 -lXext -lXpm

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" -DHAVE_SHADOW_H
CFLAGS = -Wall -Os -mtune=i386 ${INCS} ${CPPFLAGS}
LDFLAGS = -s ${LIBS}

# On *BSD remove -DHAVE_SHADOW_H from CPPFLAGS and add -DHAVE_BSD_AUTH
# On OpenBSD and Darwin remove /usr/lib/libcrypt.a from LIBS

# compiler and linker
CC = gcc

# Install mode. On BSD systems MODE=2755 and GROUP=auth
# On others MODE=4755 and GROUP=root
#MODE=2755
#GROUP=auth
