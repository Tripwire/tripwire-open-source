%define		path_to_vi /bin/vi
%define		path_to_sendmail /usr/sbin/sendmail

Name:		tripwire
Version:	2.4.2.2
Release:	1%{?dist}
Summary:	IDS (Intrusion Detection System)

License:	GPL
Group:		Applications/System
Source0:	http://download.sourceforge.net/tripwire/tripwire-%{version}-src.tar.bz2
Source1:	tripwire.cron.in
Source3:	tripwire.gif
Source4:	twcfg.txt.in
Source5:	tripwire-setup-keyfiles.in
Source6:	twpol.txt.in
Source7:	README.Fedora.in
Source9:	License-Issues
URL:		http://www.tripwire.org/
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	openssl-devel
Requires(post):	sed

Patch1:		tripwire-siggen-man8.patch

%description
Tripwire is a very valuable security tool for Linux systems, if  it  is
installed to a clean system. Tripwire should be installed  right  after
the OS installation, and before you have connected  your  system  to  a
network (i.e., before any possibility exists that someone  could  alter
files on your system).

When Tripwire is initially set up, it creates a database  that  records
certain file information. Then when it is run, it compares a designated
set of files and directories to the information stored in the database.
Added or deleted files are flagged and reported, as are any files  that
have changed from their previously recorded state in the database. When
Tripwire is run against system files  on  a  regular  basis,  any  file
changes will be spotted when Tripwire is run. Tripwire will report  the
changes, which will give system administrators a clue that they need to
enact damage control measures immediately if certain  files  have  been
altered.

%prep
%setup -q
%{__cp} -p %{SOURCE3} .

%patch1 -p1 -b .siggen.manpage

%build
%{__chmod} 755 configure
# RPM_OPT_FLAGS break the code (deadlock).
export CXXFLAGS="-O -Wall -pipe -g"
./configure -q \
	path_to_vi=%{path_to_vi} \
	path_to_sendmail=%{path_to_sendmail} \
	--prefix=/ \
	--sysconfdir=%{_sysconfdir}/tripwire \
	--sbindir=%{_sbindir} \
	--libdir=%{_var}/lib \
	--mandir=%{_mandir}

%{__make} %{?_smp_mflags}

%install
%{__rm} -fr ${RPM_BUILD_ROOT}

# Install the binaries.
%{__mkdir_p} ${RPM_BUILD_ROOT}%{_sbindir}
%{__install} -p -m755 bin/siggen ${RPM_BUILD_ROOT}%{_sbindir}
%{__install} -p -m755 bin/tripwire ${RPM_BUILD_ROOT}%{_sbindir}
%{__install} -p -m755 bin/twadmin ${RPM_BUILD_ROOT}%{_sbindir}
%{__install} -p -m755 bin/twprint ${RPM_BUILD_ROOT}%{_sbindir}

# Install the man pages.
%{__mkdir_p} ${RPM_BUILD_ROOT}%{_mandir}/{man4,man5,man8}
%{__install} -p -m644 man/man4/*.4 ${RPM_BUILD_ROOT}%{_mandir}/man4/
%{__install} -p -m644 man/man5/*.5 ${RPM_BUILD_ROOT}%{_mandir}/man5/
%{__install} -p -m644 man/man8/*.8 ${RPM_BUILD_ROOT}%{_mandir}/man8/

# Create configuration files from templates.
%{__rm} -fr _tmpcfg
%{__mkdir} _tmpcfg
for infile in %{SOURCE1} %{SOURCE4} %{SOURCE5} %{SOURCE6} %{SOURCE7} ; do
	outfile=${infile##/*/}
	outfile=${outfile%.*n}
	cat ${infile} |\
	%{__sed} -e 's|@path_to_vi@|%{path_to_vi}|g' |\
	%{__sed} -e 's|@path_to_sendmail@|%{path_to_sendmail}|g' |\
	%{__sed} -e 's|@sysconfdir@|%{_sysconfdir}|g' |\
	%{__sed} -e 's|@sbindir@|%{_sbindir}|g' |\
	%{__sed} -e 's|@vardir@|%{_var}|g' >\
	_tmpcfg/${outfile}
done
%{__mv} _tmpcfg/{tripwire-setup-keyfiles,README.Fedora} .

# Create the reports directory.
%{__install} -d -m700 ${RPM_BUILD_ROOT}%{_var}/lib/tripwire/report

# Install the cron job.
%{__install} -d -m755 ${RPM_BUILD_ROOT}%{_sysconfdir}/cron.daily
%{__install} -p -m755 _tmpcfg/tripwire.cron \
	${RPM_BUILD_ROOT}%{_sysconfdir}/cron.daily/tripwire-check
%{__rm} _tmpcfg/tripwire.cron

# Install configuration files.
%{__mkdir_p} ${RPM_BUILD_ROOT}%{_sysconfdir}/tripwire
for file in _tmpcfg/* ; do
	%{__install} -p -m644 ${file} ${RPM_BUILD_ROOT}%{_sysconfdir}/tripwire
done

# Install the keyfile setup script
%{__install} -p -m755 tripwire-setup-keyfiles ${RPM_BUILD_ROOT}%{_sbindir}

# Fix permissions on documentation files.
%{__cp} -p %{SOURCE9} .
%{__chmod} 644 \
	ChangeLog COMMERCIAL COPYING TRADEMARK tripwire.gif \
	README.Fedora policy/policyguide.txt License-Issues


%clean
%{__rm} -rf ${RPM_BUILD_ROOT}


post
# Set the real hostname in twpol.txt
%{__sed} -i -e "s|localhost|$HOSTNAME|g" %{_sysconfdir}/tripwire/twpol.txt


%files
%defattr(0644,root,root,0755)
%doc ChangeLog COMMERCIAL COPYING TRADEMARK tripwire.gif
%doc README.Fedora policy/policyguide.txt License-Issues
%attr(0700,root,root) %dir %{_sysconfdir}/tripwire
%config(noreplace) %{_sysconfdir}/tripwire/twcfg.txt
%config(noreplace) %{_sysconfdir}/tripwire/twpol.txt
%attr(0755,root,root) %{_sysconfdir}/cron.daily/tripwire-check
%attr(0700,root,root) %dir %{_var}/lib/tripwire
%attr(0700,root,root) %dir %{_var}/lib/tripwire/report
%{_mandir}/*/*
%attr(0755,root,root) %{_sbindir}/*


%changelog
* Mon Nov 21 2011 Stephane Dudzinski <tripwire@frlinux.net> 2.4.2.2
- Updated spec file
- Updated version revision in reports and all
- Added experimental policy creation (see policy/policy_generator_readme.txt)
- Fixed report formating and sendmail issues
- Added Debian patches for crypto and hostnames
- Fixed compiling issue on recent GCC compilers (-fpermissive)

* Wed Jul 13 2011 Stephane Dudzinski <tripwire@frlinux.net> 2.4.2.1
- Updated spec file to compile with 2.4.2.1

* Wed Feb 28 2007 Brandon Holbrook <fedora at theholbrooks.org> 2.4.1.1-1
- Upgrade to upstream 2.4.1.1 (obsoletes gcc4 patch)
- Merge quickstart.txt into README.Fedora and fix doc bug (#161764)

* Thu Dec 21 2006 Brandon Holbrook <fedora at theholbrooks.org> 2.4.0.1-4
- Don't print anything at install time

* Tue Dec 19 2006 Brandon Holbrook <fedora at theholbrooks.org> 2.4.0.1-3
- Changed defattr to 644,755
- removed BR: autoconf
- Inform users about README.Fedora instead of spamming the install
  with catting the whole file

* Wed Nov 15 2006 Brandon Holbrook <fedora at theholbrooks.org> 2.4.0.1-2
- chmod'ed /etc/tripwire to 0700
- Added sed to Requires(post)

* Tue Aug 22 2006 Brandon Holbrook <fedora at theholbrooks.org> 2.4.0.1-1.4
- Include COMMERCIAL file from upstream
- Print README.RPM on initial install
- Added _smp_mflags to make
- Removed ExclusiveArch: ix86
- Replaced 2.3 with 2.4 in tripwire.txt

* Tue Aug 22 2006 Brandon Holbrook <fedora at theholbrooks.org> 2.4.0.1-1.2
- Updated to 2.4.0.1

* Fri Apr  7 2005 Michael Schwendt <mschwendt[AT]users.sf.net>
- rebuilt

* Tue Jun 15 2004 Keith G. Robertson-Turner <tripwire-devel[AT]genesis-x.nildram.co.uk> 0:2.3.1-20.fdr.1
- Revision bump to supersede Fedora Legacy
- Fixed a bogus entry in twpol.txt.in (modeprobe.conf -> modprobe.conf)

* Thu Jun 10 2004 Keith G. Robertson-Turner <tripwire-devel[AT]genesis-x.nildram.co.uk> 0:2.3.1-18.fdr.9
- Applied Paul Herman's patch to fix a format string vulnerability in
 pipedmailmessage.cpp

* Sun Feb 29 2004 Keith G. Robertson-Turner <tripwire-devel[AT]genesis-x.nildram.co.uk> 0:2.3.1-18.fdr.8
- Default policy overhaul
- Spec cleanup

* Sun Feb 22 2004 Keith G. Robertson-Turner <tripwire-devel[AT]genesis-x.nildram.co.uk> 0:2.3.1-18.fdr.7
- Moved documentation data out of package description

* Sat Feb 21 2004 Keith G. Robertson-Turner <tripwire-devel[AT]genesis-x.nildram.co.uk> 0:2.3.1-18.fdr.6
- Removed explicit Buildrequires gcc-c++

* Fri Feb 20 2004 Keith G. Robertson-Turner <tripwire-devel[AT]genesis-x.nildram.co.uk> 0:2.3.1-18.fdr.5
- Finally moved twinstall.sh from the sysconfdir to the sbindir, since
  it is not a configuration file. Fixes Red Hat bug #61855
- Renamed twinstall.sh to tripwire-setup-keyfiles, since  the  name  is
  misleading. It is setting up keyfiles, not installing an  application
- Minor correction to twinstall.sh (now tripwire-setup-keyfiles), which
  made an incorrect reference to the site key rather than the local key
- Long overdue default policy update
- Added explicit Buildrequires gcc-c++, to satisfy mach

* Thu Feb 19 2004 Keith G. Robertson-Turner <tripwire-devel[AT]genesis-x.nildram.co.uk> 0:2.3.1-18.fdr.4
- Fixed siggen.8 man page, broken command synopsis syntax. Submitted by
  doclifter
- Set real hostname in post, so  Tripwire  works  first  time,  without
  editing twpol.txt
- More accurate package summary
- Spec cleanup

* Fri Nov 28 2003 Keith G. Robertson-Turner <tripwire-devel[AT]genesis-x.nildram.co.uk> 0:2.3.1-18.fdr.3
- Thanks to Michael Schwendt for really cleaning up the Spec file
- The remaining parts of the  original  tripwire-2.3.1-gcc3.patch  have
  now been implemented
- Debuginfo fully builds now

* Thu Nov 27 2003 Keith G. Robertson-Turner <tripwire-devel[AT]genesis-x.nildram.co.uk> 0:2.3.1-18.fdr.2
- Removed version specific grep dependency, since grep >= 2.3 is common
- Added openssl-devel and autoconf to build dependencies
- The tripwire-jbj.patch is now confirmed merged with tw-20030919.patch
- Added RPM optimisation flags option, disabled  by  default  since  it
  breaks the code
- Fixed file permissions of packaged files

* Wed Nov 26 2003 Keith G. Robertson-Turner <tripwire-devel[AT]genesis-x.nildram.co.uk> 0:2.3.1-18.fdr.1
- Implemented Paul Herman's tw-20030919.patch
- Removed the fhs gcc3 and jbj patches, which are now  broken/obsoleted
  by the above
- Both the mkstemp and rfc822 patches are still implemented
- Build uses autoconf for now
- Spec file given complete overhaul for stricter compliance. More to do

* Wed Jan 22 2003 Tim Powers <timp@redhat.com>
- rebuilt

* Sat Nov 16 2002 Jeff Johnson <jbj@redhat.com> 2.3.1-16
- rebuild from cvs.
- comment out debug messages to achieve compilation.
- include policyguide.txt (#72259).
- use mkstemp, not mktemp.

* Fri Aug 02 2002 Mike A. Harris <mharris@redhat.com> 2.3.1-14
- Modified default sample twpol file to remove bogus warnings (#70502)

* Fri Jun 21 2002 Tim Powers <timp@redhat.com> 2.3.1-13
- automated rebuild

* Sun May 26 2002 Tim Powers <timp@redhat.com> 2.3.1-12
- automated rebuild

* Wed May 22 2002 Mike A. Harris <mharris@redhat.com> 2.3.1-11
- Rebuilt in new build environment with gcc 3.1

* Tue Feb 26 2002 Mike A. Harris <mharris@redhat.com> 2.3.1-9
- Conditionalized gcc3 patch
- Added back the ExclusiveArch that is required but disappeared somewhere along
  the line.
- Rebuild in new build environment

* Thu Jan 31 2002 Mike A. Harris <mharris@redhat.com> 2.3.1-7
- Bump release and rebuild in new environment.
- (Elliot Lee) Add patch to make it build with gcc3.

* Thu Aug  9 2001 Nalin Dahyabhai <nalin@redhat.com> 2.3.1-5
- define USE_FHS when USES_FHS is defined, so that the database winds up
  in the right directory (#51332)
- update default twpol file to include files recently-added to the full
  installation tree

* Tue Jul 17 2001 Mike A. Harris <mharris@redhat.com> 2.3.1-4
- Applied bugfix for (#47276) to make tripwire email RFC822 compliant, using
  patch in bugreport from Michael Schwendt <mschwendt@yahoo.com>

* Tue Jul 10 2001 Mike A. Harris <mharris@redhat.com> 2.3.1-3
- Made package own dir /var/lib/tripwire

* Mon Jun 25 2001 Nalin Dahyabhai <nalin@redhat.com>
- update to 2.3.1-2

* Thu Mar  1 2001 Bill Nottingham <notting@redhat.com>
- rebuild, fix defattr. Weird.

* Tue Feb 27 2001 Nalin Dahyabhai <nalin@redhat.com>
- refresh from upstream
- modify the default policy to match the current tree more closely (#28744)
- make the text files 0644, not 0755
- defattr for the sake of the docs

* Wed Sep 20 2000 Nalin Dahyabhai <nalin@redhat.com>
- change exclusivearch: i386 to exclusivearch: %%{ix86} (#17759)

* Wed Aug 23 2000 Than Ngo <than@redhat.com>
- remove copyleft information in specfile (Bug #16765)

* Tue Aug 22 2000 Nalin Dahyabhai <nalin@redhat.com>
- remove duplicate source files
- sync up description with specspo

* Fri Aug 4 2000 Than Ngo <than@redhat.de>
- remove Vendor and Distribution from specfile (Bug #15246)

* Fri Aug 4 2000 Than Ngo <than@redhat.de>
- starts tripwire --check if it was configured before. (Bug #15384)

* Fri Aug 4 2000 Nalin Dahyabhai <nalin@redhat.com>
- fix sense of checking for the database's existence in the cron job
- actually include twinstall.sh, twcfg.txt, twpol.txt

* Thu Aug 3 2000 Than Ngo <than@redhat.de>
- permission fix (bug #15246)

* Mon Jul 31 2000 Nalin Dahyabhai <nalin@redhat.com>
- add quickstart docs (Ed)
- tweak description text (Ed)

* Thu Jul 20 2000 Nalin Dahyabhai <nalin@redhat.com>
- update .spec file to follow RPM conventions
- add tripwire --check to cron.daily

