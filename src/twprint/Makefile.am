AUTOMAKE_OPTIONS = foreign no-dependencies
AM_INSTALL_PROGRAM_FLAGS = -m 755

AM_CPPFLAGS = -I..
LIBS = -ltripwire -lcryptlib @LIBS@
LDFLAGS = @LDFLAGS@ -L../../lib
LN_S = @LN@

sbin_PROGRAMS = twprint
twprint_SOURCES = \
  stdtwprint.cpp twprint.cpp twprintcmdline.cpp twprinterrors.cpp \
  twprintmain.cpp twprintstrings.cpp

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = ../../bin/twprint

all: $(sbin_PROGRAMS)
	@test -d ../../bin && $(LN) -f $(sbin_PROGRAMS) ../../bin

