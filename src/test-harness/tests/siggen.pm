
use twtools;

package siggen;

######################################################################
# One time module initialization goes in here...
#
BEGIN {
    $description = "siggen test";
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


    twtools::logStatus(`ps > $twtools::twrootdir/hashme.txt`);
    if ( $? != 0 ) {
        twtools::logStatus("test file creation failed\n");
        $twpassed = 0;
    }

    # SkyOS doesn't have FIFOs, so this won't work
    if ( $^O ne "skyos") {
        twtools::logStatus(`mkfifo $twtools::twrootdir/donthashme.fifo`);
        if ( $? != 0 ) {
            twtools::logStatus("test fifo creation failed\n");
            $twpassed = 0;
        }
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen`);
    if ( $? != 256 ) {
      twtools::logStatus("no-args siggen failed\n");
      $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen -h -t -a`);
    if ( $? != 256 ) {
        twtools::logStatus("no-args siggen failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen --help`);
    if ( $? != 256 ) {
      twtools::logStatus("siggen --help failed\n");
      $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen --version`);
    if ( $? != 0 ) {
      twtools::logStatus("siggen --version failed\n");
      $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen --asdf 2>&1`);
    if ( $? != 256 ) {
      twtools::logStatus("siggen --asdf failed\n");
      $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen $twtools::twrootdir/hashme.txt`);
    if ( $? != 0 ) {
        twtools::logStatus("siggen hashme.txt failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen -a $twtools::twrootdir/hashme.txt`);
    if ( $? != 0 ) {
        twtools::logStatus("siggen -a hashme.txt failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen -h -t -a $twtools::twrootdir/hashme.txt`);
    if ( $? != 0 ) {
        twtools::logStatus("siggen -h -t -a hashme.txt failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen -a $twtools::twrootdir/donthashme.fifo`);
    if ( $? != 256 ) {
        twtools::logStatus("siggen -a donthashme.fifo failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen -h -t -a $twtools::twrootdir/donthashme.fifo`);
    if ( $? != 256 ) {
        twtools::logStatus("siggen -h -t -a donthashme.fifo failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen -a $twtools::twrootdir/no-existe.txt`);
    if ( $? != 256 ) {
        twtools::logStatus("siggen -a no-existe.txt failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/siggen -h -t -a $twtools::twrootdir/no-existe.txt`);
    if ( $? != 256 ) {
        twtools::logStatus("siggen -h -t -a no-existe.txt failed\n");
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
