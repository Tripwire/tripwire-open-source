
use twtools;

package email;

######################################################################
# One time module initialization goes in here...
#
BEGIN {
    $description = "email test";
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

  twtools::logStatus("*** Beginning $description\n");
  printf("%-30s", "-- $description");

  my($sending, undef, undef, $mailfrom, $mailto, $subject, undef, undef, undef, $body) = twtools::RunEmailTest();

  # Verify that various lines in the test email output look right,
  # including the RFC 2822-required CRLFs that should be in everything
  # except $sending, which isn't part of the actual email.
  #
  if ( !($sending =~ "Sending a test message to: elvis\@mars")) {
      twtools::logStatus("Unexpected sending line: $sending\n");
      $twpassed = 0;
  }

  if ( !($mailfrom =~ "taz\@cat")) {
      twtools::logStatus("Unexpected From: field: $mailfrom\n");
      $twpassed = 0;
  }

  if ( !($mailto =~ "To: elvis\@mars\r")) {
      twtools::logStatus("Unexpected To: field: $mailto\n");
      $twpassed = 0;
  }

  if ( !($subject =~ "Subject: Test email message from Tripwire\r")) {
      twtools::logStatus("Unexpected Subject field: $subject\n");
      $twpassed = 0;
  }

  if ( !($body =~ "If you receive this message, email notification from tripwire is working correctly.\r")) {
      twtools::logStatus("Unexpected message body: $body\n");
     $twpassed = 0;
  }

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
