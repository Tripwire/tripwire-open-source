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
// Name....: corestrings.cpp
// Date....: 05/06/99
// Creator.: Robert DiFalco (rdifalco)
//

#include "stdcore.h"     // for: pch (required by all core source modules)
#include "corestrings.h" // for: cCore and core::STR_IDS


TSS_BeginStringtable(cCore)

    TSS_StringEntry(core::STR_ERR2_ARCH_CRYPTO_ERR, _T("File could not be decrypted.")),
    TSS_StringEntry(core::STR_ERR2_BAD_ARG_PARAMS, _T("Argument: ")),
    TSS_StringEntry(core::STR_ERROR_ERROR, _T("### Error")),
    TSS_StringEntry(core::STR_ERROR_WARNING, _T("### Warning")), TSS_StringEntry(core::STR_ERROR_COLON, _T(":")),
    TSS_StringEntry(core::STR_ERROR_HEADER, _T("### ")), TSS_StringEntry(core::STR_ERROR_EXITING, _T("Exiting...")),
    TSS_StringEntry(core::STR_ERROR_CONTINUING, _T("Continuing...")),
    TSS_StringEntry(core::STR_ERR2_FILENAME, _T("Filename: ")),
    TSS_StringEntry(core::STR_ERROR_FILENAME, _T("Filename: ")), TSS_StringEntry(core::STR_UNKNOWN, _T("Unknown")),
    TSS_StringEntry(core::STR_NUMBER_TOO_BIG, _T("Number too big")),
    TSS_StringEntry(core::STR_SIGNAL, _T("Software interrupt forced exit:")),
    TSS_StringEntry(core::STR_NEWLINE, _T("\n")),
    TSS_StringEntry(core::STR_MEMARCHIVE_FILENAME, _T("Error occured in internal memory file")),
    TSS_StringEntry(core::STR_MEMARCHIVE_ERRSTR, _T("")),
    TSS_StringEntry(core::STR_ENDOFTIME, _T("Tripwire is not designed to run past the year 2038.\nNow exiting...")),
    TSS_StringEntry(core::STR_UNKNOWN_TIME, _T("Unknown time")),
    TSS_StringEntry(
        core::STR_BAD_TEMPDIRECTORY,
        _T("Solution: Check existence/permissions for directory specified by TEMPDIRECTORY in config file")),
    /// Particularly useful for eCharacter and eCharacterEncoding

    TSS_StringEntry(core::STR_ERR_ISNULL, _T("Argument cannot be null.")),
    TSS_StringEntry(core::STR_ERR_OVERFLOW, _T("An overflow has been detected.")),
    TSS_StringEntry(core::STR_ERR_UNDERFLOW, _T("An underflow has been detected.")),
    TSS_StringEntry(core::STR_ERR_BADCHAR, _T("Input contained an invalid character."))

        TSS_EndStringtable(cCore)
