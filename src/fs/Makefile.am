AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I$(srcdir)/..

noinst_LIBRARIES = libfs.a
libfs_adir=.
libfs_a_SOURCES = \
   fs.cpp fsdatasourceiter.cpp fserrors.cpp fsfactory.cpp	\
   fsnametranslator.cpp fsobject.cpp fsparserutil.cpp		\
   fspropcalc.cpp fspropdisplayer.cpp fspropset.cpp		\
   fsstrings.cpp fsvisitor.cpp stdfs.cpp
 
libfs_a_HEADERS = \
   fs.h fsdatasourceiter.h fserrors.h fsfactory.h \
   fsnametranslator.h fsobject.h fsparserutil.h \
   fspropcalc.h fspropdisplayer.h fspropset.h fsstrings.h \
   fsvisitor.h stdfs.h

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = *.gcno *.gcda

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libfs_a_OBJECTS)
