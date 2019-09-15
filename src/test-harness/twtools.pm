package twtools;

######################################################################
#
## NOTE: Do not define any of these variables in terms of $twrootdir,
## as the -rootdir evaluation in twtwest happens after these variables
## are initialized...
##
#
BEGIN {

    $twrootdir     = "twtest";
    $twcwd         = `pwd`;
    $twemail       = undef;
	
    $twsitekeyloc    = "key/site.key";
    $twlocalkeyloc   = "key/local.key";
    $twpolicyloc     = "policy/twpol.txt";
    $twpolfileloc    = "policy/tw.pol";
    $twcfgloc        = "tw.cfg";

    $twsitepass      = "testing";
    $twlocalpass     = "testing";

    $twbinaries      = "../../../../bin";

    $twtotaltests    = 0;
    $twfailedtests   = 0;
    $twpassedtests   = 0;
    $twskippedtests  = 0;

    # get's setup in twtest...
    #
    $reportloc       = "";

    %twcfgdirs = (

        ROOT                         => '',
        POLFILE                      => $twpolfileloc,
        DBFILE                       => 'db/$(HOSTNAME).twd',
        REPORTFILE                   => 'report/$(HOSTNAME)-$(DATE).twr',
        SITEKEYFILE                  => $twsitekeyloc,
        LOCALKEYFILE                 => $twlocalkeyloc

    );

    %twcfgprops = (
    
        EDITOR                       => '/usr/bin/vi',
        LATEPROMPTING                => 'false',
        LOOSEDIRECTORYCHECKING       => 'false',
        MAILNOVIOLATIONS             => 'true',
        EMAILREPORTLEVEL             => '4',
        REPORTLEVEL                  => '4',
        MAILMETHOD                   => 'SENDMAIL',
        SYSLOGREPORTING              => 'true',
        MAILPROGRAM                  => 'cat',
        MAILFROMADDRESS              => 'taz@cat'
        );

}

sub logStatus {

    open(LOG, ">>$twrootdir/status.log") || die "couldn't open log file...";

#    use POSIX qw(strftime);
#    my ($now_string) = strftime "%a %b %e %H:%M:%S %Y", localtime;
#    print LOG "$now_string\n";

    print LOG @_;

    close(LOG);
}

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
    mkdir("$twrootdir/bin", 0755);

    open(CFG, ">$twrootdir/twcfg.txt") || die "couldn't open cfg file...";

    print "generating configuration file...\n" if $verbose;

    # Output the twcfg.txt file...
    #
    while (($key, $value) = each(%twcfgdirs)) {

        # cfg file must have full paths, so prepend working directory...
        #
        printf(CFG "%-28s= %s\n", $key, "$twcwd/$twrootdir/$value");

        # Make the directory structure to go along with it...
        # here we pull off everything up to the first /, and use
        # it as the name of the directory to create...
        #
        my ($dir, undef) = split(/\//, $value);
        mkdir("$twrootdir/$dir", 0755) if !(-e "$twrootdir/$dir");
    }

    # Output the non directory options to the config file.
    #
    while (($key, $value) = each(%twcfgprops)) {

        printf(CFG "%-28s= %s\n", $key, $value);
    }
    

    close(CFG);


    # Symlink the exe's into our test structure...
    #
    system("cd $twrootdir/bin && ln -fs $twbinaries/twadmin twadmin");
    system("cd $twrootdir/bin && ln -fs $twbinaries/tripwire tripwire");
    system("cd $twrootdir/bin && ln -fs $twbinaries/siggen siggen");
    system("cd $twrootdir/bin && ln -fs $twbinaries/twprint twprint");
    
}


######################################################################
# Generate the local and site keys if they are not already there...
#
sub GenerateKeys {

    # Don't gen the keys if they are already sitting there...
    #
    #
    if (! (-e "$twrootdir/$twsitekeyloc")) {
        print "generating site key...\n" if $verbose;
        logStatus(`$twrootdir/bin/twadmin -m G -S $twrootdir/$twsitekeyloc -Q $twsitepass`);
    }

	$? && return 0;

    if (! (-e "$twrootdir/$twlocalkeyloc")) {
        print "generating local key...\n" if $verbose;
        logStatus(`$twrootdir/bin/twadmin -m G -L $twrootdir/$twlocalkeyloc -P $twlocalpass`);
    }

	return ($? == 0);
}


######################################################################
# Sign the configuration file...
#
sub SignConfigFile {

    if (!-e "$twrootdir/$twcfgloc") {
        print "signing configuration file...\n" if $verbose;
        logStatus(`$twrootdir/bin/twadmin -m F -Q $twsitepass -c $twrootdir/$twcfgloc -S $twrootdir/$twsitekeyloc $twrootdir/twcfg.txt`);
    }

	return ($? == 0);
}


######################################################################
# Examine encryption
#
sub ExamineEncryption {

    my ($filename) = @_;

    logStatus(`$twrootdir/bin/twadmin -m e -c $twrootdir/$twcfgloc $filename`);

    return ($? == 0);
}


######################################################################
# Add encryption
#
sub AddEncryption {

    my ($filename) = @_;
    logStatus "addding crypt to file...\n";
    logStatus(`$twrootdir/bin/twadmin -m E -c $twrootdir/$twcfgloc -P $twlocalpass -Q $twsitepass $filename 2>&1`);

    return ($? == 0);
}


######################################################################
# Remove encryption
#

sub RemoveEncryption {

    my ($filename) = @_;

    logStatus "removing crypto from file...\n";
    logStatus(`$twrootdir/bin/twadmin -m R -c $twrootdir/$twcfgloc -P $twlocalpass -Q $twsitepass $filename 2>&1`);

    return ($? == 0);
}


######################################################################
# Print polfile
#

sub PrintPolicy {

    my (%params) = %{$_[0]};
    logStatus "printing policy file...\n";

    my (@out) = `$twrootdir/bin/twadmin -m p -c $twrootdir/$twcfgloc -p $twrootdir/$twpolfileloc -S $twrootdir/$twsitekeyloc $params{opts} 2>&1`;

    my ($result) = $?;

    logStatus(@out);

    return $result;
}

######################################################################
# Print polfile
#

sub PrintConfig {

    my (%params) = %{$_[0]};

    logStatus "printing config file...\n";
    my (@out) =  `$twrootdir/bin/twadmin -m f -c $twrootdir/$twcfgloc $params{opts} 2>&1`;

    my ($result) = $?;

    logStatus(@out);

    return $result;
}


######################################################################
# Write policy text to disk... Note the contents
# of the policy file are passed in as '$twstr'.
#
sub WritePolicyFile {

    my ($twstr) = @_;

    open(FH, ">$twrootdir/$twpolicyloc") || warn $!;
    print FH $twstr;
    close(FH);
}


######################################################################
# Generate and sign the policy file... Note the contents
# of the policy file are passed in as '$twstr'.
#
sub GeneratePolicyFile {

    my ($twstr) = @_;

    WritePolicyFile($twstr);

    print "generating policy file...\n" if $verbose;

    my (@out) = `$twrootdir/bin/twadmin -m P -c $twrootdir/$twcfgloc -Q $twsitepass -p $twrootdir/$twpolfileloc $twrootdir/$twpolicyloc 2>&1`;

    my ($result) = $?;

    logStatus(@out);

    return $result;
}


######################################################################
# Generate and sign the policy file... Note the contents
# of the policy file are passed in as '$twstr'.
#
sub CreatePolicy {

    my (%params) = %{$_[0]};

    $params{policy_text} = "$twrootdir/$twpolicyloc" if( ! defined($params{policy_text}) );

    print "generating policy file...\n" if $verbose;

    my (@out) = `$twrootdir/bin/twadmin -m P -c $twrootdir/$twcfgloc -Q $twsitepass -p $twrootdir/$twpolfileloc $params{policy_text} 2>&1`;

    my ($result) = $?;

    logStatus(@out);

    return $result;
}


######################################################################
# Run tripwire to initialize the database...
#
sub InitializeDatabase {

    my ($twmsg) = @_;

    print "initializing database for '$twmsg' test...\n" if $verbose;
    my (@out) = `$twrootdir/bin/tripwire -m i -P $twsitepass -p $twrootdir/$twpolfileloc -c $twrootdir/$twcfgloc 2>&1`;

    my ($result) = $?;

    logStatus(@out);

    return $result;
}


######################################################################
# Run tripwire to update the database...
#
sub UpdateDatabase {

    my (%params) = %{$_[0]};
    $params{report}      = $reportloc if( ! defined($params{report}) );
    $params{secure_mode} = "low" if( ! defined($params{secure_mode}) );
    
    print "updating database for '$twmsg' test...\n" if $verbose;
    my (@out) = `$twrootdir/bin/tripwire -m u -a -P $twsitepass -Z $params{secure_mode} -p $twrootdir/$twpolfileloc -c $twrootdir/$twcfgloc -r $params{report} 2>&1`;

    my ($result) = $?;

    logStatus(@out);

    return $result;
}

######################################################################
# Run tripwire to update the policy...
#
sub UpdatePolicy {

    my (%params) = %{$_[0]};
    $params{secure_mode} = "low" if( ! defined($params{secure_mode}) );

    print "updating policy for '$twmsg' test...\n" if $verbose;
    logStatus(`$twrootdir/bin/tripwire -m p -P $twsitepass -Q $twlocalpass -Z $params{secure_mode} -p $twrootdir/$twpolfileloc -c $twrootdir/$twcfgloc $twrootdir/$twpolicyloc 2>&1`);

    return ($? == 0);
}


######################################################################
# Use twprint to get a report of specified level (default 0) and return
# that.
#
sub RunReport {

    my (%params) = %{$_[0]};
    my ($reportobjects);

    $params{report} = $reportloc if( ! defined($params{report}) );
    $params{report_level} = 0 if( ! defined($params{report_level}) );
    $params{report_object_list} = "" if( ! defined($params{report_object_list}) );

    my (@out) =  `$twrootdir/bin/twprint -m r -c $twrootdir/$twcfgloc -t $params{report_level} -r $params{report} $params{report_object_list} 2>&1`;

    logStatus(@out);

    return @out;

}

######################################################################
# Use twprint to get a report level 0 report and return
# that.
#
sub RunDbPrint {

    my (%params) = %{$_[0]};
 
    $params{db_object_list} = "" if( ! defined($params{db_object_list}) );
    $params{db_print_level} = "2" if( ! defined($params{db_print_level}) );

    my (@out) =  `$twrootdir/bin/twprint -m d -c $twrootdir/$twcfgloc -t $params{db_print_level} $params{db_object_list} 2>&1`;

    logStatus(@out);

    return @out;
}


######################################################################
# Run an email test (configured with mailmethod=sendmail) & capture output
#
sub RunEmailTest {

    my (@out) =  `$twrootdir/bin/tripwire --test -c $twrootdir/$twcfgloc --email elvis\@mars`;

    logStatus(@out);

    return @out;
}


######################################################################
# Run tripwire to do an integrity check on
# the test data
#
sub RunIntegrityCheck {

    my (%params) = %{$_[0]};
	$params{report} = $reportloc if( ! defined($params{report}) );
	$params{trailing_opts} = "" if( ! defined($params{trailing_opts}) );

    print("running integrity check for test '$twmsg'...\n") if $verbose;
    logStatus(`$twrootdir/bin/tripwire -m c -r $params{report} -p $twrootdir/$twpolfileloc -c $twrootdir/$twcfgloc $params{trailing_opts} 2>&1`);

    return ($? & 8);
}


######################################################################
# This function knows how to parse a level 0 report as 
# produced by twprint. It pulls out the count of each type
# of violation and returns them as a list...
#
sub AnalyzeReport {

    # Sheers off the first line to get to
    # the actual status line...
    #
    my (undef, $twstatus) = @_;

    # split the report summary line into it's fields
    #
    my (undef, undef, undef, $violations, $severity, $added, $removed, $changed) =
        split(/ /, $twstatus);

    # Split out the count for each type of
    # violation.
    #
    (undef, $violations) = split(/:/, $violations);
    (undef, $added)      = split(/:/, $added);
    (undef, $removed)    = split(/:/, $removed);
    (undef, $changed)    = split(/:/, $changed);

    # return the counts.
    #
    return ($violations, $added, $removed, $changed);
}


######################################################################
# Make the file bigger...
#
sub MakeBigger(%) {

    my (%params) = %{$_[0]};

    my ($twfile);

    if (defined $params{dir}) {
        $twfile = "$twrootdir/$params{dir}/$params{file}";
    }
    else {
        $twfile = "$twrootdir/$params{file}";
    }
        
    system("echo more >> $twfile");
}



######################################################################
# Make the file smaller, i.e. 0 byes ;)
#
sub MakeSmaller(%) {

    my (%params) = %{$_[0]};

    my ($twfile);

    if (defined $params{dir}) {
        $twfile = "$twrootdir/$params{dir}/$params{file}";
    }
    else {
        $twfile = "$twrootdir/$params{file}";
    }

    system("echo -n > $twfile");
}



######################################################################
# Access (read) the file.
#
sub Access(%) {

    my (%params) = %{$_[0]};

    my ($twfile);

    if (defined $params{dir}) {
        $twfile = "$twrootdir/$params{dir}/$params{file}";
    }
    else {
        $twfile = "$twrootdir/$params{file}";
    }

    # Here, we simply access (read) the file, thus
    # changing the last accessed timestamp.
    #
    system("cat $twfile > /dev/null");

}


######################################################################
#
# Chmod the file -- irrelevant what we chmod it
# to, we just want to affect a change.
#
sub Chmod(%) {

    my (%params) = %{$_[0]};

    my ($twfile);

    if (defined $params{dir}) {
        $twfile = "$twrootdir/$params{dir}/$params{file}";
    }
    else {
        $twfile = "$twrootdir/$params{file}";
    }
    
    # Fake a change to the file which should be caught...
    #
    system("chmod a+w $twfile");

    $? && die "chmod failed for $twfile\n";
}


######################################################################
#
# Create a symlink between target and file...
#
sub CreateSymLink(%) {

    my (%params) = %{$_[0]};

    # Just create a symlink...
    #
    system("cd $twrootdir && ln -fs $params{target} $params{file}");

    $? && die "ln -s failed for $params{target} -> $params{file}\n";
}



######################################################################
#
# Create a hard link between target and file...
#
sub CreateHardLink(%) {

    my (%params) = %{$_[0]};

    # Just create a hard link...
    #
    system("cd $twrootdir && ln -f $params{target} $params{file}");

    $? && die "ln failed for $params{target} -> $params{file}\n";
}


######################################################################
# CreateFile -- create a file with the specified contents
#   
# input:  path     -- path to the file; relative to $root
#         contents -- string to put in the file
#
sub CreateFile(%)
{
   my (%params) = %{$_[0]};

	my ($path)     = $params{'file'};
   my ($contents) = $params{'contents'};
	
    my ($twfile);

    if (defined $params{dir}) {
        $twfile = "$twrootdir/$params{dir}/$path";
    }
    else {
        $twfile = "$twrootdir/$path";
    }

	system( "echo $contents > $twfile" );

	$? && die "Create file failed for $twfile\n";
}

######################################################################
# RemoveFile -- removes the named file
#   
sub RemoveFile(%)
{
   my (%params) = %{$_[0]};
   my ($path)   = $params{'file'};
	
	if( -e "$twrootdir/$path" )
	{
		system( "rm -f $twrootdir/$path" );
	}
	
	$? && die "Remove file failed for $twrootdir/$path\n";
}


######################################################################
# CreateDir -- create a directory
#
sub CreateDir(%)
{
	my (%params) = %{$_[0]};

   my ($dir) = $params{'dir'};

	# NOTE: mkdir fails if it is already a directory!
	#
	if( ! -d "$twrootdir/$dir" )
	{
		system( "rm -f $twrootdir/$dir" );
		system( "mkdir -p $twrootdir/$dir" );
	
		$? && die "Mkdir failed for $twrootdir/$dir\n";
	}
}

######################################################################
# MoveFile -- move a file from one place to another
#             NOTE: file names are relative to $root
#   
# input:  old_name -- name of file to move
#         new_name -- where it should be moved to
#
sub MoveFile
{
    my (%params) = %{$_[0]};
    my ($file)   = $params{'file'};
    my ($target) = $params{'target'};
	
    system( "mv $twrootdir/$file $twrootdir/$target" );

    $? && die "mv $twrootdir/$file $twrootdir/$target failed!\n";
}

######################################################################
# This function iterates through the sub tests
# performing the requested operations and verifying
# that the expected and actual violation counts are
# the same.
#
sub RunIntegrityTests(\%) {

    my %tests = %{$_[0]};
    my ($violations, $added, $removed, $changed) = (0,0,0,0);


    #########################################################
    #
    # Here we tally up the count of each violation which
    # should occur
    #
    map { 

        my ($v,undef,$a,$r,$c) = split(/ /, $tests{$_}{violations}); 

        (undef, $v) = split(/:/, $v);
        (undef, $a) = split(/:/, $a);
        (undef, $r) = split(/:/, $r);
        (undef, $c) = split(/:/, $c);

        $violations += $v;
        $added      += $a;
        $removed    += $r;
        $changed    += $c;

        if ($violations != ($added + $removed + $changed)) {

            die "V: does not == A: + R: + C: for $_\n\n";
        }

    } sort keys %tests;


    #########################################################
    #
    # Call the file create function for each sub-test...
    #
    map { 

        if (defined $tests{$_}{createFunc}) {

            $tests{$_}{createFunc}->($tests{$_});
        }

    } sort keys %tests;

    #########################################################
    #
    # Now that the files are there, create the initial database...
    #
    InitializeDatabase("");


    #########################################################
    #
    # Call the file change function for each sub-test...
    #
    map { 

        if (defined $tests{$_}{changeFunc}) {

            $tests{$_}{changeFunc}->($tests{$_});
        }

    } sort keys %tests;


    #########################################################
    #
    # Run the integrity check...
    #
    RunIntegrityCheck();

    
    #########################################################
    #
    # Analyze the report generated by the integrity check...
    #
    my @twresults = RunReport( { dir => "this"} );
    my ($actualViolations, $actualAdded, $actualRemoved, $actualChanged) = AnalyzeReport(@twresults);

    logStatus("Total expected:   $violations | Total observed:   $actualViolations\n");
    logStatus("Expected added:   $added | Observed added:   $actualAdded\n");
    logStatus("Expected removed: $removed | Observed removed: $actualRemoved\n");
    logStatus("Expected changed: $changed | Observed changed: $actualChanged\n");
    
    
    # Actual and expected violations should match if everything went as 
    # planned.
    #
    return ($actualViolations == $violations && $actualAdded   == $added &&
            $actualRemoved    == $removed    && $actualChanged == $changed);
}


END {
}

1;
