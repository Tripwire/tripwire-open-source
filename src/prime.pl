#!/usr/bin/perl

    %twcfgdirs = (

        ROOT                         => '',
        POLFILE                      => 'tw.pol',
        DBFILE                       => 'database.twd',
        REPORTFILE                   => 'report.twr',
        SITEKEYFILE                  => 'site.key',
        LOCALKEYFILE                 => 'local.key',
        TEMPDIRECTORY                => 'temp/'
                  
    );

    %twcfgprops = (
    
        EDITOR                       => '/usr/bin/vi',
        LATEPROMPTING                => 'false',
        LOOSEDIRECTORYCHECKING       => 'false',
        MAILNOVIOLATIONS             => 'true',
        EMAILREPORTLEVEL             => '3',
        REPORTLEVEL                  => '3',
        MAILMETHOD                   => 'SENDMAIL',
        SYSLOGREPORTING              => 'false',
        MAILPROGRAM                  => '/usr/lib/sendmail -oi -t'
            
        );


if (scalar(@ARGV) == 0) {

  print "usage: prime rootdir bindir\n";
  exit;
}
else {

  $twcwd = `pwd`;
  chomp($twcwd);
  chop($twcwd) if $twcwd =~ /\/$/;

  $twrootdir = shift @ARGV;
  $bindir    = shift @ARGV;

  CreateDefaultConfig();
  GenerateKeys();
  SignConfigFile();

  system("cd $twrootdir; ln -s ../$bindir/tripwire") if -e "$bindir/tripwire";
  system("cd $twrootdir; ln -s ../$bindir/twadmin") if -e "$bindir/twadmin";
  system("cd $twrootdir; ln -s ../$bindir/twprint") if -e "$bindir/twprint";
  system("cd $twrootdir; ln -s ../$bindir/siggen") if -e "$bindir/siggen";

}

exit;

######################################################################
# Create a default config file in the passed
# in directory, and create the appropriate
# directory structure to along with the config
# file...
#
sub CreateDefaultConfig {

    my ($key, $value);

    # make the root and bin directories if not there
    # already...
    #
    mkdir($twrootdir,0755) if !-e $twrootdir;

    open(CFG, ">$twrootdir/twcfg.txt") || die "couldn't open cfg file...";

    print "generating configuration file...\n";

    # Output the twcfg.txt file...
    #
    while (($key, $value) = each(%twcfgdirs)) {

        # cfg file must have full paths, so prepend working directory...
        #
        printf(CFG "%-28s= %s\n", $key, "$twcwd/$twrootdir/$value");

    }

    # Output the non directory options to the config file.
    #
    while (($key, $value) = each(%twcfgprops)) {

        printf(CFG "%-28s= %s\n", $key, $value);
    }
    

    close(CFG);

}

sub GenerateKeys {

    # Don't gen the keys if they are already sitting there...
    #
    #
    if (! (-e "$twrootdir/site.key")) {
        print "generating site key...\n";
        $junk = `$bindir/twadmin -m G -S $twrootdir/site.key -Q testing`;
    }

	$? && return 0;

    if (! (-e "$twrootdir/local.key")) {
        print "generating local key...\n";
        $junk = `$bindir/twadmin -m G -L $twrootdir/local.key -P testing`;
    }

	return ($? == 0);
}


sub SignConfigFile {

    if (!-e "$twrootdir/tw.cfg") {
        print "signing configuration file...\n";
        $junk = `$bindir/twadmin -m F -Q testing -c $twrootdir/tw.cfg -S $twrootdir/site.key $twrootdir/twcfg.txt`;
    }

	return ($? == 0);
}
