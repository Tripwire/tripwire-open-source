AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I$(srcdir)/..

noinst_LIBRARIES = libfco.a
libfco_adir=.
libfco_a_SOURCES = \
   fco.cpp fcocompare.cpp fcodatasourceiter.cpp				\
   fcodatasourceiterimpl.cpp fcoerrors.cpp fconame.cpp			\
   fconametbl.cpp fcopropimpl.cpp fcopropvector.cpp			\
   fcosetimpl.cpp fcospec.cpp fcospecattr.cpp fcospechelper.cpp		\
   fcospecimpl.cpp fcospeclist.cpp fcospecutil.cpp fcostrings.cpp	\
   fcoundefprop.cpp genreinfo.cpp genrespeclist.cpp			\
   genreswitcher.cpp signature.cpp stdfco.cpp twfactory.cpp

libfco_a_HEADERS = \
   fco.h fcocompare.h fcodatasource.h fcodatasourceiter.h      \
   fcodatasourceiterimpl.h fcoerrors.h fcogenre.h fconame.h    \
   fconameinfo.h fconametbl.h fconametranslator.h fcoprop.h    \
   fcopropcalc.h fcopropdisplayer.h fcopropimpl.h fcopropset.h \
   fcopropvector.h fcosetimpl.h fcosetws.h		       \
   fcospec.h fcospecattr.h fcospechelper.h fcospecimpl.h       \
   fcospeclist.h fcospecutil.h fcostrings.h fcoundefprop.h     \
   fcovisitor.h genreinfo.h genrespeclist.h genreswitcher.h    \
   iterproxy.h parsergenreutil.h propset.h signature.h         \
   stdfco.h twfactory.h


DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = *.gcno *.gcda

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libfco_a_OBJECTS)
