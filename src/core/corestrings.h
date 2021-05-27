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
//
// Name....: corestrings.h
// Date....: 05/05/99
// Creator.: Robert DiFalco (rdifalco)
//
//

#ifndef __CORESTRINGS_H
#define __CORESTRINGS_H


#include "core.h" // for: STRINGTABLE syntax


//--Message Keys

TSS_BeginStringIds(core)

    STR_ERR2_ARCH_CRYPTO_ERR,
    STR_ERR2_BAD_ARG_PARAMS, STR_ERROR_ERROR, STR_ERROR_WARNING, STR_ERROR_COLON, STR_ERROR_HEADER, STR_ERROR_EXITING,
    STR_ERROR_CONTINUING, STR_ERR2_FILENAME, STR_ERROR_FILENAME, STR_NUMBER_TOO_BIG, STR_UNKNOWN, STR_SIGNAL,
    STR_NEWLINE, STR_MEMARCHIVE_FILENAME, STR_MEMARCHIVE_ERRSTR, STR_ENDOFTIME, STR_UNKNOWN_TIME, STR_BAD_TEMPDIRECTORY,

    /// Particularly useful for eCharacterSet and eCharacterEncoding

    STR_ERR_ISNULL, STR_ERR_OVERFLOW, STR_ERR_UNDERFLOW,
    STR_ERR_BADCHAR

    TSS_EndStringIds(core)


#endif //__CORESTRINGS_H
