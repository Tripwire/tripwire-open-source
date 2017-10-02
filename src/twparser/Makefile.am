AUTOMAKE_OPTIONS = foreign no-dependencies

AM_CPPFLAGS = -I$(srcdir)/..

noinst_LIBRARIES = libtwparser.a
libtwparser_adir=.
libtwparser_a_SOURCES = \
   genreparseinfo.cpp parserhelper.cpp parserobjects.cpp		\
   policyparser.cpp stdtwparser.cpp twparser.cpp twparsererrors.cpp	\
   twparserstrings.cpp yylex.cpp yyparse.cpp

libtwparser_a_HEADERS = \
   genreparseinfo.h parserhelper.h parserobjects.h policyparser.h \
   stdtwparser.h twparser.h twparsererrors.h twparserstrings.h \
   yylex.h yyparse.h

DEFS = @DEFS@		# This gets rid of the -I. so AM_CPPFLAGS must be more explicit
CLEANFILES = *.gcno *.gcda

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libtwparser_a_OBJECTS)
