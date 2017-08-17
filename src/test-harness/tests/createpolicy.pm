
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
}

######################################################################
# various policies
#
sub basic_policy
{
	return <<POLICY_END;

/foo -> +S;
/bar -> \$(IgnoreNone);
!/baz;

POLICY_END
}

sub variable_policy
{
    return <<POLICY_END;
\@\@section GLOBAL
FOO = /foo ;
BAR = +pinug ;

\@\@section FS
\$(FOO) -> \$(BAR);

POLICY_END
}

sub host_conditional_policy
{
	return <<POLICY_END;
\@\@ifhost localhost
\@\@print Hello World
    /foo -> \$(IgnoreNone);
\@\@else
\@\@error failed
\@\@endif

POLICY_END
}


sub test_policy_dir
{
    my ($policydir, $expected) = @_;
    my $twpassed = 1;

    opendir my $dir, $policydir or return 0;
    my @files = readdir $dir;
    closedir $dir;

    foreach my $current_file (@files) {

        if ($current_file eq "." || $current_file eq ".." ) {
            next;
        }

        twtools::logStatus "Trying policy text $policydir/$current_file\n";

        twtools::CreatePolicy({policy-text => "$policydir/$current_file"});
        if ( $? != $expected ) {
            twtools::logStatus ("create-polfile with $policydir/$current_file failed, error = $?\n");
            $twpassed = 0;
        }
    }

    return $twpassed;
}



######################################################################
#
# Run the test.
#
sub run
{
    my $twpassed = 1;

    twtools::logStatus("*** Beginning policy creation test\n");
    printf("%-30s", "-- $description");

    twtools::GeneratePolicyFile( basic_policy() );
    if ( $? != 0 ) {
        twtools::logStatus("basic create-polfile failed, error = $?\n");
        $twpassed = 0;
    }

    twtools::GeneratePolicyFile( variable_policy() );
    if ( $? != 0 ) {
        twtools::logStatus("create-polfile with variables failed, error = $?\n");
        $twpassed = 0;
    }

    twtools::GeneratePolicyFile( host_conditional_policy() );
    if ( $? != 0 ) {
        twtools::logStatus("create-polfile with ifhost conditional failed, error = $?\n");
        $twpassed = 0;
    }

    test_policy_dir( "$testpolicydir", 0 );
    if ( $? != 0 ) {
        $twpassed = 0;
    }

    test_policy_dir( "$badpolicydir", 1 );
    if ( $? != 0 ) {
        $twpassed = 0;
    }

    #########################################################
    #
    # See if the tests all succeeded...
    #
    if ($twpassed) {
        ++$twtools::twpassedtests;
        print "PASSED\n";
        return 1;
    }
    else {
        ++$twtools::twfailedtests;
        print "*FAILED*\n";
        return 0;
    }
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

