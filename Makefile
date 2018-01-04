# sftagger - simple file tagging tool

NAME = sftagger
SRC = sftagger.c
CC = cc
VERSION = 2.0-a04
PREFIX = /usr/local
CFLAGS = -std=c99 -pedantic -Wall -Os

${NAME}: ${NAME}.c
	@${CC} -o ${NAME} ${CFLAGS} ${SRC}

clean:
	@echo cleaning
	@rm -f ${NAME} ${NAME}-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p ${NAME}-${VERSION}
	@cp -R LICENSE Makefile README ${NAME}.c ${NAME}-${VERSION}
	@tar -cf - "${NAME}-${VERSION}" | \
		gzip -c > "${NAME}-${VERSION}.tar.gz"
	@rm -rf "${NAME}-${VERSION}"

install: ${NAME}
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin/${NAME}
	@cp -f sftagger ${DESTDIR}${PREFIX}/bin/.
	@chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin/${NAME}
	@rm -f ${DESTDIR}${PREFIX}/bin/${NAME}

.PHONY: clean dist install uninstall

