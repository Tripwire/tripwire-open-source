
use twtools;

package inoderef;

######################################################################
# One time module initialization goes in here...
#
BEGIN {

    %TESTS = (

              "0-createTarget" => {

                  changeFunc  => undef,
                  createFunc  => \&twtools::CreateFile,
                  file        => "parent.txt",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },

              "1-hardlink" => {

                  changeFunc  => undef,
                  createFunc  => \&twtools::CreateHardLink,
                  file        => "child1.txt",
                  target      => "parent.txt",
                  perms       => "a+w",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              "2-hardlink" => {

                  changeFunc  => \&twtools::RemoveFile,
                  createFunc  => \&twtools::CreateHardLink,
                  file        => "child2.txt",
                  target      => "parent.txt",
                  perms       => "a+w",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              "3-hardlink" => {

                  changeFunc  => \&twtools::RemoveFile,
                  createFunc  => \&twtools::CreateHardLink,
                  file        => "child3.txt",
                  target      => "child1.txt",
                  perms       => "a+w",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              );


    $description = "inode count (link) test";
}


######################################################################
# Return the policy file text for this test...
#
sub getPolicyFileString {

   return <<EOT;

$twtools::twcwd/$twtools::twrootdir/$TESTS{"0-createTarget"}{file}  -> +n;
$twtools::twcwd/$twtools::twrootdir/$TESTS{"1-hardlink"}{file}  -> -n;

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

    twtools::logStatus("*** Beginning $description\n");
    printf("%-30s", "-- $description");

    if ($^O eq "skyos" || $^O eq "haiku" || $^O eq "syllable") {
        ++$twtools::twskippedtests;
        print "SKIPPED; OS doesn't support hardlinks.\n";
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
    }
    else {
      ++$twtools::twfailedtests;
      print "*FAILED*\n";
    }
}


######################################################################
# One time module cleanup goes in here...
#
END {
}

1;
