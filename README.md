# Open Source Tripwire<sup>®</sup>

Open Source Tripwire<sup>®</sup> software is a security and data integrity tool useful for monitoring and alerting on specific file change(s) on a range of systems. The project is based on code originally contributed by [Tripwire, Inc.](http://www.tripwire.com) in 2000.

Open Source Tripwire is suitable for monitoring a small number of servers, where centralized control and reporting is not needed and professional support or system automation is not a requirement.

## General Instruction

via a fork: https://raw.githubusercontent.com/circuitStatic/tripwireOpenSource/master/README.md

The tripwire package comes with a basic configuration file
/etc/tripwire/twcfg.txt, which sets the mandatory variables
to the defaults as described in the twconfig(4) manual
page. This configuration is merely enough to set tripwire
to work.

The following five steps can serve you as a quick cookbook for
setting tripwire to work.

1. Choose a convenient HOSTNAME and generate site and local keys using
```
     twadmin --generate-keys -L /etc/tripwire/${HOSTNAME}-local.key
     twadmin --generate-keys -S /etc/tripwire/site.key
```
   This creates the files named above as arguments.

2. Compile the configuration file with
```
     twadmin --create-cfgfile -S /etc/tripwire/site.key /etc/tripwire/twcfg.txt
```
   This creates file /etc/tripwire/tw.cfg.

3. Create a policy file. A complex example can be found in
   /usr/share/doc/packages/tripwire/twpol-Linux.txt. For test purposes,
   a single rule
```
     /bin -> $(ReadOnly);   # the ending semicolon is mandatory
```
   or alike will do. Compile this with
```
     twadmin --create-polfile -S /etc/tripwire/site.key /etc/tripwire/twpol.txt
```
   provided /etc/tripwire/twpol.txt is the name of your policy file.
   This creates file /etc/tripwire/tw.pol.

4. Generates a baseline database (snapshot of the objects residing on
   the system, according to the installed policy file) using
```
     tripwire --init
```
   This creates file /var/lib/tripwire/${HOSTNAME}.twd.

5. You can check the system with
```
     tripwire --check
```
   This prints a report on the standard output and generates file
   /var/lib/tripwire/report/${HOSTNAME}-YYYYMMDD-HHMMSS.twr. The report can
   be redisplayed using
```
     twprint --print-report -r /var/lib/tripwire/report/${HOSTNAME}-YYYYMMDD-HMMSS.twr
```


## Getting Started

via template: https://gist.github.com/PurpleBooth/109311bb0361f32d87a2

TODO: These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

TODO: What things you need to install the software and how to install them

```
Give examples
```

### Installing

TODO: A step by step series of examples that tell you have to get a development env running

Say what the step will be

```
Give the example
```

And repeat

```
until finished
```

End with an example of getting some data out of the system or using it for a little demo

## Running the test suites

TODO: Explain how to run the automated tests for this system


## Deployment

TODO: Add additional notes about how to deploy this on a live system


## Authors

* [Tripwire, Inc.](http://www.tripwire.com)


## License

The developer of the original code and/or files is Tripwire, Inc.  Portions
created by Tripwire, Inc. are copyright 2000-2018 Tripwire, Inc.  Tripwire is a
registered trademark of Tripwire, Inc.  All rights reserved.

This program is free software.  The contents of this file are subject to the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.  You may redistribute it and/or modify it only in compliance with
the GNU General Public License.

This program is distributed in the hope that it will be useful.  However,
this program is distributed "AS-IS" WITHOUT ANY WARRANTY; INCLUDING THE
IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
Please see the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

Nothing in the GNU General Public License or any other license to use the
code or files shall permit you to use Tripwire's trademarks, service marks,
or other intellectual property without Tripwire's prior written consent.

If you have any questions, please contact Tripwire, Inc. at either
info@tripwire.org or www.tripwire.org.
