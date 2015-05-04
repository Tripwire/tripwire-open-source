
use twtools;

package md5sum;

######################################################################
# One time module initialization goes in here...
#
BEGIN {

    $description = "md5 hash check";
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


  # lets see if the system 'md5sum' agree's with siggen's md5 hash
  #
  my ($md5sum, undef) = split(/ /, `md5sum $twtools::twrootdir/test`);
  my $siggen = `$twtools::twrootdir/bin/siggen -h -t -M $twtools::twrootdir/test`;

  chomp $md5sum;
  chomp $siggen;

  twtools::logStatus("md5sum reports: $md5sum\n");
  twtools::logStatus("siggen reports: $siggen\n");

  $twpassed = $md5sum == $siggen;

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
