VERSION=0.0.32

DEBUG= -g # -D_DEBUG -g -fprofile-arcs -ftest-coverage # -pg -g
CXXFLAGS+=-Wall -g -O2 -DVERSION=\"${VERSION}\" $(DEBUG)
CFLAGS+=${CXXFLAGS}
LDFLAGS+=$(DEBUG) -lncurses -lstdc++

OBJS=error.o utils.o br.o nc.o pl.o

all: nagcon

nagcon: $(OBJS)
	$(CC) -Wall -W $(OBJS) $(LDFLAGS) -o nagcon

install: nagcon
	cp nagcon /usr/local/bin

clean:
	rm -f $(OBJS) nagcon core *.da *.gcov *.bb*

package: clean
	mkdir nagcon-$(VERSION)
	cp *.c* *.h Makefile readme.txt license.txt nagcon-$(VERSION)
	tar czf nagcon-$(VERSION).tgz nagcon-$(VERSION)
	rm -rf nagcon-$(VERSION)
