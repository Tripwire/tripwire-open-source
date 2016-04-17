#
# test.mak
# See common_rules.mak for comments

include ../make_include/$(SYSPRE).inc

EXECUTABLE = test
TYPE = BINARY

ADD_OBJS = file_unix.o unixfsservices.o unixfsservices_t.o archive_t.o cmdlineparser_t.o codeconvert_t.o debug_t.o displayencoder_t.o error_t.o errorbucketimpl_t.o file_t.o fileheader_t.o growheap_t.o hashtable_t.o objectpool_t.o platform_t.o refcountobj_t.o resources_t.o serializer_t.o serializerimpl_t.o srefcountobj_t.o tasktimer_t.o tchar_t.o twlocale_t.o types_t.o usernotifystdout_t.o wchar16_t.o blockfile_t.o blockrecordarray_t.o hierdatabase_t.o fcocompare_t.o fconame_t.o fconametbl_t.o fconametranslator_t.o fcopropimpl_t.o fcopropvector_t.o fcosetimpl_t.o fcospec_t.o fcospecattr_t.o fcospechelper_t.o fcospeclist_t.o fcospecutil_t.o fsspec_t.o genre_t.o genrespeclist_t.o genreswitcher_t.o signature_t.o fileutil_t.o stringencoder_t.o fsdatasourceiter_t.o fsobject_t.o fspropcalc_t.o fspropdisplayer_t.o fspropset_t.o configfile_t.o dbdatasource_t.o fcodatabasefile_t.o fcoreport_t.o textreportviewer_t.o twutil_t.o cryptoarchive_t.o crytpo_t.o keyfile_t.o

ADD_SRCS = ../core/unix/file_unix.cpp ../core/unix/unixfsservices.cpp ../core/unix/unixfsservices_t.cpp ../core/archive_t.cpp ../core/cmdlineparser_t.cpp ../core/codeconvert_t.cpp ../core/debug_t.cpp ../core/displayencoder_t.cpp ../core/error_t.cpp ../core/errorbucketimpl_t.cpp ../core/file_t.cpp ../core/fileheader_t.cpp ../core/growheap_t.cpp ../core/hashtable_t.cpp ../core/objectpool_t.cpp ../core/platform_t.cpp ../core/refcountobj_t.cpp ../core/resources_t.cpp ../core/serializer_t.cpp ../core/serializerimpl_t.cpp ../core/srefcountobj_t.cpp ../core/tasktimer_t.cpp ../core/tchar_t.cpp ../core/twlocale_t.cpp ../core/types_t.cpp ../core/usernotifystdout_t.cpp ../core/wchar16_t.cpp ../db/blockfile_t.cpp ../db/blockrecordarray_t.cpp ../db/hierdatabase_t.cpp ../fco/fcocompare_t.cpp ../fco/fconame_t.cpp ../fco/fconametbl_t.cpp ../fco/fconametranslator_t.cpp ../fco/fcopropimpl_t.cpp ../fco/fcopropvector_t.cpp ../fco/fcosetimpl_t.cpp ../fco/fcospec_t.cpp ../fco/fcospecattr_t.cpp ../fco/fcospechelper_t.cpp ../fco/fcospeclist_t.cpp ../fco/fcospecutil_t.cpp ../fco/fsspec_t.cpp ../fco/genre_t.cpp ../fco/genrespeclist_t.cpp ../fco/genreswitcher_t.cpp ../fco/signature_t.cpp ../util/fileutil_t.cpp ../util/stringencoder_t.cpp ../fs/fsdatasourceiter_t.cpp ../fs/fsobject_t.cpp ../fs/fspropcalc_t.cpp ../fs/fspropdisplayer_t.cpp ../fs/fspropset_t.cpp ../tw/configfile_t.cpp ../tw/dbdatasource_t.cpp ../tw/fcodatabasefile_t.cpp ../tw/fcoreport_t.cpp ../tw/textreportviewer_t.cpp ../tw/twutil_t.cpp ../twcrypto/cryptoarchive_t.cpp ../twcrypto/crytpo_t.cpp ../twcrypto/keyfile_t.cpp


DSP_OBJS = test.o
DSP_SRCS = test.cpp

OBJS = $(DSP_OBJS) $(ADD_OBJS)
SRCS = $(DSP_SRCS) $(ADD_SRCS)


include ../make_include/common_rules.mak


##### targets for object files #####

# Here is the list of how to compile ADD_OBJS from ADD_SRCS
# Because of the directory differences in the .cpp files, these have to be itemized 
# individually and cannot be included in the generic target in common_rules.mak

$(OBJDIR)/file_unix.o : ../core/unix/file_unix.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/unixfsservices.o : ../core/unix/unixfsservices.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/unixfsservices_t.o : ../core/unix/unixfsservices_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/archive_t.o : ../core/archive_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/cmdlineparser_t.o : ../core/cmdlineparser_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/codeconvert_t.o : ../core/codeconvert_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/debug_t.o : ../core/debug_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/displayencoder_t.o : ../core/displayencoder_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/error_t.o : ../core/error_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/errorbucketimpl_t.o : ../core/errorbucketimpl_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/file_t.o : ../core/file_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fileheader_t.o : ../core/fileheader_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/growheap_t.o : ../core/growheap_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/hashtable_t.o : ../core/hashtable_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/objectpool_t.o : ../core/objectpool_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/platform_t.o : ../core/platform_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/refcountobj_t.o : ../core/refcountobj_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/resources_t.o : ../core/resources_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/serializer_t.o : ../core/serializer_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/serializerimpl_t.o : ../core/serializerimpl_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/srefcountobj_t.o : ../core/srefcountobj_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/tasktimer_t.o : ../core/tasktimer_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/tchar_t.o : ../core/tchar_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/twlocale_t.o : ../core/twlocale_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/types_t.o : ../core/types_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/usernotifystdout_t.o : ../core/usernotifystdout_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/wchar16_t.o : ../core/wchar16_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/blockfile_t.o : ../db/blockfile_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/blockrecordarray_t.o : ../db/blockrecordarray_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/hierdatabase_t.o : ../db/hierdatabase_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/acl_t.o : ../fco/acl_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcocompare_t.o : ../fco/fcocompare_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

test_$(SYSPRE)_r/fconame_t.o : ../fco/fconame_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fconame_t.o : ../fco/fconame_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fconametbl_t.o : ../fco/fconametbl_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fconametranslator_t.o : ../fco/fconametranslator_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcopropimpl_t.o : ../fco/fcopropimpl_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcopropvector_t.o : ../fco/fcopropvector_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcosetimpl_t.o : ../fco/fcosetimpl_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcospec_t.o : ../fco/fcospec_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcospecattr_t.o : ../fco/fcospecattr_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcospechelper_t.o : ../fco/fcospechelper_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcospeclist_t.o : ../fco/fcospeclist_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcospecutil_t.o : ../fco/fcospecutil_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fsspec_t.o : ../fco/fsspec_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/genre_t.o : ../fco/genre_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/genrespeclist_t.o : ../fco/genrespeclist_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/genreswitcher_t.o : ../fco/genreswitcher_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/sid_t.o : ../fco/sid_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/signature_t.o : ../fco/signature_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fileutil_t.o : ../util/fileutil_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/stringencoder_t.o : ../util/stringencoder_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fsdatasourceiter_t.o : ../fs/fsdatasourceiter_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fsobject_t.o : ../fs/fsobject_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fspropcalc_t.o : ../fs/fspropcalc_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fspropdisplayer_t.o : ../fs/fspropdisplayer_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fspropset_t.o : ../fs/fspropset_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/configfile_t.o : ../tw/configfile_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/dbdatasource_t.o : ../tw/dbdatasource_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcodatabasefile_t.o : ../tw/fcodatabasefile_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/fcoreport_t.o : ../tw/fcoreport_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/textreportviewer_t.o : ../tw/textreportviewer_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/twutil_t.o : ../tw/twutil_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/cryptoarchive_t.o : ../twcrypto/cryptoarchive_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/crytpo_t.o : ../twcrypto/crytpo_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<

$(OBJDIR)/keyfile_t.o : ../twcrypto/keyfile_t.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(DEFINES) -c $<
