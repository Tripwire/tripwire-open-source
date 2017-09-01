SUBDIRS = cryptlib core db fco fs tw twcrypto twparser util
SUBDIRS+= twprint twadmin siggen tripwire twtest
EXTRA_DIST = buildnum.h buildnum.txt buildnuminc.pl dep_addprefix.pl rmcr.pl check_st.pl prime.pl parser test-harness core/md5.cpp core/sha.cpp

install:
	true

uninstall:
	true

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	-rm -rf test-harness/twtest