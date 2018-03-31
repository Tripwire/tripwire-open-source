dnl Process this file with autoconf to produce a configure script.
dnl
dnl

AC_INIT([tripwire], [2.4.3.7], [https://github.com/Tripwire/tripwire-open-source/issues], [tripwire], [https://github.com/Tripwire/tripwire-open-source])
AC_CONFIG_SRCDIR([src/tw/tw.cpp])
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_COPYRIGHT([The developer of the original code and/or files is Tripwire, Inc.  Portions created by Tripwire, Inc. are copyright 2000-2018 Tripwire, Inc.  Tripwire is a registered trademark of Tripwire, Inc.  All rights reserved.])
AC_REVISION([$Revision: 2.4.3.7 $])

dnl ###############
dnl Setup defaults
dnl ###############
CFLAGS=${CFLAGS:-"-O -pipe -Wall -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"}
CXXFLAGS=${CXXFLAGS:-"-O -pipe -Wall -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"}

dnl #####################
dnl Configuration options
dnl #####################

# This is primarily to support old compilers that don’t understand -Wextra
AC_ARG_ENABLE(extrawarnings,  [  —-disable-extrawarnings do not compile with -Wextra warnings enabled])
if test "x$enable_extrawarnings" != "xno"
then
   CFLAGS="${CFLAGS} -Wextra -Wno-unused-parameter"
   CXXFLAGS="${CXXFLAGS} -Wextra -Wno-unused-parameter"
fi

AC_ARG_ENABLE(static, [  --enable-static         compile static binaries])
if test "x$enable_static" = xyes
then LDFLAGS="${LDFLAGS} -static"
fi

AC_ARG_ENABLE(debug,  [  --enable-debug          compile with debuging enabled])
if test "x$enable_debug" = xyes
then
   CFLAGS="${CFLAGS} -g"
   CXXFLAGS="${CXXFLAGS} -g"
   AC_DEFINE(DEBUG, 1, [Compile with debug code])
else
   AC_DEFINE(NDEBUG, 1, [Compile without debug code])
fi

AC_ARG_ENABLE(coverage, [  --enable-coverage       enable code coverage])
if test "x$enable_coverage" = xyes
then
   CFLAGS="${CFLAGS} --coverage"
   CXXFLAGS="${CXXFLAGS} --coverage"
   LDFLAGS="${LDFLAGS} --coverage"
fi

AC_ARG_ENABLE(profiling, [  --enable-profiling      enable profiling])
if test "x$enable_profiling" = xyes
then
   CFLAGS="${CFLAGS} -pg"
   CXXFLAGS="${CXXFLAGS} -pg"
   LDFLAGS="${LDFLAGS} -pg"
fi

AC_ARG_ENABLE(urandom,  [  --enable-urandom        use /dev/urandom])
if test "x$enable_urandom" = xyes
then
   AC_DEFINE(ENABLE_DEV_URANDOM, 1, [Enable use of /dev/urandom])
fi

dnl ###################
dnl Checks for programs
dnl ###################
AC_PROG_CC([gcc clang suncc aCC xlC_r xlC cl.exe])
AC_PROG_CXX([g++ c++ clang++ sunCC aCC xlC_r xlC cl.exe])
AC_PROG_RANLIB
AC_PROG_YACC
AC_PROG_LN_S
AC_PROG_LN
AN_MAKEVAR([AR], [AC_PROG_AR])
AN_PROGRAM([ar], [AC_PROG_AR])
AC_DEFUN([AC_PROG_AR], [AC_CHECK_TOOL(AR, ar, :)])
AC_PROG_AR
AC_PATH_PROG(path_to_vi, vi)
AC_PATH_PROG(path_to_sendmail, sendmail, "", [$PATH:/usr/libexec])

if test "x${GXX}" != "x"; then
 AC_DEFINE(HAVE_GCC, 1, [Uses the GNU gcc compiler])
else
 AC_DEFINE(HAVE_GCC, 0, [Uses the GNU gcc compiler])
fi

dnl #######################
dnl Checks for header files
dnl #######################
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/param.h)
AC_CHECK_HEADERS(sys/mount.h,,,
[[#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
]])
AC_CHECK_HEADERS(sys/ustat.h sys/sysmacros.h sys/syslog.h sys/socket.h)
AC_CHECK_HEADERS(unistd.h sys/unistd.h)
AC_CHECK_HEADERS(syslog.h langinfo.h sys/statfs.h sys/select.h)
AC_CHECK_HEADERS(signum.h bits/signum.h, break )
AC_CHECK_HEADERS(stdarg.h varargs.h, break )
AC_CHECK_HEADERS(sys/utsname.h memory.h)
AC_CHECK_HEADERS(sys/fs/vx_ioctl.h)

dnl # Special case for malloc.h, because it's depreciated on most systems.
CPPFLAGS_SAVE="${CPPFLAGS}"
CPPFLAGS="${CPPFLAGS} -Werror"
AC_CHECK_HEADERS(malloc.h)
CPPFLAGS="${CPPFLAGS_SAVE}"

dnl #############################################################
dnl Checks for typedefs, structures, and compiler characteristics
dnl #############################################################
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_C_BIGENDIAN
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(time_t)

dnl All platforms we support use 2's complement, are byte aligned, etc...
AC_DEFINE(USES_1S_COMPLEMENT, 0, [Uses one's complement])
AC_DEFINE(USES_2S_COMPLEMENT, 1, [Uses two's complement])
AC_DEFINE(USES_SIGNED_MAGNITUDE, 0, [Uses signed magnitute])
AC_DEFINE(IS_BYTE_ALIGNED, 1, [Is byte aligned])
AC_DEFINE(EXCEPTION_NAMESPACE, std::, [this is the prefix for STL exception functions])

dnl We used to check for UNIX or Unix-like target platforms,
dnl but that is a little extreme, so just assume that the
dnl target is unix.  This can still be changed in config.h
AC_DEFINE(IS_UNIX, 1, [Is a unix type platform])

dnl whether or not to generate debuging code?
AC_DEFINE(NDEBUG, 1, [don't generate debuging code])

dnl look for struct stat members that aren't always there
AC_CHECK_MEMBERS([struct stat.st_rdev, struct stat.st_blocks])

dnl #############################
dnl Checks for standard functions
dnl #############################
AC_CHECK_FUNCS(strftime gethostname gethostid)
AC_CHECK_FUNCS(mkstemp mktemp, break)
AC_CHECK_FUNCS(swab)

dnl check for posix_fadvise
AC_CHECK_HEADERS(fcntl.h, [AC_CHECK_FUNCS(posix_fadvise)])

dnl check for OSX builtin hash algorithms
AC_ARG_ENABLE(commoncrypto,
        [  --disable-commoncrypto  Don't use CommonCrypto hash implementations (OSX only)])

if test "x${enable_commoncrypto}" != "xno"
then
  AC_CHECK_HEADERS(CommonCrypto/CommonDigest.h)
fi

dnl check for door support (Solaris)
AC_CHECK_HEADERS(door.h, [AC_CHECK_FUNCS(door_create)])

dnl check for event port support (Solaris)
AC_CHECK_HEADERS(port.h, [AC_CHECK_FUNCS(port_create)])

dnl ##############################################  
dnl check for various RNG/PRNG devices
dnl ##############################################  

UNAME=`uname`

dnl ############################################## 
dnl AROS pops up a "Please insert disk" dialog for /dev
dnl if script looks for devices (which don't exist)
dnl so don't even try looking.
dnl ############################################## 

if [[ $UNAME != "AROS" ]]; then

  if test -c "/dev/random"; then
    AC_DEFINE(HAVE_DEV_RANDOM, [1], [Has /dev/random])
  fi

  if test -c "/dev/urandom"; then
    AC_DEFINE(HAVE_DEV_URANDOM, [1], [Has /dev/urandom])
  fi

  if test -c "/dev/arandom"; then
    AC_DEFINE(HAVE_DEV_ARANDOM, [1], [Has /dev/arandom])
  fi

fi

dnl ##############################################
dnl Checks for various platform specific libraries
dnl ##############################################

dnl socket?  Solaris has it somewhere else.
AC_CHECK_LIB(c, socket, [:], [
 AC_CHECK_LIB(socket, socket, [
  LIBS="-lsocket $LIBS"
  ], [:]) ])

dnl gethostbyname?  Solaris has it somewhere else.
AC_CHECK_LIB(c, gethostbyname, [:], [
 AC_CHECK_LIB(nsl, gethostbyname, [
  LIBS="-lnsl $LIBS"
  ], [
    AC_CHECK_LIB(network, gethostbyname, [LIBS="-lnetwork $LIBS"] [:])
  ]) ])


AC_ARG_ENABLE(iconv,
        [  --enable-iconv          Use iconv for locale-independent report and db files])

if test "x${enable_iconv}" = xyes
then
  dnl iconv? check the usual locations
  AC_CHECK_LIB(c, iconv, [
     AC_CHECK_HEADERS(iconv.h)
    ], [
   AC_CHECK_LIB(iconv, iconv, [
     LIBS="-liconv $LIBS"
     AC_CHECK_HEADERS(iconv.h)
    ], [:]) ])
fi

dnl check for stl library
AC_LANG_SAVE
AC_LANG_CPLUSPLUS

dnl
dnl Check for STL headers
dnl
AC_CHECK_HEADER(locale,, [
  dnl Don't have native STL headers, look in other places
  AC_CACHE_CHECK([for STLport directory], ac_cv_stlportdir, [
    found_locale=""
    for inclpath in /usr/local/include /usr/local/include/stlport
    do
      CPPFLAGS_save=${CPPFLAGS}
      CPPFLAGS="${CPPFLAGS} -I${inclpath}"
      AC_TRY_COMPILE([
#include <locale>
]
	, , [ found_locale="1" ], )
      CPPFLAGS=${CPPFLAGS_save}
      if test "x${found_locale}" = "x1"
      then
        ac_cv_stlportdir=${inclpath}
        break
      fi
    done
    if test "x${found_locale}" = "x"; then
      echo "not found"
      AC_MSG_WARN( [

	*** You don't seem to have STL support in your C++ compiler.
	*** You can try to build tripwire, but it probably won't compile.
        ])
      fi
    ])
  ])
  if test "x${ac_cv_stlportdir}" != "x"
  then
    CPPFLAGS="${CPPFLAGS} -I${ac_cv_stlportdir}"
  fi
  AC_TRY_LINK([#include <locale>],,,[
    dnl Don't have native STL library, look in other places
    dnl First check for POSIX threads if we compile STLport, because it needs it
    AC_CHECK_FUNC(pthread_getspecific, [:], [

    LDFLAGS_TEMP="${LDFLAGS}"
    LDFLAGS="${LDFLAGS_TEMP} -pthread"
    unset ac_cv_func_pthread_getspecific
    AC_CHECK_FUNC(pthread_getspecific, [:], [

      LDFLAGS="${LDFLAGS_TEMP} -lpthread"
      unset ac_cv_func_pthread_getspecific
      AC_CHECK_FUNC(pthread_getspecific, [:], [
        LDFLAGS="${LDFLAGS_TEMP}"
        AC_MSG_ERROR([No posix threads detected, cannot continue.]) ])
        ])
    ])

    AC_CACHE_CHECK([for STLport library directory], ac_cv_stlportlib, [
      found_lib=""
      for inclpath in /usr/local/lib /usr/local/lib/stlport
      do
        for stllib in stlport stlport_gcc
        do
          LDFLAGS_save=${LDFLAGS}
          LIBS_save=${LIBS}
          LDFLAGS="${LDFLAGS} -L${inclpath}"
          LIBS="${LIBS} -l${stllib}"
          AC_TRY_LINK([
#include <locale>
]
          , , [ found_lib="1" ], )
          LDFLAGS=${LDFLAGS_save}
          LIBS=${LIBS_save}
          if test "x${found_lib}" = "x1"
          then
            ac_cv_stlportlib=${inclpath}
            break
          fi
        done
        if test "x${found_lib}" = "x1"
        then
          break
        fi
      done
      if test "x${found_lib}" = "x"; then
        echo "not found"
        AC_MSG_WARN( [

	*** You don't seem to have STL support in your C++ compiler.
	*** You can try to build tripwire, but it probably won't compile.
        ])
      fi
    ])
  if test "x${ac_cv_stlportdir}" != "x"
  then
    LDFLAGS="${LDFLAGS} -L${ac_cv_stlportlib}"
    LIBS="${LIBS} -l${stllib}"
  fi
  ])
AC_LANG_RESTORE

dnl #####################
dnl STL specific headers
dnl #####################
AC_CHECK_HEADERS(wchar.h)

dnl #######################################################################
dnl Local checks/hacks.  The goal is to make this part as short as possible
dnl #######################################################################
AC_DEFINE_UNQUOTED(TARGET_OS, "${target}", [Target OS])
case $target in
	i386-*-freebsd*)
		;;
	alpha-*-freebsd*)
		;;
	i386-*-openbsd*)
		;;
	*-*-netbsd*)
		;;
	*-*-libertybsd*)
		CXXFLAGS="${CXXFLAGS} -DTW_LibertyBSD"
		;;
	i[[0-9]]86-pc-linux*)
		;;
	sparc-*-linux*)
		;;
	*-sun-solaris*)
		if test "x$enable_static" = xyes
		 then
		 AC_MSG_WARN( [

Sorry, tripwire will not properly link staticaly under Solaris.
This is due to tripwire's extensive use of gethostbyname(),
which can only be linked dynamicaly.  Please rerun configure
without the --enable-static option.
])
		 AC_DEFINE(SOLARIS_NO_GETHOSTBYNAME, 1, [Don't use gethostbyname() on Solaris])
		fi
		LIBS="-ldl $LIBS"
		AC_CHECK_HEADERS(strings.h)
		;;
	alpha*-*-osf*)
		CXXFLAGS="${CXXFLAGS} -D_ALPHA"
		;;
	*)
		;;
esac

dnl ###################################################
dnl Try linking a simple program with the flags we have
dnl ###################################################
AC_TRY_LINK( , return 0 , , [ AC_MSG_WARN( [

Hmmm... doesn't look so good.  I just tried linking a program
and it failed.  One reason could be missing static libraries
if you are trying to compile a static binary.
]) ] )

dnl Check for OpenSSL, now that we have a working compiler
AC_ARG_ENABLE(openssl,
        [  --disable-openssl       Don't link against OpenSSL libraries])
AC_ARG_WITH(ssl-dir,
	[  --with-ssl-dir=PATH     Specify path to OpenSSL installation ],
	[
		if test "x$withval" != "xno" ; then
			tryssldir=$withval
		fi
	]
)

dnl #################
dnl Check for OpenSSL
dnl #################
CORE_CRYPT_O="md5.o sha.o"
if test "x${enable_openssl}" != "xno"
then
 saved_LIBS="$LIBS"
 saved_LDFLAGS="$LDFLAGS"
 saved_CPPFLAGS="$CPPFLAGS"
 if test "x$prefix" != "xNONE" ; then
	tryssldir="$tryssldir $prefix"
 fi
  AC_CACHE_CHECK([for OpenSSL directory], ac_cv_openssldir, [
	for ssldir in $tryssldir "" /usr/local/openssl /usr/lib/openssl /usr/local/ssl /usr/lib/ssl /usr/local /usr/pkg /opt /opt/openssl ; do
		CPPFLAGS="$saved_CPPFLAGS"
		LDFLAGS="$saved_LDFLAGS"
		LIBS="$saved_LIBS -lcrypto"

		# Skip directories if they don't exist
		if test ! -z "$ssldir" -a ! -d "$ssldir" ; then
			continue;
		fi
		if test ! -z "$ssldir" -a "x$ssldir" != "x/usr"; then
			# Try to use $ssldir/lib if it exists, otherwise
			# $ssldir
			if test -d "$ssldir/lib" ; then
				LDFLAGS="-L$ssldir/lib $saved_LDFLAGS"
				if test ! -z "$need_dash_r" ; then
					LDFLAGS="-R$ssldir/lib $LDFLAGS"
				fi
			else
				LDFLAGS="-L$ssldir $saved_LDFLAGS"
				if test ! -z "$need_dash_r" ; then
					LDFLAGS="-R$ssldir $LDFLAGS"
				fi
			fi
			# Try to use $ssldir/include if it exists, otherwise
			# $ssldir
			if test -d "$ssldir/include" ; then
				CPPFLAGS="-I$ssldir/include $saved_CPPFLAGS"
			else
				CPPFLAGS="-I$ssldir $saved_CPPFLAGS"
			fi
		fi

		# Basic test to check for compatible library and
		# correct linking
		if test "$cross_compiling" = yes; then
                        found_crypto=1
                else       
		AC_TRY_RUN(
			[
#include <string.h>
#include <openssl/rand.h>
int main(void)
{
	char a[2048];
	memset(a, 0, sizeof(a));
	RAND_add(a, sizeof(a), sizeof(a));
	return(RAND_status() <= 0);
}
			],
			[
				found_crypto=1
				break;
			], []
		)
                fi

		if test ! -z "$found_crypto" ; then
			break;
		fi
	done

	if test -z "$ssldir" ; then
		ssldir="(system)"
	fi

	if test -z "$found_crypto" ; then
		ac_cv_openssldir="not found"
	else
		ac_cv_openssldir=$ssldir
	fi
])

LIBS="$saved_LIBS"

if (test ! -z "$ac_cv_openssldir" && test "x$ac_cv_openssldir" != "xnot found") ; then
	dnl Need to recover ssldir - test above runs in subshell
	ssldir=$ac_cv_openssldir
	if test "x$ssldir" != "x/usr" -a "x$ssldir" != "x(system)"; then
		# Try to use $ssldir/lib if it exists, otherwise
		# $ssldir
		if test -d "$ssldir/lib" ; then
			LDFLAGS="-L$ssldir/lib $saved_LDFLAGS"
			if test ! -z "$need_dash_r" ; then
				LDFLAGS="-R$ssldir/lib $LDFLAGS"
			fi
		else
			LDFLAGS="-L$ssldir $saved_LDFLAGS"
			if test ! -z "$need_dash_r" ; then
				LDFLAGS="-R$ssldir $LDFLAGS"
			fi
		fi
		# Try to use $ssldir/include if it exists, otherwise
		# $ssldir
		if test -d "$ssldir/include" ; then
			CPPFLAGS="-I$ssldir/include $saved_CPPFLAGS"
		else
			CPPFLAGS="-I$ssldir $saved_CPPFLAGS"
		fi
	fi
	LIBS="$saved_LIBS -lcrypto"
        AC_CHECK_HEADERS(openssl/md5.h openssl/sha.h)
        CORE_CRYPT_O=""
 fi
fi

AC_CONFIG_COMMANDS([mk_lib], [test -d lib || mkdir lib])
AC_CONFIG_COMMANDS([mk_bin], [test -d bin || mkdir bin])

dnl #####################################
dnl Export the variables we use
dnl #####################################
AC_SUBST(CORE_CRYPT_O)

AC_CONFIG_FILES([
Makefile
man/Makefile
man/man4/Makefile
man/man5/Makefile
man/man8/Makefile
src/Makefile
src/cryptlib/Makefile
src/core/Makefile
src/db/Makefile
src/fco/Makefile
src/fs/Makefile
src/tw/Makefile
src/twcrypto/Makefile
src/twparser/Makefile
src/util/Makefile
src/twprint/Makefile
src/twadmin/Makefile
src/siggen/Makefile
src/tripwire/Makefile
src/twtest/Makefile
])
AC_OUTPUT
