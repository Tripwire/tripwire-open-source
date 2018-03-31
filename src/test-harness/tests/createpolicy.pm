
use twtools;

package createpolicy;


######################################################################
# One time module initialization goes in here...
#
BEGIN 
{
    $description = "policy creation test";
    $testpolicydir = "$twtools::twrootdir/../../parser/testfiles";
    $badpolicydir = "$twtools::twrootdir/../../parser/testfiles.bad";
    $twpassed = 1;
}


######################################################################
# Try all policy files in specified directory, & verify each returns expected value
#
sub test_policy_dir
{
    my ($policydir, $expected) = @_;
    # my $twpassed = 1;

    opendir my $dir, $policydir or return 0;
    my @files = readdir $dir;
    closedir $dir;

    foreach my $current_file (@files) {

        if ($current_file eq "." || $current_file eq ".." ) {
            next;
        }

        twtools::logStatus "Trying policy text $policydir/$current_file\n";

        twtools::CreatePolicy({policy_text => "$policydir/$current_file"});
        if ( $? != $expected ) {
            twtools::logStatus ("*** create-polfile with $policydir/$current_file failed, error = $?\n");
            $twpassed = 0;
        }
    }

    twtools::logStatus("Done with policy dir $policydir, result = $twpassed\n");
    return $twpassed;
}


######################################################################
#
# Run the test.
#
sub run
{
    my $out = 1;


    # First try a bunch of policies that are supposed to succeed
    #
    twtools::logStatus("*** Beginning policy creation test\n");
    printf("%-30s", "-- policy creation test");

    test_policy_dir( "$testpolicydir", 0 );
    if ($twpassed) {
        ++$twtools::twpassedtests;
        print "PASSED\n";
    }
    else {
        ++$twtools::twfailedtests;
        print "*FAILED*\n";
        $out = 0;
    }


    # Now try some bad policies
    #
    twtools::logStatus("*** Beginning bad policy rejection test\n");
    printf("%-30s", "-- bad policy rejection test");

    $twpassed = 1;
    ++$twtools::twtotaltests;

    test_policy_dir( "$badpolicydir", 256 );
    if ($twpassed) {
        ++$twtools::twpassedtests;
        print "PASSED\n";
    }
    else {
        ++$twtools::twfailedtests;
        print "*FAILED*\n";
        $out = 0;
    }


    return $out;
}



######################################################################
#
# Initialize the test
#

sub initialize 
{
    return 1;
}


######################################################################
# One time module cleanup goes in here...
#
END 
{
}

1;

