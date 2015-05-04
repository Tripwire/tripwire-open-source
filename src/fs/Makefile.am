AUTOMAKE_OPTIONS = foreign no-dependencies

INCLUDES = -I..

noinst_LIBRARIES = libfs.a
libfs_a_SOURCES = \
   fs.cpp fsdatasourceiter.cpp fserrors.cpp fsfactory.cpp	\
   fsnametranslator.cpp fsobject.cpp fsparserutil.cpp		\
   fspropcalc.cpp fspropdisplayer.cpp fspropset.cpp		\
   fsstrings.cpp fsvisitor.cpp stdfs.cpp
 
DEFS = @DEFS@		# This gets rid of the -I. so INCLUDES must be more explicit

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libfs_a_OBJECTS)
