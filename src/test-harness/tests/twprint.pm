
use twtools;

package twprint;

######################################################################
# One time module initialization goes in here...
#
BEGIN {

    %TESTS = (

              "twprintInit" => {

                  changeFunc  => \&twtools::MakeBigger,
                  createFunc  => \&twtools::CreateFile,
                  file        => "printme.txt",
                  perms       => "a+w",
                  contents    => "testing",
                  violations  => "V:1 S:0 A:0 R:0 C:1"
                  },
              );


    $description = "twprint test";
}


######################################################################
# Return the policy file text for this test...
#
sub getPolicyFileString {

   return <<EOT;

$twtools::twcwd/$twtools::twrootdir/$TESTS{"twprintInit"}{file}  -> +S;


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
    # Run a check first, to set up for twprint exercises
    #
    $twpassed = twtools::RunIntegrityTests(%TESTS);



    #########################################################
    #
    # Now run thru the valid report levels & verify return codes
    #  TODO: look for expected result in various report levels

    twtools::RunReport({ report_level => 0 });
    if ( $? != 0 ) {
        twtools::logStatus("level 0 report failed\n");
        $twpassed = 0;
    }
    
    twtools::RunReport({ report_level => 1 });
    if ( $? != 0 ) {
        twtools::logStatus("level 1 report failed\n");
        $twpassed = 0;
    }

    twtools::RunReport({ report_level => 2 });
    if ( $? != 0 ) {
        twtools::logStatus("level 2 report failed\n");
        $twpassed = 0;
    }

    twtools::RunReport({ report_level => 3 });
    if ( $? != 0 ) {
        twtools::logStatus("level 3 report failed\n");
        $twpassed = 0;
    }

    twtools::RunReport({ report_level => 4 });
    if ( $? != 0 ) {
        twtools::logStatus("level 4 report failed\n");
        $twpassed = 0;
    }

    twtools::RunReport({ report_level => "4 --verbose --hexadecimal" });
    if ( $? != 0 ) {
        twtools::logStatus("level 4 report (verbose, hex) failed\n");
        $twpassed = 0;
    }

    twtools::RunReport({ report_level => "4 --silent" });
    if ( $? != 0 ) {
        twtools::logStatus("level 4 report (silent) failed\n");
        $twpassed = 0;
    }

    twtools::RunReport({ report_object_list =>  "$twtools::twrootdir/printme.txt" });
    if ( $? != 0 ) {
        twtools::logStatus("print-report with an object list failedn");
        $twpassed = 0;
    }


    #########################################################
    #
    # Now some failure cases, to verify they fail
    #
    twtools::RunReport({ report_level => 5 });
    if ( $? != 256 ) {
        twtools::logStatus("nonexistent level 5 report failed, result = $?\n");
        $twpassed = 0;
    }

    twtools::RunReport({ report_level => "asdf" });
    if ( $? != 256 ) {
        twtools::logStatus("nonexistent level 'asdf' report failed, result = $?\n");
        $twpassed = 0;
    }

    twtools::RunReport({ report_level => "0 --help" });
    if ( $? != 256 ) {
        twtools::logStatus("print-report help mode failed, result = $?\n");
        $twpassed = 0;
    }

    twtools::RunReport({ report_level => "4 --verbose --silent" });
    if ( $? != 256 ) {
        twtools::logStatus("print-report verbose vs silent failed, result = $?\n");
        $twpassed = 0;
    }

    #########################################################
    #
    # Now try db printing
    #
    twtools::RunDbPrint();
    if ( $? != 0 ) {
        twtools::logStatus("db print failed\n");
        $twpassed = 0;
    }

    twtools::RunDbPrint({ db_object_list => "$twtools::twrootdir/printme.txt" });
    if ( $? != 0 ) {
        twtools::logStatus("db print with an object failed\n");
        $twpassed = 0;
    }

    twtools::RunDbPrint({ db_object_list => "$twtools::twrootdir/nonexistent.vbs" });
    if ( $? != 0 ) {
        twtools::logStatus("db print with nonexistent object failed\n");
        $twpassed = 0;
    }

    twtools::RunDbPrint({ db_print_level => 0 });
    if ( $? != 0 ) {
        twtools::logStatus("db print level 0 failed\n");
        $twpassed = 0;
    }

    twtools::RunDbPrint({ db_print_level => 1 });
    if ( $? != 0 ) {
        twtools::logStatus("db print level 1 failed\n");
        $twpassed = 0;
    }

    twtools::RunDbPrint({ db_print_level => 2 });
    if ( $? != 0 ) {
        twtools::logStatus("db print level 2 failed\n");
        $twpassed = 0;
    }


    #########################################################
    #
    # Now try misc help & version options
    #
    twtools::logStatus(`$twtools::twrootdir/bin/twprint 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint, no args failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint --asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint --asdf failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint -?`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint -? failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint --help all`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint --help all failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint --help d r`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint --help d r failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint --help asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint --help asdf failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint --asdf --help 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint --help --asdf failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint --print-dbfile --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint --help --print-dbfile failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint --print-report --help `);
    if ( $? != 256 ) {
        twtools::logStatus("twprint --help --print-reportfile failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint -m Z 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint -m Z failed\n");
        $twpassed = 0;
    }


    twtools::logStatus(`$twtools::twrootdir/bin/twprint -m 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint -m failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint -m -m 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twprint -m -m failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twprint --version`);
    if ( $? != 0 ) {
        twtools::logStatus("twprint --version failed\n");
        $twpassed = 0;
    }


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
