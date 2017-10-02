
use twtools;

package tripwire;

######################################################################
# One time module initialization goes in here...
#
BEGIN {
    $description = "tripwire options test";
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


    twtools::logStatus(`$twtools::twrootdir/bin/tripwire`);
    if ( $? != 2048 ) {
      twtools::logStatus("no-args tripwire failed, error = $?\n");
      $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/tripwire --version`);
    if ( $? != 0 ) {
        twtools::logStatus("tripwire --version failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/tripwire --help`);
    if ( $? != 2048 ) {
        twtools::logStatus("tripwire --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/tripwire --help all`);
    if ( $? != 2048 ) {
        twtools::logStatus("tripwire --help all failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/tripwire --help i c`);
    if ( $? != 2048 ) {
        twtools::logStatus("tripwire --help i c failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/tripwire --asdf 2>&1`);
    if ( $? != 2048 ) {
        twtools::logStatus("tripwire --asdf failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/tripwire --asdf --help 2>&1`);
    if ( $? != 2048 ) {
        twtools::logStatus("tripwire --asdf --help\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/tripwire -m 2>&1`);
    if ( $? != 2048 ) {
        twtools::logStatus("tripwire -m\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/tripwire -m Z 2>&1`);
    if ( $? != 2048 ) {
        twtools::logStatus("tripwire -m Z failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/tripwire -m -m 2>&1`);
    if ( $? != 2048 ) {
        twtools::logStatus("tripwire -m -m failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/tripwire -m i -c nope.cfg 2>&1`);
    if ( $? != 2048 ) {
        twtools::logStatus("tripwire -m -i -c nope.cfg\n");
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
