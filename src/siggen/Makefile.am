AUTOMAKE_OPTIONS = foreign no-dependencies
AM_INSTALL_PROGRAM_FLAGS = -m 755

AM_CPPFLAGS = -I$(srcdir)/..
LIBS = -ltripwire -lcryptlib @LIBS@
LDFLAGS = @LDFLAGS@ -L../../lib
LN_S = @LN@

sbin_PROGRAMS = siggen
siggendir=.
siggen_SOURCES = \
  siggen.cpp siggencmdline.cpp siggenmain.cpp \
  siggenstrings.cpp stdsiggen.cpp

siggen_HEADERS = \
  siggen.h siggencmdline.h siggenstrings.h stdsiggen.h

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = ../../bin/siggen *.gcno *.gcda

all: $(sbin_PROGRAMS)
	@test -d ../../bin && $(LN) -f $(sbin_PROGRAMS) ../../bin
