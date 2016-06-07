AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I..

noinst_LIBRARIES = libtwcrypto.a
libtwcrypto_a_SOURCES = \
   bytequeue.cpp crypto.cpp cryptoarchive.cpp keyfile.cpp \
   stdtwcrypto.cpp twcrypto.cpp twcryptoerrors.cpp

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libtwcrypto_a_OBJECTS)
