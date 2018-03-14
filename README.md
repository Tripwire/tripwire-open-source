# Open Source Tripwire<sup>®</sup>

Open Source Tripwire<sup>®</sup> software is a security and data integrity tool useful for monitoring and alerting on specific file change(s) on a range of systems. The project is based on code originally contributed by [Tripwire, Inc.](http://www.tripwire.com) in 2000.

Open Source Tripwire is suitable for monitoring a small number of servers, where centralized control and reporting is not needed and professional support or system automation is not a requirement.

## Overview
A tripwire check compares the current filesystem state against a known baseline state, and alerts on any changes it detects.  The baseline and check behavior are controlled by a policy file, which specifies which files or directories to monitor, and which attributes to monitor on them, such as hashes, file permissions, and ownership.  

When a desired change occurs, such as upgrading a package, the baseline database can be updated to the new known-good state.  The policy can also be updated, for example to reduce noise or cover a newly installed package.

## Getting Started

This section covers manual setup of Open Source Tripwire.  If you're installing from an RPM or Debian package, or via "make install", a setup script will walk you through the initial setup steps (key generation thru policy creation) and these will not need to be done by hand.

### Generating Keys
The first step is to generate site and local key files.  This is necessary because Tripwire policy, configuration, and database files are signed by default, and report files may also be signed.  The site key is used to sign config and policy files, with the idea that multiple machines will share a site key, but each will have its own local key.  The policy and config files can then be created once and distributed across these machines.

A common practice is to include the hostname in the local key filename, as follows: 
 
```
     ./twadmin --generate-keys -L /etc/tripwire/${HOSTNAME}-local.key
     ./twadmin --generate-keys -S /etc/tripwire/site.key
``` 

### Creating a configuration file
The next step is to create a Tripwire config file.  The config file contains a variety of settings including the locations of Tripwire binaries and key files, email report settings, and parameters that control baseline/check behavior.  These settings are explained in detail in the twconfig(4) manual page.  

This command line creates the config text in /etc/tripwire/twcfg.txt, validates and signs it with the site key file, and writes the results to /etc/tripwire/tw.cfg:

```
     ./twadmin --create-cfgfile -S /etc/tripwire/site.key /etc/tripwire/twcfg.txt
```
### Generating a policy file

Now it's time to configure OST for what you want to monitor.  A few simple examples of policy rules:

```
/start/point -> $(IgnoreNone); # Get all attributes for this dir tree
/another/start -> +pinugS; # Get selected attributes for this dir tree
!/start/point/subdir/to/ignore; # Don't monitor this dir tree
```
The Tripwire policy language is documented in detail in the twpolicy(4) manual page, and default policies for several popular operating systems are available in the OST project's policy subdirectory. 

```
     ./twadmin --create-polfile -S /etc/tripwire/twpol.txt
```

### Baselining your system

Now you're ready to baseline the system for the first time.  

```
     tripwire --init
```
This creates a database file in the configured directory, typically a file with a .twd extension /var/lib/tripwire.  The optional "--verbose" argument to init mode lists files and directories as they're being scanned.
   
### Running a check
```
     tripwire --check
```
This runs a check, again with an optional "--verbose" option that displays what it's doing.  Scan results are written to standard out, as well as a report file, which typically has a .twr extension and lives in /var/lib/tripwire/report.  If email reporting is enabled, emails will be sent at the end of the check.

### Printing a report
``` 
	twprint -m r -t [0-4] -r /path/to/reportfile.twr
```
The -t argument specifies the level of report verbosity, where 0 is a single line summary of the report contents, and 4 displays all gathered attributes on all changed objects.  The report level defaults to 3 if not specified on the command line or via the REPORTLEVEL config file option.

Databases can be also printed with:

```
	twprint -m d -d /path/to/database.twd
```

### Updating a database
The simplest form of update updates the database with all the changes in a report file:

```
	tripwire --update --accept-all
```

While a 
``` 
	tripwire --update
```
brings up a text report in the user's preferred editor (as configured in the config file's EDITOR option), with a checkbox next to each detected change.  If you save and exit the editor, the database will only be updated for those objects that remain selected with an [x].

### Updating a policy
Policy update mode makes it possible to modify the existing Tripwire policy without losing existing baselines.

```
	tripwire --update-policy updated-policy.txt
```

A check is run with the new policy as part of the update process.

TODO: Explain circumstances in which policy update fails due to check outcome.

### Testing the email configuration
To test email configuration:

```
	tripwire --test --email user@domain.tld
```
This sends a test email to the specified address, using the email settings specified in the config file.

## Building OST

### Prerequisites

A C++ compiler.  It's known to build with gcc and clang; OST should work with gcc versions as old as 2.95.2, although gcc older than version 3.1 will need an external STLPort package.

A POSIX-like operating system.  TODO: list

Perl 5 [what version?] to run the project's test suite.

### Configuring & Building

OST uses a standard automake build, such that your first step will generally be:
```
./configure
```

Different paths, compiler args (like Debian hardening options), non-default locations can be set up in this step.

[TODO: give some common configure options here]

You might need to run the script
```
./touchconfig.sh```
before you're able to build the project.  The script simply touches files in the right order so their last change times are not all identical, and that they're different in the right order.

Then just 
```make```
to build the project.

## Running the test suites

the ```make check``` make target runs two things:  The acceptance test suite in the src/test-harness directory, and unit tests by running twtest, which is built in the bin directory along with other tripwire binaries.  These tests can also be run separately:
```./twtest``` runs all unit tests, while ```./twtest list``` lists all available tests.
```./twtest Groupname``` runs all tests in a group, and 
```./twtest Groupname/Testname``` just runs the specified test.
 

To run the acceptance tests manually, cd to the src/test-harness directory and run ```perl ./twtest.pl```.
 

## Deployment

The ```make install``` target installs OST to the configured location, and ```make install-strip``` installs and removes symbols from the tripwire binaries.  A ```make dist``` creates a gzipped source bundle.

## Authors

* [Tripwire, Inc.](http://www.tripwire.com)


## License

The developer of the original code and/or files is Tripwire, Inc. 
Portions created by Tripwire, Inc. are copyright 2000-2018 Tripwire, Inc.
Tripwire is a registered trademark of Tripwire, Inc.  All rights reserved.

This program is free software.  The contents of this file are subject to the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.  You may redistribute it and/or modify it only in compliance with the GNU General Public License.

This program is distributed in the hope that it will be useful.  However,
this program is distributed "AS-IS" WITHOUT ANY WARRANTY; INCLUDING THE
IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
Please see the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

Nothing in the GNU General Public License or any other license to use the
code or files shall permit you to use Tripwire's trademarks, service marks, or other intellectual property without Tripwire's prior written consent.

If you have any questions, please contact Tripwire, Inc. at either
info@tripwire.org or www.tripwire.org.
