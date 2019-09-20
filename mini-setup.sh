#!/bin/sh

# Configures a minimal OST setup in the build directory, as a dev/test aid.
# This script isn't meant for use in production, and intentionally uses
# terrible hardcoded passphrases to try to discourage that.

cp src/mini-setup/twcfg.txt bin/twcfg.txt
cp src/mini-setup/twpol.txt bin/twpol.txt
cd bin

if [ ! -f "l" ] || [ ! -f "s" ]; then
  echo "***** Generating Keys *****"
  if ./twadmin --generate-keys --site-keyfile s --local-keyfile l --site-passphrase s --local-passphrase l --verbose
  then
    echo "***** Keys Generated *****" 
  else
    echo "***** Key Generation Failed *****"
    exit 1
  fi
else
  echo "***** Keys already generated, skipping this step *****"
fi

if [ ! -f "tw.cfg" ]; then
  echo "***** Creating Config File *****"
  if ./twadmin --create-cfgfile --site-keyfile s --site-passphrase s --verbose --cfgfile ./tw.cfg ./twcfg.txt
  then
    echo "***** Config File Created *****"
  else
    echo "***** Config File Creation Failed *****"
    exit 1
  fi
else
    echo "***** Config file already exists, skipping this step *****"
fi

if [ ! -f "test.pol" ]; then
  echo "***** Creating Policy File *****"
  if ./twadmin --create-polfile --site-passphrase s --verbose --cfgfile ./tw.cfg ./twpol.txt
  then
    echo "***** Policy File Created *****"
  else
    echo "***** Policy File Creation Failed *****"
    exit 1
  fi
else
  echo "***** Policy file already exists, skipping this step *****"
fi

if [ ! -d "/tmp/tw-test" ]; then 
  mkdir /tmp/tw-test
fi

if [ ! -f "test.twd" ]; then
  echo "***** Initializing Database *****"
  if ./tripwire --init --local-passphrase l --verbose --cfgfile ./tw.cfg 
  then
    echo "***** Database Initialized *****"
  else
    echo "***** Database Initialization Failed *****"
    exit 1
  fi
else
  echo "***** Database already initialized, skipping this step *****"
fi

touch /tmp/tw-test/hello.txt

echo "***** Running a Check *****"
./tripwire --check --verbose --cfgfile ./tw.cfg

echo "***** Setup Completed *****"
