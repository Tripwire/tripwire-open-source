AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I$(srcdir)/..

noinst_LIBRARIES = libtwcrypto.a
libtwcrypto_adir=.
libtwcrypto_a_SOURCES = \
   bytequeue.cpp crypto.cpp cryptoarchive.cpp keyfile.cpp \
   stdtwcrypto.cpp twcrypto.cpp twcryptoerrors.cpp

libtwcrypto_a_HEADERS = \
   bytequeue.h crypto.h cryptoarchive.h keyfile.h \
   stdtwcrypto.h twcrypto.h twcryptoerrors.h

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = *.gcno *.gcda

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libtwcrypto_a_OBJECTS)
