AUTOMAKE_OPTIONS = foreign no-dependencies

INCLUDES = -I..

noinst_LIBRARIES = libtwparser.a
libtwparser_a_SOURCES = \
   genreparseinfo.cpp parserhelper.cpp parserobjects.cpp		\
   policyparser.cpp stdtwparser.cpp twparser.cpp twparsererrors.cpp	\
   twparserstrings.cpp yylex.cpp yyparse.cpp

DEFS = @DEFS@		# This gets rid of the -I. so INCLUDES must be more explicit

all: $(noinst_LIBRARIES)
	$(AR) ru ../../lib/libtripwire.a $(libtwparser_a_OBJECTS)
