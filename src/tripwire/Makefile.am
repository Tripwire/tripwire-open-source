AUTOMAKE_OPTIONS = foreign no-dependencies
AM_INSTALL_PROGRAM_FLAGS = -m 755

INCLUDES = -I..
LIBS = -ltripwire -lcryptlib @LIBS@
LDFLAGS = @LDFLAGS@ -L../../lib

sbin_PROGRAMS = tripwire
tripwire_SOURCES =	\
  generatedb.cpp integritycheck.cpp mailmessage.cpp pipedmailmessage.cpp \
  policyupdate.cpp smtpmailmessage.cpp stdtripwire.cpp syslog_trip.cpp	 \
  tripwire.cpp tripwireerrors.cpp tripwiremain.cpp tripwirestrings.cpp	 \
  tripwireutil.cpp twcmdline.cpp twcmdlineutil.cpp updatedb.cpp

DEFS = @DEFS@		# This gets rid of the -I. so INCLUDES must be more explicit
CLEANFILES = ../../bin/tripwire ../../lib/libtripwire.a

all: $(sbin_PROGRAMS)
	@test -d ../../bin && ln -f $(sbin_PROGRAMS) ../../bin
