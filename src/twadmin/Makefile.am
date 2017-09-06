AUTOMAKE_OPTIONS = foreign no-dependencies
AM_INSTALL_PROGRAM_FLAGS = -m 755

AM_CPPFLAGS = -I$(srcdir)/..
LIBS = -ltripwire -lcryptlib @LIBS@
LDFLAGS = @LDFLAGS@ -L../../lib
LN_S = @LN@

sbin_PROGRAMS = twadmin
twadmindir=.
twadmin_SOURCES = \
  keygeneration.cpp stdtwadmin.cpp twadmin.cpp twadmincl.cpp \
  twadminerrors.cpp twadminmain.cpp twadminstrings.cpp

twadmin_HEADERS = \
  keygeneration.h resource.h stdtwadmin.h twadmin.h \
  twadmincl.h twadminerrors.h twadminstrings.h

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = ../../bin/twadmin *.gcno *.gcda

all: $(sbin_PROGRAMS)
	@test -d ../../bin && $(LN) -f $(sbin_PROGRAMS) ../../bin
