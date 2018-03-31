AUTOMAKE_OPTIONS = foreign no-dependencies
SUBDIRS = man src
EXTRA_DIST = COMMERCIAL MAINTAINERS TRADEMARK LICENSE Packaging ReadMe-2.4.3 README.md autogen.sh autogen.sh.README touchconfig.sh contrib policy installer

install-data-hook:
	INSTALL_STRIP_FLAG="$(INSTALL_STRIP_FLAG)" \
	prefix="$(prefix)" sysconfdir="$(sysconfdir)" \
	path_to_vi="$(path_to_vi)" path_to_sendmail="$(path_to_sendmail)" \
	$(top_srcdir)/installer/install.sh

uninstall-hook:
	rm -f ${prefix}/sbin/tripwire $(prefix)/sbin/twadmin $(prefix)/sbin/twprint $(prefix)/sbin/siggen
	rm -Rf $(prefix)/doc

check:
	rm -Rf $(top_srcdir)/src/test-harness/twtest
	rm -Rf $(top_srcdir)/bin/TWTestData
	cd $(top_srcdir)/src/test-harness && perl ./twtest.pl
	cd $(top_srcdir)/bin && ./twtest all

test: check

.PHONY: targets
targets:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$' | xargs
