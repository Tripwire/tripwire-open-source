AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I.. -I../..
LN=@LN@
DEFAULT_INCLUDES = $(D_INCLUDES)
EXTRA_DIST=license.txt readme.txt rmcr.pl

noinst_LIBRARIES = libcryptlib.a
libcryptlib_adir=.
libcryptlib_a_SOURCES = \
  algebra.cpp asn.cpp cryptlib.cpp des.cpp dessp.cpp elgamal.cpp \
  eprecomp.cpp filters.cpp forkjoin.cpp integer.cpp iterhash.cpp misc.cpp \
  nbtheory.cpp pch.cpp queue.cpp rng.cpp sha.cpp zbits.cpp zdeflate.cpp \
  zinflate.cpp ztrees.cpp
 
libcryptlib_a_HEADERS = \
  algebra.h asn.h config.h cryptlib.h des.h elgamal.h eprecomp.h filters.h \
  forkjoin.h integer.h iterhash.h misc.h modarith.h nbtheory.h pch.h \
  queue.h rng.h sha.h smartptr.h words.h zbits.h zdeflate.h zinflate.h \
  ztrees.h

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = ../../lib/libcryptlib.a *.gcno *.gcda

all: $(noinst_LIBRARIES)
	$(LN) -f $(noinst_LIBRARIES) ../../lib/libcryptlib.a
