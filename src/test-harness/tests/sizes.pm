
use twtools;

package sizes;

######################################################################
# One time module initialization goes in here...
#
BEGIN {

    %TESTS = (

              "0-getsBigger" => {

                  changeFunc  => \&twtools::MakeBigger,
                  createFunc  => \&twtools::CreateFile,
                  file        => "getsbigger.txt",
                  perms       => "a+w",
                  contents    => "test",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },

              "1-getsSmaller" => {

                  changeFunc  => \&twtools::MakeSmaller,
                  createFunc  => \&twtools::CreateFile,
                  file        => "getssmaller.txt",
                  perms       => "a+w",
                  contents    => "test",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },

              "2-okayToGrow" => {

                  changeFunc  => \&twtools::MakeBigger,
                  createFunc  => \&twtools::CreateFile,
                  file        => "okaytogrow.txt",
                  perms       => "a+w",
                  contents    => "test",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  }

              );


    $description = "file sizes test";
}


######################################################################
# Return the policy file text for this test...
#
sub getPolicyFileString {

   return <<EOT;

$twtools::twcwd/$twtools::twrootdir/$TESTS{"0-getsBigger"}{file}  -> +s;
$twtools::twcwd/$twtools::twrootdir/$TESTS{"1-getsSmaller"}{file} -> +s;
$twtools::twcwd/$twtools::twrootdir/$TESTS{"2-okayToGrow"}{file}  -> +l;

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

  my $twpassed = 1;

  twtools::logStatus("*** Beginning $description\n");
  printf("%-30s", "-- $description");

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
