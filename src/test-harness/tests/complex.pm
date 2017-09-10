
use twtools;

package complex;

######################################################################
# One time module initialization goes in here...
#
BEGIN {

    %TESTS = (

              "0-createDir" => {

                  # An add violation for adding blah.txt,
                  # and a change violation for changing the
                  # contents of the directory proper.
                  #
                  changeFunc  => \&twtools::CreateFile,
                  createFunc  => \&twtools::CreateDir,
                  file        => "blah.txt",
                  dir         => "temp",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:2 S:0 A:1 R:0 C:1"
                  },

              "1-createFile" => {

                  changeFunc  => undef,
                  createFunc  => \&twtools::CreateFile,
                  file        => "test.txt",
                  dir         => "temp",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              "2-createDir" => {

                  changeFunc  => undef,
                  createFunc  => \&twtools::CreateDir,
                  dir         => "temp/dog",
                  perms       => "a+w",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              "3-createDir" => {

                  changeFunc  => undef,
                  createFunc  => \&twtools::CreateDir,
                  dir         => "temp/cat",
                  perms       => "a+w",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              # Shouldn't get any violation for creating this
              # file since we put a stop rule on temp/dog.
              #
              "4-createFile" => {

                  changeFunc  => \&twtools::Chmod,
                  createFunc  => \&twtools::CreateFile,
                  dir         => "temp/dog",
                  file        => "test.txt",
                  perms       => "a+r",
                  contents    => "testing",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },

              # One change violation for the directory temp/cat, and
              # one change violation for the file test.txt itself.
              #
              "5-createFile" => {

                  changeFunc  => \&twtools::Chmod,
                  createFunc  => \&twtools::CreateFile,
                  dir         => "temp/cat",
                  file        => "test.txt",
                  perms       => "a+r",
                  contents    => "testing",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },
              );


    $description = "nested policy test";
    
}


######################################################################
# Return the policy file text for this test...
#
sub getPolicyFileString {

   return <<EOT;

(rulename=level1) 
{

  $twtools::twcwd/$twtools::twrootdir/$TESTS{"1-createFile"}{dir}/$TESTS{"1-createFile"}{file}  -> \$(ReadOnly);

  (rulename=level2) 
  {
    $twtools::twcwd/$twtools::twrootdir/$TESTS{"0-createDir"}{dir}  -> \$(ReadOnly) (recurse=true);
    !$twtools::twcwd/$twtools::twrootdir/$TESTS{"2-createDir"}{dir};
  }
  
}

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

    if ($^O eq "skyos" || $^O eq "gnu") {
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
      print "PASSED\n";
      ++$twtools::twpassedtests;
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
