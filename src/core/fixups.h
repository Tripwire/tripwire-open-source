//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2019 Tripwire,
// Inc. Tripwire is a registered trademark of Tripwire, Inc.  All rights
// reserved.
//
// This program is free software.  The contents of this file are subject
// to the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.  You may redistribute it and/or modify it
// only in compliance with the GNU General Public License.
//
// This program is distributed in the hope that it will be useful.
// However, this program is distributed AS-IS WITHOUT ANY
// WARRANTY; INCLUDING THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
// FOR A PARTICULAR PURPOSE.  Please see the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
//
// Nothing in the GNU General Public License or any other license to use
// the code or files shall permit you to use Tripwire's trademarks,
// service marks, or other intellectual property without Tripwire's
// prior written consent.
//
// If you have any questions, please contact Tripwire, Inc. at either
// info@tripwire.org or www.tripwire.org.


// ====================================================================
// fixups.h :
// This file is included at the end of the generated config.h, for repairing things
// the configure script got wrong.  Because this does happen now and then.
//
// This header is intentionally not wrapped with the usual #ifndef/#define/#endif
// since config.h itself doesn't do that, and we need to be inclded right after
// it each time it's included.


// =====================================================================
// Cross compiling to powerpc-wrs-vxworks from linux amd64, observed that
// AC_CHECK_FUNC defines HAVE_xxx for every checked function, including
// ones that don't and can't exist like fork() & symlink(), and others that
// could at least optionally exist, but don't in my cross compiler, like socket().
//

#if HAVE_BROKEN_AC_CHECK_FUNC
  #if defined(__VXWORKS__) || defined(__vxworks)
    #undef HAVE__EXIT
    #undef HAVE_CHOWN
    #undef HAVE_EXECVE
    #undef HAVE_FORK
    #undef HAVE_GETHOSTID
    #undef HAVE_GETUID
    #undef HAVE_LSTAT
    #undef HAVE_MKSTEMP
    #undef HAVE_MKTEMP
    #undef HAVE_POPEN
    #undef HAVE_SOCKET
    #undef HAVE_SYMLINK
    #undef HAVE_SYSLOG
    #undef HAVE_READLINK
    #undef HAVE_TZSET
  #endif
#endif

