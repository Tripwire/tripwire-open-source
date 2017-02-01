AUTOMAKE_OPTIONS = foreign no-dependencies
SUBDIRS = man src
EXTRA_DIST = COMMERCIAL MAINTAINERS TRADEMARK LICENSE Packaging README-2.4.3 README.md autogen.sh autogen.sh.README touchconfig.sh contrib policy installer

install-data-hook:
	prefix="$(prefix)" sysconfdir="$(sysconfdir)" \
        path_to_vi="$(path_to_vi)" path_to_sendmail="$(path_to_sendmail)" \
        ./installer/install.sh
