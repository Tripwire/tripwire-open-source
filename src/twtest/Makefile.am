AUTOMAKE_OPTIONS = foreign no-dependencies
AM_INSTALL_PROGRAM_FLAGS = -m 755

AM_CPPFLAGS = -I$(srcdir)/..
LIBS = -ltripwire -lcryptlib @LIBS@
LDFLAGS = @LDFLAGS@ -L../../lib
LN_S = @LN@

sbin_PROGRAMS = twtest

twtestdir=.

twtest_SOURCES = \
archive_t.cpp \
blockfile_t.cpp \
blockrecordarray_t.cpp \
charutil_t.cpp \
cmdlineparser_t.cpp \
codeconvert_t.cpp \
configfile_t.cpp \
cryptoarchive_t.cpp \
crypto_t.cpp \
dbdatasource_t.cpp \
debug_t.cpp \
displayencoder_t.cpp \
error_t.cpp \
errorbucketimpl_t.cpp \
fcocompare_t.cpp \
fcodatabasefile_t.cpp \
fconame_t.cpp \
fconametbl_t.cpp \
fconametranslator_t.cpp \
fcopropimpl_t.cpp \
fcopropvector_t.cpp \
fcoreport_t.cpp \
fcosetimpl_t.cpp \
fcospec_t.cpp \
fcospecattr_t.cpp \
fcospechelper_t.cpp \
fcospeclist_t.cpp \
fcospecutil_t.cpp \
file_t.cpp \
fileheader_t.cpp \
fileutil_t.cpp \
fsdatasourceiter_t.cpp \
fsobject_t.cpp \
fspropcalc_t.cpp \
fspropdisplayer_t.cpp \
fspropset_t.cpp \
fsspec_t.cpp \
genre_t.cpp \
genrespeclist_t.cpp \
genreswitcher_t.cpp \
growheap_t.cpp \
hashtable_t.cpp \
hierdatabase_t.cpp \
keyfile_t.cpp \
platform_t.cpp \
policyparser_t.cpp \
refcountobj_t.cpp \
resources_t.cpp \
serializer_t.cpp \
serializerimpl_t.cpp \
signature_t.cpp \
srefcountobj_t.cpp \
stdtest.cpp \
stdtest.h \
stringencoder_t.cpp \
tasktimer_t.cpp \
tchar_t.cpp \
test.cpp \
test.h \
textreportviewer_t.cpp \
twlocale_t.cpp \
twutil_t.cpp \
types_t.cpp \
unixfsservices_t.cpp \
usernotifystdout_t.cpp \
wchar16_t.cpp

twtest_HEADERS = stdtest.h stringutil_t.h test.h

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	-rm -rf ../../bin/TWTestData
	-rm -f ../../bin/twtest 
	-rm -f *.gcno *.gcda

all: $(sbin_PROGRAMS)
	@test -d ../../bin && $(LN) -f $(sbin_PROGRAMS) ../../bin
