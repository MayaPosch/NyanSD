# Makefile for the NyanSD reference implementation client.
#
# 2020/04/27, Maya Posch

GCC = g++
MAKEDIR = mkdir -p
RM = rm
STRIP = strip

OUTPUT = nyansd-browse
INCLUDE = -I $(TOP)/src
LIB := -lnyansd -lPocoNet -lPocoUtil -lPocoFoundation
CFLAGS := $(INCLUDE) -std=c++11 -g3

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
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 bin/$(OUTPUT) $(DESTDIR)$(PREFIX)/bin/
	