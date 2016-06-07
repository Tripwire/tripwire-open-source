AUTOMAKE_OPTIONS = foreign no-dependencies
AM_INSTALL_PROGRAM_FLAGS = -m 755

AM_CPPFLAGS = -I..
LIBS = -ltripwire -lcryptlib @LIBS@
LDFLAGS = @LDFLAGS@ -L../../lib
LN_S = @LN@

sbin_PROGRAMS = tripwire
tripwire_SOURCES =	\
  generatedb.cpp integritycheck.cpp mailmessage.cpp pipedmailmessage.cpp \
  policyupdate.cpp smtpmailmessage.cpp stdtripwire.cpp syslog_trip.cpp	 \
  tripwire.cpp tripwireerrors.cpp tripwiremain.cpp tripwirestrings.cpp	 \
  tripwireutil.cpp twcmdline.cpp twcmdlineutil.cpp updatedb.cpp

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = ../../bin/tripwire ../../lib/libtripwire.a

all: $(sbin_PROGRAMS)
	@test -d ../../bin && $(LN) -f $(sbin_PROGRAMS) ../../bin
