CC = gcc
CFLAGS = -D _BSD_SOURCE -D _XOPEN_SOURCE -Wall -O3
LD = gcc
LDFLAGS = -shared
INSTALLDIR = /usr/local/lib
LDCONFIG = ldconfig

objects = ts.o strings.o dates.o csv.o mktime_mod.o
target = libdickinson.so.0.1

all: $(target)

$(target): $(objects)
	$(LD) $(LDFLAGS) -o $(target) $(objects) $(LIBS)

install:
	cp $(target) $(INSTALLDIR)
	$(LDCONFIG)

clean:
	rm -f *.o *.so $(targetdir)/core $(target)
