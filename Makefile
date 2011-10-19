CC:=cc
INSTALL:=install

PREFIX:=/usr
GROUP:=svusers
SVDIR:=.sv

CFLAGS:=-ggdb -O2

.PHONY: clean install

usersv: src/usersv.c
	${CC} -o usersv -DGROUP='"${GROUP}"' -DSVDIR='"${SVDIR}"' ${LDFLAGS} ${CFLAGS} src/usersv.c

clean:
	-rm usersv

install: usersv
	${INSTALL} -g root -o root --strip usersv ${PREFIX}/sbin
