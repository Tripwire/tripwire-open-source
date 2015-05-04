AUTOMAKE_OPTIONS = foreign no-dependencies
AM_INSTALL_PROGRAM_FLAGS = -m 755

INCLUDES = -I..
LIBS = -ltripwire -lcryptlib @LIBS@
LDFLAGS = @LDFLAGS@ -L../../lib

sbin_PROGRAMS = twadmin
twadmin_SOURCES = \
  keygeneration.cpp stdtwadmin.cpp twadmin.cpp twadmincl.cpp \
  twadminerrors.cpp twadminmain.cpp twadminstrings.cpp

DEFS = @DEFS@		# This gets rid of the -I. so INCLUDES must be more explicit
CLEANFILES = ../../bin/twadmin

all: $(sbin_PROGRAMS)
	@test -d ../../bin && ln -f $(sbin_PROGRAMS) ../../bin
