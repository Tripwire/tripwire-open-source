#!/usr/bin/perl

use strict;
use lib '.';
use twtools;

my @twtests       = ();
my @twtestresults = ();


######################################################################
# process command line arguments...
#
sub processCommandLine() {

    while (scalar(@ARGV)) {
        
        my ($arg) = shift @ARGV;
        
        
        if ($arg =~ /-email/) {
            
            $twtools::twemail = shift @ARGV;
        }
        
        
        if ($arg =~ /-rootdir/) {
            
            $twtools::twrootdir = shift @ARGV;
        }


        if ($arg =~ /-bin/) {

            $twtools::twbinaries = shift @ARGV;

        }


        if ($arg !~ /^-/) {

            push @twtests, $arg;
        }
        
    }


    chomp($twtools::twrootdir);
    chomp($twtools::twcwd);

    # Make sure there is no trailing '/'
    # 
    chop($twtools::twcwd) if $twtools::twcwd =~ /\/$/;
    chop($twtools::twrootdir) if $twtools::twrootdir =~ /\/$/;

    # report file location, passed to tripwire executables
    # if not explicitly specified.
    $twtools::reportloc = "$twtools::twrootdir/report/test.twr";

}


######################################################################
# try and find any .pm files in the ./tests directory
# which are test scripts...
#
sub prepareListOfTests {

    my @tests;

    opendir(DIR, "./tests");
    @tests = readdir(DIR);
    closedir(DIR);

    # These two nasties first pull out only the files
    # ending in ".pm", then strip the ".pm" from each
    # name, which is necessary later on when we dynamically
    # use each module...
    #
    @twtests = grep(/\.pm$/, @tests);
    map { $_ =~ /(.*)\.pm$/; $_ = "$1"; } @twtests;
}


######################################################################
# Call the initialize for each test script passing in 
# the current working directory, and the user specified
# root directory for creating the test tree structure.
#
sub runTests {

    my ($module);
    my ($ret) = 0;

    for $module (@twtests) {

	++$twtools::twtotaltests;

        # use the module
        #
        eval qq{use tests::$module};

        # if that worked, call initialize...
        #
        if (!$@) { # make sure use succeeded....

            eval qq{ 

                ${module}->initialize() if defined &${module}::initialize
            };

            eval qq {
                ${module}->run() if defined &${module}::run
            };

            eval qq {
                ${module}->cleanup() if defined &${module}::cleanup
            };

        }
        else {
            print "Could not find or load test module '$module', error = $@ \n";
        }
    }
}


######################################################################
######################################################################
######################################################################

# Turn off buffered output...
$| = 1;

processCommandLine();

prepareListOfTests() if scalar(@twtests) == 0;  # only if none were on the cmdline

print "\n";
print "initializing for tests on $^O...\n\n";

print "logging to $ENV{'PWD'}/$twtools::twrootdir/status.log\n\n";

# all tests can assume a base configuration, i.e. default tw.cfg, site and local keys
#
twtools::CreateDefaultConfig();
twtools::GenerateKeys();
twtools::SignConfigFile();

print "test results:\n";
print "=============\n\n";

# Do the tests...
#
runTests();

# Any test that didn't report a status gets counted as skipped.
$twtools::twskippedtests += ($twtools::twtotaltests - ($twtools::twpassedtests + $twtools::twfailedtests + $twtools::twskippedtests));

print "\n\n$twtools::twtotaltests test(s) run\n";
print "$twtools::twpassedtests test(s) passed\n";
print "$twtools::twfailedtests test(s) failed\n";
print "$twtools::twskippedtests test(s) skipped\n\n";

exit($twtools::twfailedtests);

