AUTOMAKE_OPTIONS = foreign no-dependencies
AM_INSTALL_PROGRAM_FLAGS = -m 755

INCLUDES = -I..
LIBS = -ltripwire -lcryptlib @LIBS@
LDFLAGS = @LDFLAGS@ -L../../lib

sbin_PROGRAMS = twprint
twprint_SOURCES = \
  stdtwprint.cpp twprint.cpp twprintcmdline.cpp twprinterrors.cpp \
  twprintmain.cpp twprintstrings.cpp

DEFS = @DEFS@		# This gets rid of the -I. so INCLUDES must be more explicit
CLEANFILES = ../../bin/twprint

all: $(sbin_PROGRAMS)
	@test -d ../../bin && ln -f $(sbin_PROGRAMS) ../../bin

