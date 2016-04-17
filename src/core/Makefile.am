AUTOMAKE_OPTIONS = foreign no-dependencies

INCLUDES = -I..

noinst_LIBRARIES = libcore.a
libcore_a_SOURCES = \
   file_unix.cpp unixfsservices.cpp					\
   archive.cpp charutil.cpp		\
   cmdlineparser.cpp codeconvert.cpp core.cpp coreerrors.cpp		\
   corestrings.cpp crc32.cpp debug.cpp displayencoder.cpp		\
   displayutil.cpp error.cpp errorbucketimpl.cpp errortable.cpp		\
   errorutil.cpp fileerror.cpp fileheader.cpp fsservices.cpp		\
   growheap.cpp hashtable.cpp haval.cpp msystem.cpp ntmbs.cpp		\
   objectpool.cpp refcountobj.cpp serializable.cpp serializer.cpp	\
   serializerimpl.cpp serializerutil.cpp serstring.cpp			\
   srefcountobj.cpp srefcounttbl.cpp stdcore.cpp stringutil.cpp		\
   timebomb.cpp timeconvert.cpp tw_signal.cpp twlimits.cpp twlocale.cpp	\
   unixexcept.cpp usernotify.cpp usernotifystdout.cpp utf8.cpp		\
   wchar16.cpp

libcore_a_LIBADD = @CORE_CRYPT_O@
libcore_a_DEPENDENCIES = @CORE_CRYPT_O@
 
DEFS = @DEFS@		# This gets rid of the -I. so INCLUDES must be more explicit

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libcore_a_OBJECTS) $(libcore_a_LIBADD)
