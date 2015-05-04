###############################################################################
# buildnuminc.pl
#
# Open the file 'buildnum.h', search for the line '#define BUILD_NUM "DDD"' (where
# DDD is a decimal integer), increment DDD, re-save the "buildnum.h" with
# the new build number
#
###############################################################################

$filename = 'buildnum.h';
$textname = 'buildnum.txt';
$batname  = 'buildnum.bat';
$project  = $ENV{"PROPROJ"};

open (TEXT, "<$textname") || die "Unable to open text file containing current build number!\n"; 

while (<TEXT>) {
    $curver = $_;
}

print "$curver\n";

if ($curver !="") {
    $newbuildnum = $curver + 1;
};

$verstring = "$project.$newbuildnum";
print ("Incrementing Project Build Number to: $verstring\n");
close (TEXT);

open (HEADER, ">$filename");
print (HEADER "#define BUILD_NUM _T(\"$newbuildnum\")\n\n");
close (HEADER);

open (TEXT, ">$textname");
print (TEXT "$newbuildnum\n");
close (TEXT);

open (BAT,   ">$batname");
print (BAT "set BLDLBL=Build_$verstring\n");
print (BAT "set BLDNUM=$verstring\n");
close (BAT);
