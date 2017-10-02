
use twtools;

package readonly;


######################################################################
# One time module initialization goes in here...
#
BEGIN {

    %TESTS = (

              "0-permsChange" => {

                  changeFunc  => \&twtools::Chmod,
                  createFunc  => \&twtools::CreateFile,
                  file        => "readonly.txt",
                  perms       => "a+r-w",
                  contents    => "testing...",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },

              "1-accessed" => {

                  changeFunc  => \&twtools::Access,
                  createFunc  => \&twtools::CreateFile,
                  file        => "accessed.txt",
                  perms       => "a+r-w",
                  contents    => "testing...",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              "2-removed" => {

                  changeFunc  => \&twtools::RemoveFile,
                  createFunc  => \&twtools::CreateFile,
                  file        => "removed.txt",
                  perms       => "a+r-w",
                  contents    => "testing",
                  violations  => "V:1 S:0 A:0 R:1 C:0"
                  }

              );


    $description = "readonly test";
}


######################################################################
# Return the policy file text for this test...
#
sub getPolicyFileString {

   return <<EOT;

$twtools::twcwd/$twtools::twrootdir/$TESTS{"0-permsChange"}{file} -> \$(ReadOnly) -a;
$twtools::twcwd/$twtools::twrootdir/$TESTS{"1-accessed"}{file}    -> \$(ReadOnly) -a;
$twtools::twcwd/$twtools::twrootdir/$TESTS{"2-removed"}{file}     -> \$(ReadOnly) -a;

EOT

}


######################################################################
#
# Initialize, get ready to run this test...
#

sub initialize() {
    
    my $twstr = getPolicyFileString();
    twtools::GeneratePolicyFile($twstr);

}


######################################################################
#
# Run the test.
#
sub run() {

    twtools::logStatus("\n\n*** Beginning $description\n");
    printf("%-30s", "-- $description");

    if ($^O eq "skyos") {
        ++$twtools::twskippedtests;
        print "SKIPPED; SkyOS doesn't support readonly files.\n";
        return;
    } elsif ($^O eq "gnu") {
        ++$twtools::twskippedtests;
        print "SKIPPED; TODO: Hurd has fewer expected changes here; refactor so we can test for correct values\n";
        return;
    }


    my $twpassed = 1;

    #########################################################
    #
    # Run the tests describe above in the %TESTS structure.
    #
    $twpassed = twtools::RunIntegrityTests(%TESTS);


    #########################################################
    #
    # See if the tests all succeeded...
    #
    if ($twpassed) {
      ++$twtools::twpassedtests;
      print "PASSED\n";
      return 0;
    }
    else {
      print "*FAILED*\n";
      ++$twtools::twfailedtests;
    }
}


######################################################################
# One time module cleanup goes in here...
#
END {
}

1;

