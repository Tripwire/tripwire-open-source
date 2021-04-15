dnl Process this file with autoconf to produce a configure script.
dnl
dnl

AC_INIT([tripwire], [2.4.3.8], [https://github.com/Tripwire/tripwire-open-source/issues], [tripwire], [https://github.com/Tripwire/tripwire-open-source])
AC_CONFIG_SRCDIR([src/tw/tw.cpp])
AC_CANONICAL_HOST([])
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_COPYRIGHT([The developer of the original code and/or files is Tripwire, Inc.  Portions created by Tripwire, Inc. are copyright 2000-2019 Tripwire, Inc.  Tripwire is a registered trademark of Tripwire, Inc.  All rights reserved.])
AC_REVISION([$Revision: 2.4.3.8 $])

dnl ensure that things like _GNU_SOURCE are enabled, since we use a handful of extended
dnl things like O_NOATIME when they're available.  autotools wants this to happen early
dnl before it tries to compile anything
AC_USE_SYSTEM_EXTENSIONS

dnl AC_LANG_CPLUSPLUS wants to set CXXFLAGS to include '-g -O2', but we probably don't want to
dnl include debug symbols by default, so save off the previous value & restore it afterward
tmp_cxxflags=${CXXFLAGS}
AC_LANG_CPLUSPLUS
CXXFLAGS=${tmp_cxxflags}

dnl *******************************************************
dnl Append an include to config.h for a handcrafted header
dnl that sets things back to (hopefully) correct values
dnl for known cases where autoconf does the Wrong Thing
dnl *******************************************************
AH_BOTTOM([#include "core/fixups.h"])

dnl ###################
dnl Checks for programs
dnl ###################
dnl AC_PROG_CC([gcc clang suncc aCC xlc_r gxlc cl])
AC_PROG_CXX([g++ clang++ sunCC aCC xlc++_r gxlc++ ixlc icpc owcc cl KCC cfront c++])
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


dnl #########################################
dnl Set up per-compiler constants & base args
dnl #########################################

dnl first look at compilers that define __GNUC__
dnl since automake is good at that.
dnl
if test "x${GXX}" != "x"; then

    CXXFLAGS=${CXXFLAGS:-"-O -pipe"}

    AC_DEFINE(HAVE_GNUC, 1, [Compiler supports GNU C++ dialect & args])
    
    case "${CXX}" in
        *clang++*)
            AC_DEFINE(HAVE_CLANG, 1, [Uses the Clang++ compiler])
	    ;;
	icpc)
            AC_DEFINE(HAVE_INTEL_ICC, 1, [Uses the Intel ICC compiler])
	    ;;	
	*)
            AC_DEFINE(HAVE_GCC, 1, [Uses the GNU g++ compiler])
	    ;;	
    esac

else
    dnl in case Clang stops defining __GNUC__ or user undefines it
    if test "x${CXX}" = "xclang++"; then
	AC_DEFINE(HAVE_CLANG, 1, [Uses the Clang compiler])
	CXXFLAGS=${CXXFLAGS:-"-O -pipe"}
    fi

    if test "x${CXX}" = "xgxlc++"; then
        AC_DEFINE(HAVE_IBM_GXLC, 1, [Uses the gxlc++ compiler])

        dnl OBJECT_MODE needs to be set while compiling
        export OBJECT_MODE=64
        CXXFLAGS=${CXXFLAGS:-"-O -maix64 -Wx,-qinfo=all"}
        ARFLAGS=${ARFLAGS:-"-X 64"}
    fi

    if test "x${CXX}" = "xxlc++_r"; then
        AC_DEFINE(HAVE_IBM_XLC, 1, [Uses the IBM XL C++ compiler])

        dnl OBJECT_MODE needs to be set while compiling
        export OBJECT_MODE=64
        CXXFLAGS=${CXXFLAGS:-"-O -q64 -qinfo=all -qsuppress=1540-5311:1540-5337:1540-5341"}
        ARFLAGS=${ARFLAGS:-"-X 64"}
    fi

    dnl *** TODO these compilers need work on default args ****

    if test "x${CXX}" = "xsunCC"; then
        AC_DEFINE(HAVE_ORACLE_SUNCC, 1, [Uses the sunCC compiler])
    fi

    if test "x${CXX}" = "xaCC"; then
        AC_DEFINE(HAVE_HP_ACC, 1, [Uses the aCC compiler])
    fi

    if test "x${CXX}" = "xowcc"; then
        AC_DEFINE(HAVE_OPENWATCOM, 1, [Uses the OpenWatcom compiler])
        CXXFLAGS=${CXXFLAGS:-"-xs"}
    fi
 

fi

dnl ######################
dnl Enable various warnings
dnl #####################

AX_CHECK_COMPILE_FLAG([-Wall],        [CXXFLAGS="$CXXFLAGS -Wall"])
AX_CHECK_COMPILE_FLAG([-Wextra],      [CXXFLAGS="$CXXFLAGS -Wextra"])
dnl OpenWatcom compiler understands -Wextra but not -Wno-unused-parameter, so detect it separately
AX_CHECK_COMPILE_FLAG([-Wno-unused-parameter], [CXXFLAGS="$CXXFLAGS -Wno-unused-parameter"])

dnl AX_CHECK_COMPILE_FLAG([-Weverything], [CXXFLAGS="$CXXFLAGS -Weverything"])

dnl #####################
dnl Configuration options
dnl #####################
dnl
dnl  These options generally expect a compiler+linker
dnl  that understand gcc-like command line options.

AC_ARG_ENABLE(static, [  --enable-static         compile static binaries])
if test "x$enable_static" = xyes
then LDFLAGS="${LDFLAGS} -static"
fi

AC_ARG_ENABLE(debug,  [  --enable-debug          compile with debuging enabled])
if test "x$enable_debug" = xyes
then
   CXXFLAGS="${CXXFLAGS} -g"
   AC_DEFINE(DEBUG, 1, [Compile with debug code])
else
   AC_DEFINE(NDEBUG, 1, [Compile without debug code])
fi

AC_ARG_ENABLE(coverage, [  --enable-coverage       enable code coverage])
if test "x$enable_coverage" = xyes
then
   CXXFLAGS="${CXXFLAGS} --coverage"
   LDFLAGS="${LDFLAGS} --coverage"
fi

AC_ARG_ENABLE(profiling, [  --enable-profiling      enable profiling])
if test "x$enable_profiling" = xyes
then
   CXXFLAGS="${CXXFLAGS} -pg"
   LDFLAGS="${LDFLAGS} -pg"
fi

AC_ARG_ENABLE(urandom,  [  --enable-urandom        use /dev/urandom])
if test "x$enable_urandom" = xyes
then
   AC_DEFINE(ENABLE_DEV_URANDOM, 1, [Enable use of /dev/urandom])
fi



dnl #######################
dnl Checks for header files
dnl #######################
AC_HEADER_STDC
AC_HEADER_DIRENT

AC_CHECK_HEADERS(grp.h)
AC_CHECK_HEADERS(langinfo.h)
AC_CHECK_HEADERS(memory.h)
AC_CHECK_HEADERS(pthread.h)
AC_CHECK_HEADERS(pwd.h)
AC_CHECK_HEADERS(signum.h bits/signum.h, break)
AC_CHECK_HEADERS(stdarg.h varargs.h, break)
AC_CHECK_HEADERS(stdint.h)
AC_CHECK_HEADERS(syslog.h sys/syslog.h, break)
AC_CHECK_HEADERS(termios.h)
AC_CHECK_HEADERS(unistd.h sys/unistd.h, break)
AC_CHECK_HEADERS(features.h)
AC_CHECK_HEADERS(sys/file.h)
AC_CHECK_HEADERS(sys/param.h)
AC_CHECK_HEADERS(sys/mount.h,,,
[[#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
]])

AC_CHECK_HEADERS(sys/select.h)
AC_CHECK_HEADERS(sys/socket.h)
AC_CHECK_HEADERS(sys/statfs.h)
AC_CHECK_HEADERS(sys/sysmacros.h)
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_HEADERS(sys/ustat.h)
AC_CHECK_HEADERS(sys/utsname.h)
AC_CHECK_HEADERS(sys/wait.h)

AC_CHECK_HEADERS(sys/fs/vx_ioctl.h)
AC_CHECK_HEADERS(unixlib/local.h)
AC_CHECK_HEADERS(proto/bsdsocket.h)
AC_CHECK_HEADERS(proto/exec.h)
AC_CHECK_HEADERS(bsdsocket/socketbasetags.h)

dnl # Special case for malloc.h, because it's deprecated on most systems.
CPPFLAGS_SAVE="${CPPFLAGS}"
CPPFLAGS="${CPPFLAGS} -Werror"
AC_CHECK_HEADERS(malloc.h)
CPPFLAGS="${CPPFLAGS_SAVE}"

dnl #############################################################
dnl Checks for typedefs, structures, and compiler characteristics
dnl #############################################################
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_TYPE_UINTPTR_T
AC_HEADER_TIME
AC_C_BIGENDIAN
AC_C_CHAR_UNSIGNED

AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(time_t)
AC_CHECK_SIZEOF(wchar_t)

AC_CHECK_ALIGNOF(int)
AC_CHECK_ALIGNOF(long)
AC_CHECK_ALIGNOF(long long)

dnl TODO: These could be replaced with actual detection logic, for the few
dnl   remaining platforms that have non-2s complement signed ints
AC_DEFINE(USES_1S_COMPLEMENT, 0, [Uses one's complement])
AC_DEFINE(USES_2S_COMPLEMENT, 1, [Uses two's complement])
AC_DEFINE(USES_SIGNED_MAGNITUDE, 0, [Uses signed magnitute])

AC_DEFINE(EXCEPTION_NAMESPACE, std::, [this is the prefix for STL exception functions])

dnl whether or not to generate debuging code?
dnl this gets defined elsewhere based on config args AC_DEFINE(NDEBUG, 1, [don't generate debuging code])

dnl look for struct stat members that aren't always there:
dnl   st_rdev, st_blksize, & st_blocks are standard but a few platforms don't have them.
dnl   st_flags & st_gen are BSD-isms & are mostly seen there, including macOS
dnl   st_flag (singular) is AIX-only, while st_attrib holds DOS attribute bits on VxWorks
AC_CHECK_MEMBERS([struct stat.st_rdev, struct stat.st_blksize, struct stat.st_blocks, struct stat.st_flags, struct stat.st_gen, struct stat.st_flag, struct stat.st_attrib])

dnl detect large file support & use it where available
AC_SYS_LARGEFILE

dnl detect whether Posix termios is available.
dnl we use termios to turn echo off when user is entering a passphrase
AC_SYS_POSIX_TERMIOS

dnl detect whether C++ compiler supports any variant of 'restrict' keyword
dnl (the macro says 'C' but works for both C & C++)
AC_C_RESTRICT

dnl ******************************************
dnl Check whether AC_CHECK_FUNC(S) lies to us
dnl ******************************************
AC_CHECK_FUNCS(broken_ac_check_func)

dnl #############################
dnl Checks for standard functions
dnl #############################
AC_CHECK_FUNCS(strftime)
AC_CHECK_FUNCS(getaddrinfo gethostname gethostbyname gethostid)
AC_CHECK_FUNCS(getuid getgid)
dnl various ways of creating/naming temp files
AC_CHECK_FUNCS(mkstemp mktemp tmpnam, break)
dnl i18n string fns
AC_CHECK_FUNCS(mblen mbtowc)
dnl misc assorted
AC_CHECK_FUNCS(swab tzset _exit popen memcpy_s)

dnl posix way to read files
AC_CHECK_FUNCS(openat open read close)
dnl standard libc way to read files
AC_CHECK_FUNCS(fopen fread fclose)
dnl only standard way to read dir contents
AC_CHECK_FUNCS(opendir readdir closedir)
dnl posix (and extended) ways to get file attributes
AC_CHECK_FUNCS(statx fstatat lstat stat)
dnl posix way to get symlink contents
AC_CHECK_FUNCS(readlinkat readlink)

dnl posix fns that make filesystem changes
AC_CHECK_FUNCS(chown chmod link symlink mknod mkfifo)
dnl more posix fns that make fs changes
AC_CHECK_FUNCS(unlink rename ftruncate)
dnl posix syslog fns
AC_CHECK_FUNCS(openlog syslog closelog)
dnl posix process fns
AC_CHECK_FUNCS(fork vfork execve posix_spawn)
dnl bsd socket fns
AC_CHECK_FUNCS(socket connect bind)
dnl posix user & group stuff
AC_CHECK_FUNCS(setuid setgid getgrgid)

dnl check for posix_fadvise
AC_CHECK_HEADERS(fcntl.h, [AC_CHECK_FUNCS(posix_fadvise)])

dnl check for setlocale
AC_CHECK_HEADERS(locale.h, [AC_CHECK_FUNCS(setlocale)])
dnl check for a C isprint
AC_CHECK_HEADERS(ctype.h, [AC_CHECK_FUNCS(isprint)])

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
dnl check for directio syscall (Solaris)
AC_CHECK_FUNCS(directio)

dnl check for existence of fseeko() as well as ftello()
AC_FUNC_FSEEKO


dnl ##############################################  
dnl check for various RNG/PRNG devices
dnl ##############################################  


dnl ############################################## 
dnl AROS pops up a "Please insert disk" dialog for /dev
dnl if script looks for devices (which don't exist)
dnl so don't even try looking.
dnl ############################################## 

case $host_os in
  aros*)
    ;;
  *)
    if test "$cross_compiling" != yes; then
        AC_CHECK_FILES(/dev/random /dev/urandom /dev/arandom)
    fi
    ;;
esac

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

dnl Check for STL support, either native or STLport

dnl
dnl Check for STL headers
dnl

if test -z "${STLPORT}"; then
   STLPORT="."
fi

stlport_include="${STLPORT}/stlport"
stlport_lib="${STLPORT}/lib"

AC_CHECK_HEADERS(limits limits.h)

dnl Use <locale> as a proxy for C++98-compliant-ish STL
dnl and look for STLport if file isn't found immediately
AC_CHECK_HEADERS(locale,,[
  dnl Don't have native STL headers, look in other places
  AC_CACHE_CHECK([for STLport directory], ac_cv_stlportdir, [
    echo "Native <locale> header not found; checking if STLport is available"
    found_locale=""
    for inclpath in ${stlport_include} /usr/local/include /usr/local/include/stlport
    do
      CPPFLAGS_save=${CPPFLAGS}
      CPPFLAGS="${CPPFLAGS} -I${inclpath} -nostdinc++"
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
    CPPFLAGS="${CPPFLAGS} -I ${ac_cv_stlportdir} -nostdinc++"
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
        AC_MSG_WARN([No posix threads detected; OST may not link.]) ])
        ])
    ])

    AC_CACHE_CHECK([for STLport library directory], ac_cv_stlportlib, [
      found_lib=""
      for inclpath in ${stlport_lib} /usr/local/lib /usr/local/lib/stlport
      do
        for stllib in stlport stlport_gcc
        do
          LDFLAGS_save=${LDFLAGS}
          LIBS_save=${LIBS}
	  LD_LIBRARY_PATH_save=${LD_LIBRARY_PATH}
	  
          LDFLAGS="${LDFLAGS} -L${inclpath}"
          LIBS="${LIBS} -l${stllib}"
	  export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:+:LD_LIBRARY_PATH}${inclpath}"
	  
          AC_TRY_LINK([
#include <locale>
]
          , , [ found_lib="1" ], )
          LDFLAGS=${LDFLAGS_save}
          LIBS=${LIBS_save}
	  export LD_LIBRARY_PATH="${LD_LIBRARY_PATH_save}"
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
    LDFLAGS="${LDFLAGS} -L ${ac_cv_stlportlib}"
    LIBS="${LIBS} -l${stllib}"
    export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:+:LD_LIBRARY_PATH}${ac_cv_stlportlib}"

    AC_DEFINE(HAVE_LOCALE, 1, [Define to 1 if you have the <locale> header file.])
    AC_DEFINE(HAVE_STLPORT, 1, [Define if STLport library is used])
  fi
  ])


dnl #####################
dnl STL specific headers
dnl #####################
AC_CHECK_HEADERS(wchar.h)
AC_CHECK_HEADERS(sstream strstream)

dnl #######################################################################
dnl Local checks/hacks.  The goal is to make this part as short as possible
dnl #######################################################################
AC_DEFINE_UNQUOTED(TARGET_OS, "${host}", [Target OS])
case $host in
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

Sorry, tripwire will not properly link statically under Solaris.
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
		if test "x$cross_compiling" = "xyes"; then
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
