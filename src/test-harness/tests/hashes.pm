
use twtools;

package hashes;

######################################################################
# One time module initialization goes in here...
#
BEGIN {

    %TESTS = (

              "0-createFile" => {

                  changeFunc  => \&twtools::MakeBigger,
                  createFunc  => \&twtools::CreateFile,
                  file        => "crc32.txt",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },

              "1-createFile" => {

                  changeFunc  => \&twtools::MakeBigger,
                  createFunc  => \&twtools::CreateFile,
                  file        => "md5.txt",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },

              "2-createFile" => {

                  changeFunc  => \&twtools::MakeBigger,
                  createFunc  => \&twtools::CreateFile,
                  file        => "sha.txt",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },

              "3-createFile" => {

                  changeFunc  => \&twtools::MakeSmaller,
                  createFunc  => \&twtools::CreateFile,
                  file        => "haval.txt",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },

              "4-createFile" => {

                  # just a little trickery here -- the changeFunc
                  # re-creates the file with the same contents,
                  # so the hashes shouldn't change, although things
                  # like timestamps will...
                  #
                  changeFunc  => \&twtools::CreateFile,
                  createFunc  => \&twtools::CreateFile,
                  file        => "cmsh.txt",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:0 S:0 A:0 R:0 C:0"
                  },


              );


    $description = "hashes (CMSH) test";
}


######################################################################
# Return the policy file text for this test...
#
sub getPolicyFileString {

   return <<EOT;

$twtools::twcwd/$twtools::twrootdir/$TESTS{"0-createFile"}{file}  -> +C;
$twtools::twcwd/$twtools::twrootdir/$TESTS{"1-createFile"}{file}  -> +M;
$twtools::twcwd/$twtools::twrootdir/$TESTS{"2-createFile"}{file}  -> +S;
$twtools::twcwd/$twtools::twrootdir/$TESTS{"3-createFile"}{file}  -> +H;
$twtools::twcwd/$twtools::twrootdir/$TESTS{"4-createFile"}{file}  -> +CMSH;

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
