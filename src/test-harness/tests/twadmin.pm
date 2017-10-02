
use twtools;

package twadmin;

######################################################################
# One time module initialization goes in here...
#
BEGIN {
    $description = "twadmin test";
}


######################################################################
# Return the policy file text for this test...
#
sub getPolicyFileString {

   return <<EOT;

$twtools::twcwd/$twtools::twrootdir/$TESTS{"twadminInit"}{file}  -> +S;


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

sub runTests() {


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
    # print-policy & print-config variants
    #

    twtools::PrintConfig();
    if ( $? != 0 ) {
        twtools::logStatus("base print-cfgfile failed, error = $?\n");
        $twpassed = 0;
    }

    twtools::PrintConfig(opts => "--verbose");
    if ( $? != 0 ) {
        twtools::logStatus("print-cfgfile --verbose failed, error = $?\n");
        $twpassed = 0;
    }

    twtools::PrintConfig(opts => "--silent");
    if ( $? != 0 ) {
        twtools::logStatus("print-cfgfile --silent failed\, error = $?n");
        $twpassed = 0;
    }

    twtools::PrintPolicy();
    if ( $? != 0 ) {
        twtools::logStatus("base print-polfile failed, error = $?\n");
        $twpassed = 0;
    }

    twtools::PrintPolicy(opts => "--verbose");
    if ( $? != 0 ) {
        twtools::logStatus("print-polfile --verbose failed, error = $?\n");
        $twpassed = 0;
    }

    twtools::PrintPolicy(opts => "--silent");
    if ( $? != 0 ) {
        twtools::logStatus("print-polfile --silent failed, error = $?\n");
        $twpassed = 0;
    }

    #########################################################
    #
    # Now try misc help & version options
    #
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin, no args failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --asdf failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin -?`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin -? failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --help all`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --help all failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --help f p`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --help d r failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --help asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --help asdf failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --asdf --help 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --help --asdf failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin -m 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin -m failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin -m Z 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin -m Z failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin -m -m 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin -m -m failed\n");
        $twpassed = 0;
    }

    #########################################################
    #
    # Various missing files
    #
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin -m P 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --create-polfile --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin -m P nope.txt 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --create-polfile --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin -m P -c nope.cfg nope.txt 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --create-polfile --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin -m P -S site.nope nope.txt 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --create-polfile --help failed\n");
        $twpassed = 0;
    }


    #########################################################
    #
    # Per-mode help & errors
    #

    # create-polfile
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --create-polfile --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --create-polfile --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --create-polfile --verbose --silent 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --help --create-polfile --verbose --silent failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --create-polfile --asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --create-polfile --asdf failed\n");
        $twpassed = 0;
    }


    # create-cfgfile
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --create-cfgfile --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin  --create-cfgfile --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --create-cfgfile --verbose --silent 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --create-cfgfile --verbose --silent failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --create-cfgfile --asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --create-cfgfile --asdf failed\n");
        $twpassed = 0;
    }


    # print-polfile
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --print-polfile --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --print-polfile --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --print-polfile --verbose --silent 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --print-polfile --verbose --silent failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --print-polfile --asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --print-polfile --asdf failed\n");
        $twpassed = 0;
    }


    # print-cfgfile
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --print-cfgfile --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --print-cfgfile --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --print-cfgfile --verbose --silent 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --print-cfgfile --verbose --silent failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --print-cfgfile --asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --print-cfgfile --asdf failed\n");
        $twpassed = 0;
    }


    # encrypt
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --encrypt --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --encrypt --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --encrypt --verbose --silent 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --encrypt --verbose --silent failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --encrypt --asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --encrypt --asdf failed\n");
        $twpassed = 0;
    }


    # remove-encryption
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --remove-encryption --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --remove-encryption --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --remove-encryption --verbose --silent 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --remove-encryption --verbose --silent failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --remove-encryption --asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --remove-encryption --asdf failed\n");
        $twpassed = 0;
    }


    # examine
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --examine --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --examine --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --examine --verbose --silent 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --examine --verbose --silent failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --examine --asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --examine --asdf failed\n");
        $twpassed = 0;
    }


    # generate-keys
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --generate-keys --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --generate-keys --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --generate-keys --verbose --silent 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --generate-keys --verbose --silent failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --generate-keys--asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --generate-keys--asdf failed\n");
        $twpassed = 0;
    }

    # change-passphrases
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --change-passphrases --help`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --change-passphrases --help failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --change-passphrases --verbose --silent 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --change-passphrases --verbose --silent` failed\n");
        $twpassed = 0;
    }

    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --change-passphrases --asdf 2>&1`);
    if ( $? != 256 ) {
        twtools::logStatus("twadmin --change-passphrases --asdf failed\n");
        $twpassed = 0;
    }

    
    #########################################################
    #
    # Version
    #
    twtools::logStatus(`$twtools::twrootdir/bin/twadmin --version`);
    if ( $? != 0 ) {
        twtools::logStatus("twadmin --version failed\n");
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
