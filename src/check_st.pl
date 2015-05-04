###############################################################################
# check_st.pl
#
# created 26 may 1999 mdb
#
# Purpose:
#	This script verifies that all of the string refrenced in a tripwire string 
#	table header file has a corresponding entry in the .cpp file
#
# Usage:
#	Run from the root Tripwire directory. set PKGS to the package names that you 
#	want to check.
#
# TODO: gracefully handle packages with no string table
# TODO: handle commented out blocks in cpp files.
#
@PKGS = (	"core", "fco", "tw", "fs", "ntfs", 
			"ntreg", "siggen", "tripwire", "twadmin", "twprint" );	

##-----------------------------------------------------------------------------
## execute main
##-----------------------------------------------------------------------------
sub main;
main();
exit 0;

##-----------------------------------------------------------------------------
## make_string_list
##
##		opens the string header file and retrieves all of the string enums 
##		(identified by lines starting with \s*STR_) and returns them in a list
##
##	Parameters:
##		package name (ie -- core)
##-----------------------------------------------------------------------------
sub make_string_list
{
	my $pkg_name = shift( @_ );
	my $header_name = "${pkg_name}/${pkg_name}strings.h";
	my @rtn_list;
	open( IN, "<$header_name" ) || die "Unable to open $header_name\n";

	while( <IN> )
	{
		if( /^\s*(STR_[a-zA-Z0-9_]+)/ )
		{
			push( @rtn_list, $1 );
		}
	}
	close( IN );

	return @rtn_list;
}

##-----------------------------------------------------------------------------
## verify_string_list
##
##		takes as input a list of string identifiers, opens the string cpp
##		file and makes sure that all strings in the list appear in that file.
##
##	Limitation:
##		can't tell whether entries in the cpp file are commented out.
##-----------------------------------------------------------------------------
sub verify_string_list
{
	my $pkg_name = shift( @_ );
	my @str_list = @_;
	my $cpp_name = "${pkg_name}/${pkg_name}strings.cpp";
	my @cpp_lines;
	my $result;

	# first, load the entire cpp file into an array...
	#
	open( IN, "<$cpp_name" ) || die "Unable to open $cpp_name\n";
	while( <IN> )
	{
		push( @cpp_lines, $_ );
	}

	# now, iterate through the string list, looking for each
	# string in the cpp file.
	#
	foreach $str (@str_list)
	{
		$result = grep( /$str/, @cpp_lines );
		if( $result == 0 )
		{
			print "$str is missing from $pkg_name string table!\n";
		}
	}
}

##-----------------------------------------------------------------------------
## main
##-----------------------------------------------------------------------------
sub main
{
	my @str_list;
	foreach $pkg (@PKGS)
	{
		print "\n*** Processing $pkg ***\n";
		@str_list = make_string_list( $pkg );
		verify_string_list( $pkg, @str_list );
	}
}