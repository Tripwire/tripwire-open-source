
use twtools;

package dirs;

######################################################################
# One time module initialization goes in here...
#
BEGIN {

    %TESTS = (

              "0-createTempDir" => {

                  changeFunc  => undef,
                  createFunc  => \&twtools::CreateDir,
                  dir         => "temp",
                  perms       => "a+w",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              "1-createChildDir" => {

                  changeFunc  => undef,
                  createFunc  => \&twtools::CreateDir,
                  dir         => "temp/cat",
                  perms       => "a+w",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              "2-createChildDir" => {

                  changeFunc  => undef,
                  createFunc  => \&twtools::CreateDir,
                  dir         => "temp/cat/gray",
                  perms       => "a+w",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              "3-createFile" => { 

                  # this one will cause an add (file added) AND change
                  # (contents of temp changed) violation.

                  changeFunc  => \&twtools::CreateFile,
                  createFunc  => undef,
                  file        => "file.txt",
                  dir         => "temp",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:2 S:0 A:1 R:0 C:1"
                  },

              "4-createFile" => {

                  # this one will cause a changed violation, since recurse
                  # is set to 1...

                  changeFunc  => \&twtools::CreateFile,
                  createFunc  => undef,
                  file        => "file.txt",
                  dir         => "temp/cat",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },

              "5-createFile" => {

                  # we are below the recurse level here, so no
                  # violation for doing whatever...
                  #
                  changeFunc  => \&twtools::CreateFile,
                  createFunc  => undef,
                  file        => "file.txt",
                  dir         => "temp/cat/gray",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              );


    $description = "directory (recurse) test";
}


######################################################################
# Return the policy file text for this test...
#
sub getPolicyFileString {

   return <<EOT;

$twtools::twcwd/$twtools::twrootdir/$TESTS{"0-createTempDir"}{dir} -> \$(ReadOnly)(recurse = 1);

EOT

}

######################################################################
#
# Initialize, get ready to run this test...
#
sub initialize() {
    
    my $twstr = getPolicyFileString();
    twtools::GeneratePolicyFile($twstr);

    # this test requires a clean start of it's tree
    #
    system("rm -rf $twtools::twrootdir/" . $TESTS{"0-createTempDir"}{dir});
}


######################################################################
#
# Run the test.
#
sub run() {

    twtools::logStatus("*** Beginning $description\n");
    printf("%-30s", "-- $description");

    if (($^O eq "skyos") || ($^O eq "dragonfly")) {
        ++$twtools::twskippedtests;
        print "SKIPPED; TODO: OS has fewer expected changes here; refactor so we can test for correct values\n";
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
