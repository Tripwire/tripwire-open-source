#Open Source Tripwire® 

Open Source Tripwire® software is a security and data integrity tool useful for monitoring and alerting on specific file change(s) on a range of systems. The project is based on code originally contributed by [Tripwire, Inc.](http://www.tripwire.com)in 2000.

Open Source Tripwire is suitable for monitoring a small number of Linux servers, where centralized control and reporting is not needed and professional support or system automation is not a requirement. 

#General Instruction

The tripwire package comes with a basic configuration file
/etc/tripwire/twcfg.txt, which sets the mandatory variables
to the defaults as described in the twconfig(4) manual
page. This configuration is merely enough to set tripwire
to work.

The following five steps can serve you as a quick cookbook for
setting tripwire to work.

1. Choose a convenient HOSTNAME and generate site and local keys using

     twadmin --generate-keys -L /etc/tripwire/${HOSTNAME}-local.key
     twadmin --generate-keys -S /etc/tripwire/site.key

   This creates the files named above as arguments.

2. Compile the configuration file with

     twadmin --create-cfgfile -S /etc/tripwire/site.key /etc/tripwire/twcfg.txt

   This creates file /etc/tripwire/tw.cfg.

3. Create a policy file. A complex example can be found in
   /usr/share/doc/packages/tripwire/twpol-Linux.txt. For test purposes,
   a single rule

     /bin -> $(ReadOnly);   # the ending semicolon is mandatory

   or alike will do. Compile this with

     twadmin --create-polfile -S /etc/tripwire/site.key /etc/tripwire/twpol.txt

   provided /etc/tripwire/twpol.txt is the name of your policy file.
   This creates file /etc/tripwire/tw.pol.

4. Generates a baseline database (snapshot of the objects residing on
   the system, according to the installed policy file) using

     tripwire --init

   This creates file /var/lib/tripwire/${HOSTNAME}.twd.

5. You can check the system with

     tripwire --check

   This prints a report on the standard output and generates file
   /var/lib/tripwire/report/${HOSTNAME}-YYYYMMDD-HHMMSS.twr. The report can
   be redisplayed using

     twprint --print-report -r /var/lib/tripwire/report/${HOSTNAME}-YYYYMMDD-HMMSS.twr
