AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I..

noinst_LIBRARIES = libfco.a
libfco_a_SOURCES = \
   fco.cpp fcocompare.cpp fcodatasourceiter.cpp				\
   fcodatasourceiterimpl.cpp fcoerrors.cpp fconame.cpp			\
   fconametbl.cpp fcopropimpl.cpp fcopropvector.cpp			\
   fcosetimpl.cpp fcospec.cpp fcospecattr.cpp fcospechelper.cpp		\
   fcospecimpl.cpp fcospeclist.cpp fcospecutil.cpp fcostrings.cpp	\
   fcoundefprop.cpp genreinfo.cpp genrespeclist.cpp			\
   genreswitcher.cpp signature.cpp stdfco.cpp twfactory.cpp
 
DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libfco_a_OBJECTS)
