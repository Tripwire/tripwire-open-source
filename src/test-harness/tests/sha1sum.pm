
use twtools;

package sha1sum;

######################################################################
# One time module initialization goes in here...
#
BEGIN {

    $description = "sha1 hash check";
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


  # lets see if the system 'sha1sum' agree's with siggen's sha1 hash
  #
  my ($sha1sum, undef) = split(/ /, `sha1sum $twtools::twrootdir/test`);
  if ($sha1sum eq "") {
      twtools::logStatus("sha1sum not found, trying openssl instead\n");
      (undef, $sha1sum) = split(/=/, `openssl sha1 $twtools::twrootdir/test`);
  }
  if ($sha1sum eq "") {
      ++$twtools::twskippedtests;
      print "SKIPPED\n";
      return;
  }

  my $siggen = `$twtools::twrootdir/bin/siggen -h -t -S $twtools::twrootdir/test`;

  chomp $sha1sum;
  chomp $siggen;
  $sha1sum =~ s/^\s+|\s+$//g;
  $siggen =~ s/^\s+|\s+$//g;

  twtools::logStatus("sha1sum reports: $sha1sum\n");
  twtools::logStatus("siggen reports: $siggen\n");

  $twpassed = ($sha1sum eq $siggen);

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
