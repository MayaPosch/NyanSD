# Makefile for the NyanSD project.

# 2020/04/27, Maya Posch

export TOP := $(CURDIR)

ifdef ANDROID
TOOLCHAIN_PREFIX := arm-linux-androideabi-
ifdef OS
TOOLCHAIN_POSTFIX := .cmd
endif
GCC = $(TOOLCHAIN_PREFIX)g++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else
GCC = g++
MAKEDIR = mkdir -p
RM = rm
AR = ar
endif

OUTPUT = libnyansd
VERSION = 0.1
INCLUDE = -I src
LIBS := -lPocoNet -lPocoUtil -lPocoFoundation
CFLAGS := $(INCLUDE) -g3 -std=c++11 -O0
SHARED_FLAGS := -fPIC -shared -Wl,-soname,$(OUTPUT).so.$(VERSION)

ifdef ANDROID
CFLAGS += -fPIC
endif

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
ifndef ANDROID
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__ -DPOCO_WIN32_UTF8
	LIBS += -lws2_32
endif
else
	LIBS += -pthread
endif

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(addprefix obj/static/,$(notdir) $(SOURCES:.cpp=.o))
SHARED_OBJECTS := $(addprefix obj/shared/,$(notdir) $(SOURCES:.cpp=.o))

all: lib apps

lib: makedir lib/$(OUTPUT).a lib/$(OUTPUT).so.$(VERSION)
	
obj/static/%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)
	
obj/shared/%.o: %.cpp
	$(GCC) -c -o $@ $< $(SHARED_FLAGS) $(CFLAGS) $(LIBS)
	
lib/$(OUTPUT).a: $(OBJECTS)
	-rm -f $@
	$(AR) rcs $@ $^
	
lib/$(OUTPUT).so.$(VERSION): $(SHARED_OBJECTS)
	$(GCC) -o $@ $(CFLAGS) $(SHARED_FLAGS) $(SHARED_OBJECTS) $(LIBS)
	
makedir:
	$(MAKEDIR) lib
	$(MAKEDIR) obj/static/src
	$(MAKEDIR) obj/shared/src
	
apps: client server
	
client:
	$(MAKE) -C ./client
	
server:
	$(MAKE) -C ./server

clean: clean-lib clean-client clean-server

clean-lib:
	$(RM) $(OBJECTS) $(SHARED_OBJECTS)
	
clean-client:
	$(MAKE) -C ./client clean
	
clean-server:
	$(MAKE) -C ./server clean

PREFIX ?= /usr

.PHONY: install client server
install:
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 lib/$(OUTPUT).a $(DESTDIR)$(PREFIX)/lib/
	install -m 644 lib/$(OUTPUT).so.$(VERSION) $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/
	install -m 644 src/*.h $(DESTDIR)$(PREFIX)/include/
	cd $(DESTDIR)$(PREFIX)/lib && \
		if [ -f $(OUTPUT).so ]; then \
			rm $(OUTPUT).so; \
		fi && \
		ln -s $(OUTPUT).so.$(VERSION) $(OUTPUT).so


