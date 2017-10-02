AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I$(srcdir)/..

noinst_LIBRARIES = libdb.a
libdb_adir=.
libdb_a_SOURCES = \
   blockfile.cpp blockrecordarray.cpp blockrecordfile.cpp 	\
   db.cpp dberrors.cpp hierdatabase.cpp hierdbpath.cpp stddb.cpp
 
libdb_a_HEADERS = \
   block.h blockfile.h blockrecordarray.h blockrecordfile.h \
   db.h dberrors.h hierdatabase.h hierdbnode.h hierdbpath.h stddb.h

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = *.gcno *.gcda

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libdb_a_OBJECTS)
