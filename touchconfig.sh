#!/bin/sh
# On a fresh clone/checkout/untar/etc., sometimes make thinks
# automake files are out of sync because they all have the same timestamp,
# and insists they need to be regenerated, though they really don't.
# This script bumps the timestamps on the right files in the right order,
# such that they don't all match, and make can proceed on its way.
touch configure.ac acinclude.m4 aclocal.m4 configure Makefile.am Makefile.in config.h.in


