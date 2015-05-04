AUTOMAKE_OPTIONS = foreign no-dependencies
AM_INSTALL_PROGRAM_FLAGS = -m 755

INCLUDES = -I..
LIBS = -ltripwire -lcryptlib @LIBS@
LDFLAGS = @LDFLAGS@ -L../../lib

sbin_PROGRAMS = siggen
siggen_SOURCES = \
  siggen.cpp siggencmdline.cpp siggenmain.cpp \
  siggenstrings.cpp stdsiggen.cpp

DEFS = @DEFS@		# This gets rid of the -I. so INCLUDES must be more explicit
CLEANFILES = ../../bin/siggen

all: $(sbin_PROGRAMS)
	@test -d ../../bin && ln -f $(sbin_PROGRAMS) ../../bin
