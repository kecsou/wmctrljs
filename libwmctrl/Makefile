CC=gcc
AM_CFLAGS = -Wall -Werror
#CFLAGS=-I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LIBS = -lX11 -lXmu -lSM -lICE

SOURCES=$(wildcard src/*.c)
TEST_SRC=$(wildcard test/*.c)
OBJS=$(SOURCES:.c=.o)
TARGET_LIB=libwmctrl.a

wmctrltest:$(TARGET_LIB)
	#$(CC) $(AM_CFLAGS) -fsanitize=address -g -L. -o wmctrlTest.out $(TEST_SRC) -lwmctrl $(LIBS)
	$(CC) $(AM_CFLAGS) -fsanitize=address -g -o wmctrlTest.out $(SOURCES) $(TEST_SRC) $(LIBS)
	make clean

$(TARGET_LIB):$(OBJS)
	$(CC) $(AM_CFLAGS) -g -c $(SOURCES) $(LIBS)
	ar rcs $(TARGET_LIB) $(OBJS)

clean:
	$(RM) src/*.o
	$(RM) *.o

cleanAll: $(clean)
	$(RM) wmctrl.so
	$(RM) *.a
	$(RM) *.out