AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I$(srcdir)/..

noinst_LIBRARIES = libcore.a
libcore_adir=.
libcore_a_SOURCES = \
   file_unix.cpp unixfsservices.cpp					\
   archive.cpp charutil.cpp		\
   cmdlineparser.cpp codeconvert.cpp core.cpp coreerrors.cpp		\
   corestrings.cpp crc32.cpp debug.cpp displayencoder.cpp		\
   displayutil.cpp epoch.cpp error.cpp errorbucketimpl.cpp errortable.cpp \
   errorutil.cpp fileerror.cpp fileheader.cpp fsservices.cpp		\
   growheap.cpp hashtable.cpp haval.cpp msystem.cpp ntmbs.cpp		\
   refcountobj.cpp serializable.cpp serializer.cpp	\
   serializerimpl.cpp serializerutil.cpp serstring.cpp 			\
   srefcountobj.cpp srefcounttbl.cpp stdcore.cpp stringutil.cpp		\
   timeconvert.cpp tw_signal.cpp twlimits.cpp twlocale.cpp      	\
   unixexcept.cpp usernotify.cpp usernotifystdout.cpp		\
   wchar16.cpp

libcore_a_HEADERS = archive.h charutil.h cmdlineparser.h codeconvert.h       \
   core.h coreerrors.h corestrings.h crc32.h debug.h displayencoder.h        \
   displayutil.h epoch.h error.h errorbucket.h errorbucketimpl.h errorgeneral.h \
   errortable.h errorutil.h file.h fileerror.h fileheader.h fixedfilebuf.h   \
   fixups.h fsservices.h growheap.h hashtable.h haval.h md5.h msystem.h ntdbs.h \
   ntmbs.h package.h platform.h refcountobj.h resources.h                    \
   serializable.h serializer.h serializerimpl.h serializerutil.h serstring.h \
   sha.h srefcountobj.h srefcounttbl.h stdcore.h stringutil.h tasktimer.h    \
   tchar.h timeconvert.h tw_signal.h twlimits.h twlocale.h                   \
   twstringslang.h typed.h types.h unixexcept.h unixfsservices.h upperbound.h \
   usernotify.h usernotifystdout.h wchar16.h

libcore_a_LIBADD = @CORE_CRYPT_O@
libcore_a_DEPENDENCIES = @CORE_CRYPT_O@
 
DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = *.gcno *.gcda

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libcore_a_OBJECTS) $(libcore_a_LIBADD)
