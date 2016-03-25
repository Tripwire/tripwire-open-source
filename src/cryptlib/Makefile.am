AUTOMAKE_OPTIONS = foreign no-dependencies

INCLUDES = -I.. -I../..
LN=@LN@
DEFAULT_INCLUDES = $(D_INCLUDES)

noinst_LIBRARIES = libcryptlib.a
libcryptlib_a_SOURCES = \
  algebra.cpp asn.cpp cryptlib.cpp des.cpp dessp.cpp elgamal.cpp \
  eprecomp.cpp filters.cpp forkjoin.cpp integer.cpp iterhash.cpp misc.cpp \
  nbtheory.cpp pch.cpp queue.cpp rng.cpp sha.cpp zbits.cpp zdeflate.cpp \
  zinflate.cpp ztrees.cpp
 
DEFS = @DEFS@		# This gets rid of the -I. so INCLUDES must be more explicit
CLEANFILES = ../../lib/libcryptlib.a

all: $(noinst_LIBRARIES)
	$(LN) -f $(noinst_LIBRARIES) ../../lib/libcryptlib.a
