AUTOMAKE_OPTIONS = foreign no-dependencies
SUBDIRS = man src
EXTRA_DIST = COMMERCIAL MAINTAINERS TRADEMARK LICENSE Packaging ReadMe-2.4.3 README.md autogen.sh autogen.sh.README touchconfig.sh contrib policy installer

install-data-hook:
	export INSTALL_STRIP_FLAG
	prefix="$(prefix)" sysconfdir="$(sysconfdir)" \
        path_to_vi="$(path_to_vi)" path_to_sendmail="$(path_to_sendmail)" \
        $(top_srcdir)/installer/install.sh

uninstall-hook:
	rm -f ${prefix}/sbin/tripwire $(prefix)/sbin/twadmin $(prefix)/sbin/twprint $(prefix)/sbin/siggen
	rm -Rf $(prefix)/doc

check:
	rm -Rf $(top_srcdir)/src/test-harness/twtest
	cd $(top_srcdir)/src/test-harness && ./twtest.pl
	$(top_srcdir)/bin/twtest all
