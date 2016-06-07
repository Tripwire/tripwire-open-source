AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I..

noinst_LIBRARIES = libdb.a
libdb_a_SOURCES = \
   blockfile.cpp blockrecordarray.cpp blockrecordfile.cpp 	\
   db.cpp hierdatabase.cpp hierdbpath.cpp stddb.cpp
 
DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libdb_a_OBJECTS)
