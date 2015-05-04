AUTOMAKE_OPTIONS = foreign no-dependencies

INCLUDES = -I..

noinst_LIBRARIES = libutil.a
libutil_a_SOURCES = \
   fileutil.cpp stdutil.cpp stringencoder.cpp \
   util.cpp utilerrors.cpp utilstrings.cpp

DEFS = @DEFS@		# This gets rid of the -I. so INCLUDES must be more explicit

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libutil_a_OBJECTS)
	$(RANLIB) ../../lib/libtripwire.a
