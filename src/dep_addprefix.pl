#!/usr/bin/perl -w
    
# include a null char so VSS treats this as a binary file --> " #"

#####################################################################
# dep_addprefix.pl
#
# Usage: dep_addprefix.pl myproject.mak.dep objdir1 [objdir2 ...]
#
# Modifies a "make depends" file so that it supports the generation of
# object files in subdirs.
#
# For example if our source file is foo.cpp and we want to have
# the release and debug objects live different directories we would 
# run:
#
#   dep_addprefix.pl Makefile.dep release debug
#
# This would take the instances of 
#
#   foo.o: dependantcy.h
#
# and change them to
#
#   release/foo.o: dependancy.h
#   debug/foo.o: dependancy.h
#
#
# ### May 2nd, 2000 - dmb
#
#   Added the ability to cope with g++ -M output, which uses the "\" character
#   to continue long lines. As a first step we turn multiple lines ending with 
#   "\" to a single line in order to cope with this.
#

if (!defined($ARGV[0]) || !defined($ARGV[1])) 
{
    print <<EOF;
Usage: dep_addprefix.pl myproject.mak.dep objdir1 [objdir2 ...]

Example: dep_addprefix.pl tripwire.mak.dep tripwire_r tripwire_d
EOF
    exit(1);
}

$dep_file = $ARGV[0];

$i = 1;
@objdir_list = ();
while (defined($ARGV[$i]))
{
    if ($ARGV[$i] =~ /\/$/)
    {
        push @objdir_list, $ARGV[$i];
    }
    else
    {
        push @objdir_list, $ARGV[$i] . "/";
    }
    $i++;
}

$text = "";
$process_string = "";
open (IN, "<$dep_file") || die "Can't open $dep_file for reading.\n";
while (defined($instring = <IN>))
{
    # the following if/else logic concatinates lines that end with the "\" character
    if ($instring =~ /.*\\$/)
    {
        $instring =~ s/(.*)\\$/$1/;
        $process_string .= substr($instring, 0, -1); # use substr(,,-1) to avoid eol character
    }
    else
    {
        $process_string .= $instring;

        if ($process_string =~ /^.*:.*$/)
        {
            foreach $objdir (@objdir_list)
            {
                $text .= $objdir . $process_string;
            }
        }
        else
        {
            $text .= $process_string;
        }

        $process_string = "";
    }
}

close(IN);

open (OUT, ">$dep_file") || die "Can't open $dep_file for writing.\n";
print OUT $text;
close (OUT);

exit(0);
__END__

