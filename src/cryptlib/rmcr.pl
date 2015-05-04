#!/usr/bin/perl -w

# rmcr.pl
#
# remove all "\r\n" sequences.

if (!$ARGV[0]) {
	print <<EOF;
Usage: rmcr file1 [file2 ...]

Replaces \\r\\n with \\n.
EOF
}

foreach $filename (@ARGV) 
{
    $buffer = "";
    if (open (IN, "<$filename"))
    {
	while (defined($instring = <IN>)) 
        {
            $instring =~ s/\r\n/\n/;
	    $buffer .= $instring;
        }
        close (IN);

        if (open (OUT, ">$filename"))
        {
	    print OUT $buffer;
	    close (OUT);
        }
        else 
        {
            print "Could not open $filename for writing.\n";
        }
    }
    else 
    {
        print "Could not open $filename for reading.\n";
    }
}
