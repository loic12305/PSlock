# pupslock - screen locker + screen saver
# based on slock: http://tools.suckless.org/slock
# slock - simple screen locker
# © 2006-2007 Anselm R. Garbe, Sander van Dijk
# modified by goingnuts.dk 160811

include config.mk

SRC = PSlock.c
OBJ = ${SRC:.c=.o}

all: options PSlock

options:
	@echo PSlock build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

pupslock: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f PSlock ${OBJ} PSlock-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p PSlock-${VERSION}
	@cp -R LICENSE Makefile README config.mk ${SRC} PSlock-${VERSION}
	@tar -cf PSlock-${VERSION}.tar PSlock-${VERSION}
	@gzip PSlock-${VERSION}.tar
	@rm -rf PSlock-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f PSlock ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/PSlock
	@chmod u+s ${DESTDIR}${PREFIX}/bin/PSlock

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/PSlock

.PHONY: all options clean dist install uninstall
