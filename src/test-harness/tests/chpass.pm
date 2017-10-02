
use twtools;

package chpass;

######################################################################
# One time module initialization goes in here...
#
BEGIN {
    $description = "change passphrases test";
}

######################################################################
#
# Initialize, get ready to run this test...
#
sub initialize() {
    
}


######################################################################
#
# Run the test.
#
sub run() {

  my $twpassed = 1;
  my $newpass = "password";

  twtools::logStatus("*** Beginning $description\n");
  printf("%-30s", "-- $description");


  twtools::logStatus(`$twtools::twrootdir/bin/twadmin --change-passphrases -S $twtools::twrootdir/$twtools::twsitekeyloc --site-passphrase $newpass --site-passphrase-old $twtools::twsitepass`);
  if ( $? != 0 ) {
      twtools::logStatus("first change site passphrase failed\n");
      $twpassed = 0;
  }

  twtools::logStatus(`$twtools::twrootdir/bin/twadmin --change-passphrases -S $twtools::twrootdir/$twtools::twsitekeyloc --site-passphrase $twtools::twsitepass --site-passphrase-old $newpass`);
  if ( $? != 0 ) {
      twtools::logStatus("second change site passphrase failed\n");
      $twpassed = 0;
  }

  twtools::logStatus(`$twtools::twrootdir/bin/twadmin --change-passphrases -L $twtools::twrootdir/$twtools::twlocalkeyloc --local-passphrase $newpass --local-passphrase-old $twtools::twlocalpass`);
  if ( $? != 0 ) {
      twtools::logStatus("first change local passphrase failed\n");
      $twpassed = 0;
  }

  twtools::logStatus(`$twtools::twrootdir/bin/twadmin --change-passphrases -L $twtools::twrootdir/$twtools::twlocalkeyloc --local-passphrase $twtools::twlocalpass --local-passphrase-old $newpass`);
  if ( $? != 0 ) {
      twtools::logStatus("second change local passphrase failed\n");
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
# One time module cleanup goes in here...
#
END {
}

1;
