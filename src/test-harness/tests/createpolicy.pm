
use twtools;

package createpolicy;


######################################################################
# One time module initialization goes in here...
#
BEGIN 
{
    $description = "policy creation test";
    $testpolicydir = "$twtools::twrootdir/../../parser/testfiles";
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

    # Test with existing test case files in the src/policy/testfiles directory.
    #
    twtools::CreatePolicy({policy-text => "$testpolicydir/directives.txt"});
    if ( $? != 0 ) {
        twtools::logStatus("create-polfile with directives.txt failed, error = $?\n");
        $twpassed = 0;
    }

    twtools::CreatePolicy({policy-text => "$testpolicydir/pol.txt"});
    if ( $? != 0 ) {
        twtools::logStatus("create-polfile with pol.txt failed, error = $?\n");
        $twpassed = 0;
    }

    twtools::CreatePolicy({policy-text => "$testpolicydir/poleasy.txt"});
    if ( $? != 0 ) {
        twtools::logStatus("create-polfile with poleasy.txt failed, error = $?\n");
        $twpassed = 0;
    }

    twtools::CreatePolicy({policy-text => "$testpolicydir/polhard.txt"});
    if ( $? != 0 ) {
        twtools::logStatus("create-polfile with polhard.txt failed, error = $?\n");
        $twpassed = 0;
    }

    twtools::CreatePolicy({policy-text => "$testpolicydir/polruleattr.txt"});
    if ( $? != 0 ) {
        twtools::logStatus("create-polfile with polruleattr.txt failed, error = $?\n");
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

