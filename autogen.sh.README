a u t o g e n . s h
"also known as buildconf"
http://buildconf.brlcad.org


INTRODUCTION
============

The autogen.sh script provides automatic build system preparation for
projects that use the GNU Autotools build system.  In brief, the
script is a drop-in replacement for running 'autoreconf' while
detecting and cleanly reporting on a variety of common configuration
issues.

The script does a lot more than 'autoreconf' can, though, accounting
for a lot of common issues, bugs, and misconfiguration problems that
would otherwise be problems passed on to your users.  Unlike the
philosophy of some of the GNU developers, the autogen.sh script helps
make things "just work" a little better without imposing unnecessary
burden on your users.


INSTALLING
==========

To install the script, simply copy the script into your source
repository and run it:

sh autogen.sh


RUNNING
=======

The AUTORECONF, AUTOCONF, AUTOMAKE, LIBTOOLIZE, ACLOCAL, AUTOHEADER
environment variables and corresponding _OPTIONS variables (e.g.
AUTORECONF_OPTIONS) may be used to override the default automatic
detection behaviors.  Similarly the _VERSION variables will override
the minimum required version numbers.  Otherwise, you can edit the
script directly to set minimum version numbers.

Examples:

  To obtain help on usage:
    ./autogen.sh --help

  To obtain verbose output:
    ./autogen.sh --verbose

  To skip autoreconf and prepare manually:
    AUTORECONF=false ./autogen.sh

  To verbosely try running with an older (unsupported) autoconf:
    AUTOCONF_VERSION=2.50 ./autogen.sh --verbose


CONTACT
=======

Author:
Christopher Sean Morrison <morrison@brlcad.org>

Patches:
Sebastian Pipping <sebastian@pipping.org>

The autogen.sh script is distributed under the terms of a standard
3-clause BSD-style license.  See the script for the exact language.
