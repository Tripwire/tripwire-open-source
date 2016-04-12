AUTOMAKE_OPTIONS = foreign no-dependencies
SUBDIRS = man src
EXTRA_DIST = COMMERCIAL MAINTAINERS TRADEMARK touchconfig.sh contrib policy

install-data-hook:
	prefix="$(prefix)" sysconfdir="$(sysconfdir)" \
        path_to_vi="$(path_to_vi)" path_to_sendmail="$(path_to_sendmail)" \
        ./install/install.sh
