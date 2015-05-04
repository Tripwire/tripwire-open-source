AUTOMAKE_OPTIONS = foreign no-dependencies

INCLUDES = -I..

noinst_LIBRARIES = libtw.a
libtw_a_SOURCES = \
   configfile.cpp dbdatasource.cpp dbdebug.cpp dbexplore.cpp	\
   fcodatabasefile.cpp fcodatabaseutil.cpp fcoreport.cpp	\
   fcoreportutil.cpp filemanipulator.cpp headerinfo.cpp		\
   policyfile.cpp stdtw.cpp systeminfo.cpp textdbviewer.cpp	\
   textreportviewer.cpp tw.cpp twerrors.cpp twinit.cpp		\
   twstrings.cpp twutil.cpp
 
DEFS = @DEFS@ -DCONFIG_DIR=\"$(sysconfdir)\"

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libtw_a_OBJECTS)
