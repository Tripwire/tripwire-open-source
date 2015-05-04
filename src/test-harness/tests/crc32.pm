
use twtools;

package crc32;

######################################################################
# One time module initialization goes in here...
#
BEGIN {

    $description = "crc32 hash check";
}


######################################################################
#
# Initialize, get ready to run this test...
#
sub initialize() {

  twtools::CreateFile( { file => "test", contents => "deadbeef"x5000} );
}


######################################################################
#
# Run the test.
#
sub run() {

  my $twpassed = 1;

  twtools::logStatus("*** Beginning $description\n");
  printf("%-30s", "-- $description");


  # lets see if the system 'cksum' agree's with siggen's md5 hash
  #
  my ($crc32, undef) = split(/ /, `cksum $twtools::twrootdir/test`);
  my $siggen = `$twtools::twrootdir/bin/siggen -h -t -C $twtools::twrootdir/test`;

  chomp $md5sum;
  chomp $siggen;

  # cksum issues results in decimal, so get siggen's result in base10.
  $siggen = hex($siggen);

  twtools::logStatus(" cksum reports: $crc32\n");
  twtools::logStatus("siggen reports: $siggen\n");

  $twpassed = $crc32 == $siggen;

  #########################################################
  #
  # See if the tests all succeeded...
  #
  if ($twpassed) {
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
