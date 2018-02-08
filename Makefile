# sftagger - simple file tagging tool

NAME = sftagger
SRC = sftagger.c
CC = cc
VERSION = 3.0a-04
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
CFLAGS = -std=c99 -pedantic -Wall -Os

${NAME}: ${NAME}.c
	@${CC} -o ${NAME} ${CFLAGS} ${SRC}

clean:
	@echo cleaning
	@rm -f ${NAME} ${NAME}-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p ${NAME}-${VERSION}
	@cp -R LICENSE Makefile README ${SRC} ${NAME}.1 ${NAME}-${VERSION}
	@tar -cf - "${NAME}-${VERSION}" | \
		gzip -c > "${NAME}-${VERSION}.tar.gz"
	@rm -rf "${NAME}-${VERSION}"

install: ${NAME}
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin/${NAME}
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f sftagger ${DESTDIR}${PREFIX}/bin/.
	@chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < ${NAME}.1 > ${DESTDIR}${MANPREFIX}/man1/${NAME}.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/${NAME}.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin/${NAME}
	@rm -f ${DESTDIR}${PREFIX}/bin/${NAME}
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/${NAME}.1

.PHONY: clean dist install uninstall

