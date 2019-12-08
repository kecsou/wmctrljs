CC=gcc
AM_CFLAGS = -Wall -Werror
INCLUDE_CFLAGS=-I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LDADD = -lglib-2.0 -lX11 -lXmu -lSM -lICE

SRC=$(wildcard src/*.c)
TEST_SRC=$(wildcard test/*.c)

wmctrltest:
	$(CC) $(AM_CFLAGS) -fsanitize=address -g $(INCLUDE_CFLAGS) $(SRC) $(TEST_SRC) $(LDADD) -o wmctrlTest.out

lib:
	$(CC) $(AM_CFLAGS) -fPIC -shared $(INCLUDE_CFLAGS) $(SRC) $(LDADD) -o wmctrl.so
