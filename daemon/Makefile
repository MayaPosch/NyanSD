# Makefile for the NyanSD daemon server.
#
# 2020/04/27, Maya Posch

GCC = g++
MAKEDIR = mkdir -p
RM = rm
STRIP = strip

OUTPUT = nyansdd
INCLUDE = -I $(TOP)/src
LIB := -lnyansd -lPocoNet -lPocoUtil -lPocoFoundation -lstdc++fs
CFLAGS := $(INCLUDE) -std=c++17 -g3

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__ -DPOCO_WIN32_UTF8
	EXTENSION := .exe
	#LIB += -lws2_32
else
	LIB += -pthread
endif

SOURCES := $(wildcard *.cpp)
OBJECTS := $(addprefix obj/,$(notdir) $(SOURCES:.cpp=.o))

all: makedir bin/$(OUTPUT)

makedir:
	$(MAKEDIR) obj
	$(MAKEDIR) bin

obj/%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)
	
bin/$(OUTPUT): $(OBJECTS)
	$(GCC) -o $@ $(OBJECTS) $(LIB)
	cp $@ $@.debug
	$(STRIP) -S --strip-unneeded $@$(EXTENSION)

clean:
	$(RM) $(OBJECTS)

# --- INSTALLATION ---

PREFIX ?= /usr/local
	
# ifeq ($(PREFIX),/usr/local)
	# CONFDIR := $(PREFIX)/etc
# else
	# CONFDIR := /etc
# endif

CONFDIR := /etc
	
.PHONY: install
install:
	install -d $(DESTDIR)$(PREFIX)/bin/ \
		-d $(DESTDIR)$(CONFDIR)/nyansd/services
	install -m 755 bin/$(OUTPUT) $(DESTDIR)$(PREFIX)/bin/

.PHONY: install-systemd
install-systemd:
	install -d $(DESTDIR)$(CONFDIR)/systemd/system/ 
	sed ${SED_REPLACE} systemd/nyansd.service > $(DESTDIR)$(CONFDIR)/systemd/system/nyansd.service

.PHONY: install-openrc
install-openrc:
	install -d $(DESTDIR)$(CONFDIR)/init.d/	
	sed ${SED_REPLACE} openrc/nyansd > $(DESTDIR)$(CONFDIR)/init.d/nyansd
	