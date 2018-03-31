#!/bin/sh

#########################################################
#########################################################
##
## Tripwire(R) 2.4 Open Source install script
##
#########################################################
#########################################################

##=======================================================
## Setup
##=======================================================

##-------------------------------------------------------
## Sanity checks, mostly stolen (with permission)
## from Larry Wall's metaconfig.
##-------------------------------------------------------

PATH=".:/bin:/usr/bin:/usr/local/bin:$PATH"
export PATH || (echo 'You must use sh to run this script'; kill $$)
if [ ! -t 0 ] ; then
	echo "Say 'sh install.sh', not 'sh < install.sh'"
	exit 1
fi

##-------------------------------------------------------
## The usage message.
##-------------------------------------------------------

USAGE="install.sh [<configfile>] [-n] [-f] [-s <sitepassphrase>] [-l <localpassphrase>] [-d <installdir>]"

##-------------------------------------------------------
## Figure out how to do an echo without newline.
##-------------------------------------------------------

if [ "`echo -n`" = "-n" ] ; then
	n=""
	c="\c"
else
	n=" -n"
	c=""
fi

##-------------------------------------------------------
## Can't live without sed.
##-------------------------------------------------------

(echo 'abc' | sed 's/abc/def/') 2> /dev/null 1>&2
if [ $? -ne 0 ]; then
    echo "sed command not found -- aborting install."
	exit 1
fi

##-------------------------------------------------------
## Does this system have a copy of awk we can use?
##-------------------------------------------------------

AWK=""
awknames="awk nawk gawk"
for p in $awknames; do
	($p '{ exit 0 }' < /dev/null) 2> /dev/null 1>&2
	if [ $? -eq 0 ]; then
		AWK=$p
		break
	fi
done

##-------------------------------------------------------
## Does this system have a pager that we can use?
## Use cat if desperate.
##-------------------------------------------------------

MORE="cat"
morenames="less more most pg cat"
for p in $morenames; do
    pagerpath=`command -v $p`

    if [ -z $pagerpath ]; then
        continue
    fi

    if [ -x $pagerpath ]; then
        MORE=$pagerpath
        break
    fi
done

##-------------------------------------------------------
## Does this system have a copy of uname we can use?
##-------------------------------------------------------

UNAME=""
(uname -s -r -v) 2> /dev/null 1>&2
if [ $? -eq 0 ]; then
	UNAME=uname
fi

##=======================================================
## Command line
##=======================================================

##-------------------------------------------------------
## Miscellaneous configuration parameters.
##-------------------------------------------------------

# set a few location variables if caller didn't pass them to us
prefix="${prefix:=/usr/local}"
sysconfdir="${sysconfdir:=/usr/local/etc}"
path_to_vi="${path_to_vi:=/usr/bin/vi}"

# License File name
TWLICENSEFILE="COPYING"

# Default Tripwire configuration file.
TW_CONFIG_FILE="tw.cfg"

# Name of initial cleartext Tripwire config file.
CLR_CONFIG_FILE="twcfg.txt"

# Name of initial cleartext Tripwire policy file.
CLR_POLICY_FILE="twpol.txt"

# Default installation script configuration file.
INSTALL_CONFIG_FILE="./installer/install.cfg"

# Relative path to dir that contains the pkg files.
PKG_DIR="pkg"

# Names of the package files.
# See also copy-files section below.
BIN_PKG="bin.pkg"
POL_PKG="policy.pkg"
MAN_PKG="man.pkg"

# Starting directory.
START_DIR=`pwd`

# Site passphrase.
TW_SITE_PASS=""

# Local passphrase.
TW_LOCAL_PASS=""

# If clobber==true, overwrite files; if false, do not overwrite files.
CLOBBER="false"

# If prompt==true, ask for confirmation before continuing with install.
PROMPT="true"

# Guess where the toplevel for the distribution is.
# A bad guess is the current directory
TAR_DIR=${TAR_DIR:-${START_DIR}}

OS=`uname -s`
POLICYSRC="twpol-${OS:=GENERIC}.txt"
if [ ! -r ${TAR_DIR}/policy/${POLICYSRC} ]; then
    OS=`uname -o`
    POLICYSRC="twpol-${OS:=GENERIC}.txt"
fi

if [ ! -r ${TAR_DIR}/policy/${POLICYSRC} ]; then
    POLICYSRC="twpol-GENERIC.txt"
fi


##-------------------------------------------------------
## Parse the command line.
##-------------------------------------------------------

while [ "x$1" != "x" ] ; do
    case "$1" in
	-n) PROMPT="false"; xCLOBBER="true" ;;
	-f) xCLOBBER="true" ;;
	-s) case "$2" in
	    "" | -*)
		echo "Error: missing sitepassphrase with -s option." 1>&2
		echo "$USAGE"
		exit 1 ;;
	    *) TW_SITE_PASS="$2"; shift ;;
	    esac ;;
	-l) case "$2" in
	    "" | -*)
		echo "Error: missing localpassphrase with -l option." 1>&2
		echo "$USAGE"
		exit 1 ;;
	    *) TW_LOCAL_PASS="$2"; shift ;;
	    esac ;;
        -d) case "$2" in
            "" | -*)
                echo "Error: missing install dir with -d option." 1>&2
                echo "$USAGE"
                exit 1 ;;
            *) prefix="$2"; sysconfdir="$2/bin"; shift ;;
            esac ;;
	-*) echo "Error: unknown argument $1" 1>&2
	    echo "$USAGE"
	    exit 1 ;;
	*) INSTALL_CONFIG_FILE="$1"
	    if [ $xgot ] ; then
		echo "Error: incorrect number of arguments" 1>&2
		echo "$USAGE"
		exit 1
	    fi
	    xgot="true"
	    ;;
    esac
    shift
done

##-------------------------------------------------------
## Print the sign-on banner here before the first
## non-error message is displayed.
##-------------------------------------------------------

cat << END_OF_TEXT

Installer program for:
Tripwire(R) 2.4 Open Source

Copyright (C) 1998-2017 Tripwire, Inc.
Tripwire is a registered trademark of Tripwire, Inc. All rights reserved.

END_OF_TEXT


##-------------------------------------------------------
## Print a message if the user aborts the install.
##-------------------------------------------------------

trap "echo ; echo 'Installation has been halted.' ; exit 1" 1 2 15

##-------------------------------------------------------
## The pathname variables we expect to find in the
## install.cfg file.
##-------------------------------------------------------

paths="TWBIN TWMAN TWPOLICY TWREPORT TWDB TWSITEKEYDIR TWLOCALKEYDIR"
path2="TWBIN TWPOLICY TWREPORT TWDB TWSITEKEYDIR TWLOCALKEYDIR"
path3="TWMAN TWDOCS"

##=======================================================
## User License Agreement
##=======================================================

##-------------------------------------------------------
## Where is the license agreement file?
##-------------------------------------------------------
EULA_PATH="$TWLICENSEFILE"

##-------------------------------------------------------
## If prompting is turned on, get acceptance from user before moving on.
##-------------------------------------------------------

if [ "$PROMPT" = "true" ] ; then
  echo
  echo "LICENSE AGREEMENT for Tripwire(R) 2.4 Open Source"
  echo
  echo "Please read the following license agreement.  You must accept the"
  echo "agreement to continue installing Tripwire."
  echo
  echo "Press ENTER to view the License Agreement."
  echo
  read inputstring

  # Show the user the license:
  ${PAGER:-${MORE}} $EULA_PATH

  # Now confirm that the user agrees to the license:
  echo
  echo 'Please type "accept" to indicate your acceptance of this'
        (echo $n "license agreement. [do not accept] " $c) 1>&2
	read ans
	case "$ans" in
		accept|Accept|ACCEPT) ;;
		*) echo "YOU DID NOT ACCEPT THE LICENSE AGREEMENT.  INSTALLATION TERMINATED."
			exit 1
			;;
	esac
fi

##-------------------------------------------------------
## Execute the installation configuration file
##-------------------------------------------------------

echo "Using configuration file $INSTALL_CONFIG_FILE"
echo
. $INSTALL_CONFIG_FILE

##=======================================================
## Process the configuration parameters.
##=======================================================

##-------------------------------------------------------
## Value on command line, if present, overrides value in
## config file.  Value must either be "true" or "false"
## exactly; if it's not the former, make it the latter.
##-------------------------------------------------------

CLOBBER=${xCLOBBER-$CLOBBER}
if [ ! "$CLOBBER" = "true" ] ; then
	CLOBBER="false"
fi

##-------------------------------------------------------
## If no prompting was selected, both site and local
## passphrases must be specified on the command line.
##-------------------------------------------------------

if [ "$PROMPT" = "false" ] ; then
	if [ -z "$TW_SITE_PASS" ] || [ -z "$TW_LOCAL_PASS" ] ; then
		echo "Error: You must specify site and local passphrase" 1>&2
		echo "if no prompting is chosen." 1>&2
		echo "$USAGE"
		exit 1
    fi
fi

##-------------------------------------------------------
## For each pathname variable:
## Make sure it's defined.
## Strip trailing slashes from each of the directory variables.
## Determine length of longest parameter name.
##-------------------------------------------------------

len=0
for i in $paths; do

# Is it defined?
	eval "test \"\$${i}\""
	if [ $? -ne 0 ] ; then
		echo "Error: configuration parameter \$$i undefined." 1>&2
		echo "There is an error in the configuration file ${INSTALL_CONFIG_FILE}." 1>&2
		exit 1
	fi

# Strip trailing slashes.
# Squash multiple internal slashes down to one.
	eval "xtmp=\$${i}"
	xtmp=`echo $xtmp | sed 's/\/*$//'`
	xtmp=`echo $xtmp | sed 's/\/\/*/\//g'`
	eval "${i}=\"$xtmp\""

# Does it start with a slash (i.e. is it an absolute pathname)?
	ytmp=`echo $xtmp | sed 's/^\///'`
	if [ "$xtmp" = "$ytmp" ] ; then
		echo "Error: \$$i is not an absolute pathname." 1>&2
		echo 'Relative pathnames may not be used.' 1>&2
		exit 1
	fi

# What is the length of the longest variable name?
	if [ "$AWK" != "" ] ; then
		xlen=`echo "${i}" | $AWK '{ print length }'`
		if [ $xlen -gt $len ] ; then
			len=$xlen
		fi
	fi
done

##-------------------------------------------------------
## Check Mailmethod for SMTP.  If SMTP ignore MAILPROGRAM.
## If SENDMAIL is specified, verify that the specified 
## mail program exists
##-------------------------------------------------------
echo "Checking for programs specified in install configuration file...."
echo

TWMAILMETHOD=${TWMAILMETHOD:-'SENDMAIL'}
if [ "$TWMAILMETHOD" = "SENDMAIL" ] ; then

	if [ -n "$TWMAILPROGRAM" ] ; then
        	echo "${TWMAILPROGRAM} exists.  Continuing installation."
        	echo
	else
	        echo ""
        	echo "*** No sendmail found. Edit install.cfg and set"
		echo "*** TWMAILPROGRAM to full path to sendmail, or"
		echo "*** change TWMAILMETHOD to SMTP and set SMTP"
		echo "*** settings appropriately."
		echo ""
        	exit 1
	fi
else
        echo "Using SMTP mail protocol."
	echo "MAILPROGRAM variable will be ignored."
        echo "Continuing installation."
        echo
        fi

##-------------------------------------------------------
## Verify that the specified editor program exists
##-------------------------------------------------------

# If user specified an editor in $path_to_vi or $TWEDITOR, try that first.
# $path_to_vi defaults to /usr/bin/vi, so we usually succeed here.
#
if [ -n ${TWEDITOR} ]; then
    TWEDITOR_PATH=`command -v $TWEDITOR`
fi

# If user's environment includes $EDITOR, try that next
if [ -n ${EDITOR} ] && [ -z ${TWEDITOR_PATH} ]; then
    TWEDITOR_PATH=`command -v $EDITOR`
fi

# Ok, now search path for vi
if [ -z ${TWEDITOR_PATH} ]; then
    TWEDITOR_PATH=`command -v vi`
fi

# Try vim in case there isn't a link named vi
if [ -z ${TWEDITOR_PATH} ]; then
    TWEDITOR_PATH=`command -v vim`
fi

# No vi/vim? See if nano is present
if [ -z ${TWEDITOR_PATH} ]; then
    TWEDITOR_PATH=`command -v nano`
fi

# No vi or nano? See if emacs is available
if [ -z ${TWEDITOR_PATH} ]; then
    TWEDITOR_PATH=`command -v emacs`
fi

if [ -n ${TWEDITOR_PATH} ]; then
    TWEDITOR=$TWEDITOR_PATH
fi

if [ -x ${TWEDITOR} ]; then
    echo "${TWEDITOR} exists.  Continuing installation."
    echo
else
    echo "${TWEDITOR} not found. Continuing, but your configuration may need to be edited after installation."
    echo
fi

##-------------------------------------------------------
## Where are the bin files?  Did they build okay?  Don't
## bother to install if they don't exist.
##-------------------------------------------------------

echo
echo "----------------------------------------------"
echo "Verifying existence of binaries..."
echo

BASE_DIR="./"
if [ ! -z "$BASE_DIR" ] ; then
	BIN_DIR="${BASE_DIR}bin"
fi

CWD=`pwd`
cd "$TAR_DIR"
TAR_DIR=`pwd`
cd "$BASE_DIR"
files="$BIN_DIR/siggen $BIN_DIR/tripwire $BIN_DIR/twprint $BIN_DIR/twadmin"
for i in $files; do
    if [ -s "$i" ] ; then
        echo "$i found"
    else
        echo "$i missing.  Build did not complete successfully."
        exit 1
    fi
done
cd "$CWD"

##-------------------------------------------------------
## Print the list of target directories.
##-------------------------------------------------------

echo
echo "This program will copy Tripwire files to the following directories:"
echo

for i in $paths; do
	if [ "$AWK" != "" ] ; then
		eval "echo \"${i}\"	| $AWK '{printf \"%${len}s: \", \$1}'"
		eval "echo \"\$${i}\""
	else
		eval "echo \"\$${i}\""
	fi
done

##-------------------------------------------------------
## Display value of clobber.
##-------------------------------------------------------

echo
echo "CLOBBER is $CLOBBER."

##-------------------------------------------------------
## Prompt to continue.
##-------------------------------------------------------

if [ "$PROMPT" = "true" ] ; then
	echo
	(echo $n "Continue with installation? [y/n] " $c) 1>&2
	read ans
	case "$ans" in
		[yY]*) ;;
		*) echo "Installation has been halted."
			exit 1;
			;;
	esac
fi

##=======================================================
## Create directories.
##=======================================================

echo
echo "----------------------------------------------"
echo "Creating directories..."
echo

##-------------------------------------------------------
## Create only directories that do not already exist.
## Change permissions only on directories we create.
## Exit if mkdir fails.
##-------------------------------------------------------

for i in $path2; do
	eval "d=\$${i}"
	if [ ! -d "$d" ] ; then
		mkdir -p "$d"
		if [ ! -d "$d" ] ; then
			echo "Error: unable to create directory $d"
			exit 1
		else
			echo "$d: created"
			chmod 0750 "$d" > /dev/null
		fi
	else
		echo "$d: already exists"
	fi
done

for i in $path3; do
	eval "d=\$${i}"
	if [ ! -d "$d" ] ; then
		mkdir -p "$d"
		if [ ! -d "$d" ] ; then
			echo "Error: unable to create directory $d"
			exit 1
		else
			echo "$d: created"
			chmod 0755 "$d" > /dev/null
		fi
	else
		echo "$d: already exists"
	fi
done


##=======================================================
## Copy all files to the location specified.
##=======================================================

echo
echo "----------------------------------------------"
echo "Copying files..."
echo

##-------------------------------------------------------
## Find all the relevant files. For each
## file in the list, if it exists with size greater
## than zero AND (clobber == false), then don't
## overwrite that file.  
## Associate loose file names with directories.
## Special case the Release_Notes file through README_LOC.
## This is used in the install script's closing message.
## Make sure README_LOC and fil1 stay in sync.
##-------------------------------------------------------

#f1=' ff=$README ; d="" ; dd=$TWDOCS ; rr=0444 '
#f2=' ff=$REL_NOTES ; d="" ; dd=$TWDOCS ; rr=0444 '
f3=' ff=$TWLICENSEFILE ; d="" ; dd=$TWDOCS ; rr=0444 '
f4=' ff=tripwire ; d="/bin" ; dd=$TWBIN ; rr=0550 '
f5=' ff=twadmin ; d="/bin" ; dd=$TWBIN ; rr=0550 '
f6=' ff=twprint ; d="/bin" ; dd=$TWBIN ; rr=0550 '
f7=' ff=siggen ; d="/bin" ; dd=$TWBIN ; rr=0550 '
f8=' ff=TRADEMARK ; d="" ; dd=$TWDOCS ; rr=0444 '
f9=' ff=policyguide.txt ; d="/policy" ; dd=$TWDOCS ; rr=0444 '
f10=' ff=${POLICYSRC} ; d="/policy" ; dd=$TWPOLICY ; rr=0640 '
#f11=' ff=twpolicy.4 ; d="/man/man4" ; dd=$TWMAN/man4 ; rr=0444 '
#f12=' ff=twconfig.4 ; d="/man/man4" ; dd=$TWMAN/man4 ; rr=0444 '
#f13=' ff=twfiles.5 ; d="/man/man5" ; dd=$TWMAN/man5 ; rr=0444 '
#f14=' ff=siggen.8 ; d="/man/man8" ; dd=$TWMAN/man8 ; rr=0444 '
#f15=' ff=tripwire.8 ; d="/man/man8" ; dd=$TWMAN/man8 ; rr=0444 '
#f16=' ff=twadmin.8 ; d="/man/man8" ; dd=$TWMAN/man8 ; rr=0444 '
#f17=' ff=twintro.8 ; d="/man/man8" ; dd=$TWMAN/man8 ; rr=0444 '
#f18=' ff=twprint.8 ; d="/man/man8" ; dd=$TWMAN/man8 ; rr=0444 '
f19=' ff=COMMERCIAL ; d="" ; dd=$TWDOCS ; rr=0444 '
f20=' ff=ReadMe-2.4.3 ; d="" ; dd=$TWDOCS ; rr=0444 '
f21=' ff=ChangeLog ; d="" ; dd=$TWDOCS ; rr=0444 '

# Binaries and manpages are already installed by the install target
loosefiles="f3 f4 f5 f6 f7 f8 f9 f10 f19 f20 f21"

for i in $loosefiles; do
	eval "eval \"\$$i\""
	f=${TAR_DIR}$d/$ff
	ff=${dd}/$ff
	if [ -s $ff ] && [ "$CLOBBER" = "false" ] ; then
		echo "$ff: file already exists"
	else
		cp "$f" "$dd"
                if [ $? -eq 0 ]; then
			echo "$ff: copied"
			    chmod "$rr" "$ff" > /dev/null
                        else
			echo "$ff: copy failed"
		fi
       	fi
done

if [ -n "$INSTALL_STRIP_FLAG" ] ; then
  echo "INSTALL_STRIP_FLAG is set, stripping binaries"
  chmod u+w "$TWBIN/siggen" "$TWBIN/tripwire" "$TWBIN/twadmin" "$TWBIN/twprint"
  strip "$TWBIN/siggen" "$TWBIN/tripwire" "$TWBIN/twadmin" "$TWBIN/twprint"
  chmod u-w "$TWBIN/siggen" "$TWBIN/tripwire" "$TWBIN/twadmin" "$TWBIN/twprint"
fi

#Make extra sure we don't install the unit test binary to sbin
if  [ -e "$TWBIN/twtest" ] ; then
  rm -f "$TWBIN/twtest"
fi


##=======================================================
## Files are now present on user's system.
## Begin Tripwire configuration.
##=======================================================

##-------------------------------------------------------
## Set default values for Tripwire file names.
##-------------------------------------------------------

HOST_NAME=`uname -n 2>/dev/null`

LOCAL_KEY="${TWLOCALKEYDIR}/${HOST_NAME:=localhost}-local.key"
SITE_KEY="${TWSITEKEYDIR}/site.key"
CONFIG_FILE="${TWPOLICY}/$TW_CONFIG_FILE"  # Signed config file
POLICY_FILE="${TWPOLICY}/tw.pol"        # Signed policy file
TXT_CFG="${TWPOLICY}/${CLR_CONFIG_FILE}"   # Cleartext config file
TXT_POL="${TWPOLICY}/$CLR_POLICY_FILE"  # Cleartext policy file
TWADMIN="${TWBIN}/twadmin"

##-------------------------------------------------------
## If user has to enter a passphrase, give some
## advice about what is appropriate.
##-------------------------------------------------------

if [ -z "$TW_SITE_PASS" ] || [ -z "$TW_LOCAL_PASS" ]; then
cat << END_OF_TEXT

----------------------------------------------
The Tripwire site and local passphrases are used to
sign a variety of files, such as the configuration,
policy, and database files.

Passphrases should be at least 8 characters in length
and contain both letters and numbers.

See the Tripwire manual for more information.
END_OF_TEXT
fi

##=======================================================
## Generate keys.
##=======================================================

echo
echo "----------------------------------------------"
echo "Creating key files..."

##-------------------------------------------------------
## Site key file.
##-------------------------------------------------------

# If clobber is true, and prompting is off (unattended operation)
# and the key file already exists, remove it.  Otherwise twadmin
# will prompt with an "are you sure?" message.

if [ "$CLOBBER" = "true" ] && [ "$PROMPT" = "false" ] && [ -f "$SITE_KEY" ] ; then
        rm -f "$SITE_KEY"
fi

if [ -f "$SITE_KEY" ] && [ "$CLOBBER" = "false" ] ; then
	echo "The site key file \"$SITE_KEY\""
	echo 'exists and will not be overwritten.'
else
	cmdargs="--generate-keys --site-keyfile \"$SITE_KEY\""
	if [ -n "$TW_SITE_PASS" ] ; then
		cmdargs="$cmdargs --site-passphrase \"$TW_SITE_PASS\""
     	fi
	eval "\"$TWADMIN\" $cmdargs"
	if [ $? -ne 0 ] ; then
		echo "Error: site key generation failed"
		exit 1
        else chmod 640 "$SITE_KEY"
	fi
fi

##-------------------------------------------------------
## Local key file.
##-------------------------------------------------------

# If clobber is true, and prompting is off (unattended operation)
# and the key file already exists, remove it.  Otherwise twadmin
# will prompt with an "are you sure?" message.

if [ "$CLOBBER" = "true" ] && [ "$PROMPT" = "false" ] && [ -f "$LOCAL_KEY" ] ; then
        rm -f "$LOCAL_KEY"
fi

if [ -f "$LOCAL_KEY" ] && [ "$CLOBBER" = "false" ] ; then
	echo "The site key file \"$LOCAL_KEY\""
	echo 'exists and will not be overwritten.'
else
	cmdargs="--generate-keys --local-keyfile \"$LOCAL_KEY\""
	if [ -n "$TW_LOCAL_PASS" ] ; then
		cmdargs="$cmdargs --local-passphrase \"$TW_LOCAL_PASS\""
        fi
	eval "\"$TWADMIN\" $cmdargs"
	if [ $? -ne 0 ] ; then
		echo "Error: local key generation failed"
		exit 1
        else chmod 640 "$LOCAL_KEY"
	fi
fi

##=======================================================
## Generate tripwire configuration file.
##=======================================================

echo
echo "----------------------------------------------"
echo "Generating Tripwire configuration file..."

cat << END_OF_TEXT > "$TXT_CFG"
ROOT          =$TWBIN
POLFILE       =$POLICY_FILE
DBFILE        =$TWDB/\$(HOSTNAME).twd
REPORTFILE    =$TWREPORT/\$(HOSTNAME)-\$(DATE).twr
SITEKEYFILE   =$SITE_KEY
LOCALKEYFILE  =$LOCAL_KEY
EDITOR        =$TWEDITOR
LATEPROMPTING =${TWLATEPROMPTING:-false}
LOOSEDIRECTORYCHECKING =${TWLOOSEDIRCHK:-false}
MAILNOVIOLATIONS =${TWMAILNOVIOLATIONS:-true}
EMAILREPORTLEVEL =${TWEMAILREPORTLEVEL:-3}
REPORTLEVEL   =${TWREPORTLEVEL:-3}
MAILMETHOD    =${TWMAILMETHOD:-SENDMAIL}
SYSLOGREPORTING =${TWSYSLOG:=true}
END_OF_TEXT

if [ "$TWMAILMETHOD" = "SMTP" ] ; then
cat << SMTP_TEXT >> "$TXT_CFG"
SMTPHOST      =${TWSMTPHOST:-mail.domain.com}
SMTPPORT      =${TWSMTPPORT:-"25"}
SMTP_TEXT
else
cat << SENDMAIL_TEXT >> "$TXT_CFG"
MAILPROGRAM   =$TWMAILPROGRAM
SENDMAIL_TEXT
fi

if [ ! -s "$TXT_CFG" ] ; then
	echo "Error: unable to create $TXT_CFG"
	exit 1
fi

chmod 640 "$TXT_CFG"

##=======================================================
## Create signed tripwire configuration file.
##=======================================================

echo
echo "----------------------------------------------"
echo "Creating signed configuration file..."

##-------------------------------------------------------
## If noclobber, then backup any existing config file.
##-------------------------------------------------------

if [ "$CLOBBER" = "false" ] && [ -s "$CONFIG_FILE" ] ; then
	backup="${CONFIG_FILE}.$$.bak"
	echo "Backing up $CONFIG_FILE"
	echo "        to $backup"
	`mv "$CONFIG_FILE" "$backup"`
	if [ $? -ne 0 ] ; then
		echo "Error: backup of configuration file failed."
		exit 1
	fi
fi

##-------------------------------------------------------
## Build command line.
##-------------------------------------------------------

cmdargs="--create-cfgfile"
cmdargs="$cmdargs --cfgfile \"$CONFIG_FILE\""
cmdargs="$cmdargs --site-keyfile \"$SITE_KEY\""
if [ -n "$TW_SITE_PASS" ] ; then
	cmdargs="$cmdargs --site-passphrase \"$TW_SITE_PASS\""
fi

##-------------------------------------------------------
## Sign the file.
##-------------------------------------------------------

eval "\"$TWADMIN\" $cmdargs \"$TXT_CFG\""
if [ $? -ne 0 ] ; then
	echo "Error: signing of configuration file failed."
	exit 1
fi

# Set the rights properly
chmod 640 "$CONFIG_FILE"

##-------------------------------------------------------
## We keep the cleartext version around.
##-------------------------------------------------------

cat << END_OF_TEXT

A clear-text version of the Tripwire configuration file
$TXT_CFG
has been preserved for your inspection.  It is recommended
that you delete this file manually after you have examined it.

END_OF_TEXT

##=======================================================
## Modify default policy file with file locations
##=======================================================

echo
echo "----------------------------------------------"
echo "Customizing default policy file..."

sed '/@@section GLOBAL/,/@@section FS/  {
  s?^\(TWROOT=\).*$?TWDOCS='\""$TWDOCS"\"';?
  s?^\(TWBIN=\).*$?\1'\""$TWBIN"\"';?
  s?^\(TWPOL=\).*$?\1'\""$TWPOLICY"\"';?
  s?^\(TWDB=\).*$?\1'\""$TWDB"\"';?
  s?^\(TWSKEY=\).*$?\1'\""$TWSITEKEYDIR"\"';?
  s?^\(TWLKEY=\).*$?\1'\""$TWLOCALKEYDIR"\"';?
  s?^\(TWREPORT=\).*$?\1'\""$TWREPORT"\"';?
  s?^\(HOSTNAME=\).*$?\1'"$HOST_NAME"';?
}' "${TWPOLICY}/${POLICYSRC}" > "${TXT_POL}.tmp"

# copy the tmp file back over the default policy
[ -f "${TXT_POL}" ] && cp "${TXT_POL}" "${TXT_POL}.bak"
mv "${TXT_POL}.tmp" "${TXT_POL}"
rm -f "${TWPOLICY}/${POLICYSRC}"

# reset rights on the policy files to 640
[ -f "${TXT_POL}" ] && chmod 640 "$TXT_POL"
[ -f "${TXT_POL}.bak" ] && chmod 640 "${TXT_POL}.bak"


##=======================================================
## Create signed tripwire policy file.
##=======================================================

echo
echo "----------------------------------------------"
echo "Creating signed policy file..."

##-------------------------------------------------------
## If noclobber, then backup any existing policy file.
##-------------------------------------------------------

if [ "$CLOBBER" = "false" ] && [ -s "$POLICY_FILE" ] ; then
	backup="${POLICY_FILE}.$$.bak"
	echo "Backing up $POLICY_FILE"
	echo "        to $backup"
	mv "$POLICY_FILE" "$backup"
	if [ $? -ne 0 ] ; then
		echo "Error: backup of policy file failed."
		exit 1
	fi
fi

##-------------------------------------------------------
## Build command line.
##-------------------------------------------------------

cmdargs="--create-polfile"
cmdargs="$cmdargs --cfgfile \"$CONFIG_FILE\""
cmdargs="$cmdargs --site-keyfile \"$SITE_KEY\""
if [ -n "$TW_SITE_PASS" ] ; then
	cmdargs="$cmdargs --site-passphrase \"$TW_SITE_PASS\""
fi

##-------------------------------------------------------
## Sign the file.
##-------------------------------------------------------

eval "\"$TWADMIN\" $cmdargs \"$TXT_POL\""
if [ $? -ne 0 ] ; then
	echo "Error: signing of policy file failed."
	exit 1
fi

# Set the proper rights on the newly signed policy file.
chmod 0640 "$POLICY_FILE"

##-------------------------------------------------------
## We keep the cleartext version around.
##-------------------------------------------------------

cat << END_OF_TEXT

A clear-text version of the Tripwire policy file
$TXT_POL
has been preserved for your inspection.  This implements
a minimal policy, intended only to test essential
Tripwire functionality.  You should edit the policy file
to describe your system, and then use twadmin to generate
a new signed copy of the Tripwire policy.

END_OF_TEXT

##=======================================================
## Clean-up.
##=======================================================

cat << END_OF_TEXT

----------------------------------------------
The installation succeeded.

Please refer to documentation in $TWDOCS
for release information and to the printed user documentation
for further instructions on using Tripwire 2.4 Open Source.

END_OF_TEXT

cd "$START_DIR"

