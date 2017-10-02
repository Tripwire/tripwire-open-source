AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I$(srcdir)/..

noinst_LIBRARIES = libutil.a
libutil_adir = .
libutil_a_SOURCES = \
   fileutil.cpp stdutil.cpp stringencoder.cpp \
   util.cpp utilerrors.cpp utilstrings.cpp

libutil_a_HEADERS = fileutil.h miscutil.h stdutil.h \
   stringencoder.h util.h utilerrors.h utilstrings.h

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = *.gcno *.gcda

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libutil_a_OBJECTS)
	$(RANLIB) ../../lib/libtripwire.a
