AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I$(srcdir)/..

noinst_LIBRARIES = libtw.a
libtw_adir=.
libtw_a_SOURCES = \
   configfile.cpp dbdatasource.cpp dbdebug.cpp dbexplore.cpp	\
   fcodatabasefile.cpp fcodatabaseutil.cpp fcoreport.cpp	\
   fcoreportutil.cpp filemanipulator.cpp headerinfo.cpp		\
   policyfile.cpp stdtw.cpp systeminfo.cpp textdbviewer.cpp	\
   textreportviewer.cpp tw.cpp twerrors.cpp twinit.cpp		\
   twstrings.cpp twutil.cpp
 
libtw_a_HEADERS = \
   configfile.h dbdatasource.h dbdebug.h dbexplore.h \
   fcodatabasefile.h fcodatabaseutil.h fcoreport.h   \
   fcoreportutil.h filemanipulator.h headerinfo.h policyfile.h \
   stdtw.h systeminfo.h textdbviewer.h textreportviewer.h \
   tw.h twerrors.h twinit.h twstrings.h twutil.h

DEFS = @DEFS@ -DCONFIG_DIR=\"$(sysconfdir)\"
CLEANFILES = *.gcno *.gcda

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libtw_a_OBJECTS)
