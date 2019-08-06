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
//
/*
 * This is the header file; include it in programs that use
 * the more secure system call (or the more secure popen call)
 * It also contains error codes and such
 *
 * Author information:
 * Matt Bishop
 * Department of Computer Science
 * University of California at Davis
 * Davis, CA  95616-8562
 * phone (916) 752-8060
 * email bishop@cs.ucdavis.edu
 *
 * This code is placed in the public domain.  I do ask that
 * you keep my name associated with it, that you not represent
 * it as written by you, and that you preserve these comments.
 * This software is provided "as is" and without any guarantees
 * of any sort.
 *
 * Version information:
 * 1.0          May 25, 1994            Matt Bishop
 */
/*
 * forward declarations
 */

#include "platform.h"

#if SUPPORTS_POSIX_FORK_EXEC

#ifdef __STDC__
void le_clobber(void);
int le_set(const char*);
int le_unset(const char*);
int le_umask(int);
int le_openfd(int);
int le_closefd(int);
int le_euid(int);
int le_egid(int);

#if USES_MSYSTEM
int msystem(const char*);
#endif

#if USES_MPOPEN
FILE *mpopen(const char*, const char*);
#endif

int mpclose(FILE *);
int mfpopen(const char*, FILE *[]);
int mfpclose(int, FILE *[]);
int mxfpopen(const char*[], FILE *[]);
int mxfpclose(int, FILE *[]);
int schild(const char*, const char*[], const char*[], FILE *[], int);
int echild(int);
#else
void le_clobber();
int le_set();
int le_unset();
int le_umask();
int le_openfd();
int le_closefd();
int le_euid();
int le_egid();
int msystem();
FILE *mpopen();
int mpclose();
int mfpopen();
int mfpclose();
int mxfpopen();
int mxfpclose();
int schild();
int echild();
#endif

/*
 * define error codes
 */
#define SE_NONE     0       /* no error */
#define SE_NOMEM    -1      /* no memory */
#define SE_NOPIPE   -2      /* no pipes */
#define SE_NOVAR    -3      /* variable not defined */
#define SE_BADFD    -4      /* invalid file descriptor */

/*
 * default security settings
 */
#ifndef DEF_UMASK
#   define DEF_UMASK    077     /* only owner has privileges */
#endif
#ifndef UID_RESET
#   define UID_RESET    -2      /* reset EUID to RUID */
#endif
#ifndef GID_RESET
#   define GID_RESET    -2      /* reset EGID to RGID */
#endif
#ifndef DEF_PATH
#   if IS_FREEBSD
#       define DEF_PATH "PATH=/sbin:/usr/sbin:/bin:/usr/bin"  /* default search path */
#   else
#       define DEF_PATH "PATH=/bin:/usr/bin:/usr/ucb"   /* default search path */
#   endif
#endif
#ifndef DEF_SHELL
#   define DEF_SHELL    "SHELL=/bin/sh" /* default shell */
#endif
#ifndef DEF_IFS
#   define DEF_IFS      "IFS= \t\n" /* default IFS */
#endif
#ifndef DEF_TZ
#   define DEF_TZ       "TZ"        /* default TZ */
#endif
#ifndef NOSHELL
#   define NOSHELL      "/bin/sh"   /* use this if no shell */
#endif

#endif // SUPPORTS_POSIX_FORK_EXEC
